// dmx dimmer, transceiver module

#include "master.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>      // TODO: disable with fuses and remove refs
//#include <util/delay.h>  // TODO: use or remove
#include <avr/interrupt.h>

#include "iocontrol.h"
#include "fakedelay.h"  // TODO: remove?
#include "dmx.h"
#include "spi.h"
#include "adc.h"
#include "usart.h"

dmx_t dmx = {IDLE, 1, 0, 0, 0, 0, 0, 255};


int main (void) {
    /* uint8_t confl = 0; */
    /* uint8_t confh = 0; */
    uint8_t c;      // channel iterator
    uint8_t chanval;
    uint8_t retval;

    wdt_disable();

    leds_init();
    delay_ms(200);
    led_off(0);
    delay_ms(200);
    led_off(1);

    /* TODO: make sure master starts up later than slave
     * otherwise slave doesn't get first interrupt
     * perhaps this won't happen when master takes time to read settings
     */
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);

    /* to read in configuration from panel's DIP switches, a single
     * pulse must be sent on the SCK line prior to enabling SPI
     * properly; this is due to 74166's mode of operation and is not
     * needed if 74165 ICs are used instead; consult datasheets
     */
    cfg_init();

    /* cfg_select(); */
    /* delay_ms(1);   // maybe redundant, should be one set-up time */

    /* cfg_mode_parallel(); */
    /* delay_ms(1); */

    /* // pulse to read in bits */
    /* output_low(SPI_PORT, SPI_SCK); */
    /* output_high(SPI_PORT, SPI_SCK); */

    /* cfg_mode_serial(); */
    /* delay_ms(1); */

    // proceed with proper SPI operation
    spi_master_init();
    //SPCR |= (1<<CPOL);  // otherwise can't read first bit

    // TODO: cfg_read()
    /* // read in first octet */
    /* confl = spi_master_transmit(SPI_TRANSMIT_DUMMY); */

    /* // read in second octet */
    /* confh = spi_master_transmit(SPI_TRANSMIT_DUMMY); */

    /* output_high(SPI_PORT, SPI_SCK);  // TODO: is this needed? */
    /* cfg_deselect(); */
    /* cfg_reset_enable(); */

    /* adc_init(); */
    usart_init();
    sei();

    while (1) {
	// see if preheat/maxval on panel changed
	/* if ( !(adc_running()) ) { */
	/*     adc_channel_toggle(); */
	/*     adc_start(); */
	/* } */

	// iterate over DMX channels
	for (c = 0; c < DMX_CHANNELS; c++) {
	    // check if new data present for this channel
	    if (dmx.dataisnew & ((uint16_t)1 << c)) {
		/* even newer data may come while transmitting current
		 * value, so unset the channel's flag asap
		 */
		dmx.dataisnew &= ~((uint16_t)1 << c);

		/* make sure chanval is in [preheat; maxval] range, but
		 * use a temporary var for this - otherwise the USART
		 * interrupt might change it back again
		 */
		chanval = dmx.chanval[c];
		/* if (chanval < dmx.preheat) */
		/*     chanval = dmx.preheat; */
		/* if (chanval > dmx.maxval) */
		/*     chanval = dmx.maxval; */

		led_on(0); // debug

		// for any slave, set which of the 4 dmx channels t'is for
		spi_chan_select(c%4);

		// select one of three slave arbiters
		spi_request_interrupt(c/4);

		led_off(0); // debug

		// wait 'till slave ready
		led_on(1);
		while ( !(SPI_SLAVES_PIN & _BV(SPI_OUT_OK_PIN)) );
		led_off(1);
		
		// transmit channel's value
		retval = spi_master_transmit(chanval);

		// pull-ups on other end, reduce power consumption
		spi_chan_select(SPI_CHAN_RESET);

		// check if transmission not successful
		if (retval != SPI_TRANSMIT_DUMMY) {
		    dmx.dataisnew |= ((uint16_t)1 << c);
		}
		
	    }  // if (new chan data)
	}  // for (channel iterate)
    }  // while (1)

    return 1;
}

// interrupt: usart receive complete
ISR (USART_RX_vect, ISR_BLOCK) {
    // reading data clears status flags, so read status first
    dmx.status = UCSR0A;
    dmx.data = UDR0;

    led_on(0);

    // data overrun or frame error (break condition)
    if ( dmx.status & (_BV(DOR0)|_BV(FE0)) ) {
	dmx.state = BREAK;
    }
    else {
	switch (dmx.state) {  // previous slot's state
	case BREAK:
	    if (dmx.data != 0) dmx.state = IDLE;  // invalid start code
	    else {
		dmx.slot = dmx.address - 1;  // skip this many slots
		if (dmx.slot == 0) dmx.state = DATA; // dmx.address == 1
		else dmx.state = SKIP;
	    }
	    break;
	case SKIP:
	    if (--dmx.slot == 0) dmx.state = DATA;
	    break;
	case DATA:
	    if (dmx.chanval[dmx.slot] != dmx.data) {
		dmx.chanval[dmx.slot] = dmx.data;
		dmx.dataisnew |= ((uint16_t)1 << dmx.slot);
	    }
	    if (++dmx.slot == DMX_CHANNELS) dmx.state = IDLE;
	    break;
	case IDLE:
	    break;
	}
    }

    led_off(0);
}

// interrupt: ADC conversion complete
/* ISR (ADC_vect, ISR_NOBLOCK) { */
/*     if (adc_channel_get()) */
/* 	dmx.maxval = ADCH/2 + 128; */
/*     else */
/* 	dmx.preheat = ADCH/2; */
/* } */
