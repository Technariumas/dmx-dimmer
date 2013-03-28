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

volatile dmx_t dmx = {IDLE, 1, 0, 0, 0, 0, 0, 255};


// hack: separate slave OK lines are needed :(
uint8_t slave_is_available (uint8_t s) {
    switch (s) {
    case 0:
    	if (SPI_SLAVES_PIN & _BV(SPI_OUT_OK1_PIN)) return 0;
    	break;
    case 1:
    	if (SPI_SLAVES_OUTOK_PIN & _BV(SPI_OUT_OK2_PIN)) return 0;
    	break;
    case 2:
    	if (SPI_SLAVES_OUTOK_PIN & _BV(SPI_OUT_OK3_PIN)) return 0;
    	break;
    default:
    	return 0;
    	break;
    } // switch

    return 1;
}


int main (void) {
    uint8_t confl = 0;  // configuration from panel (low byte)
    uint8_t confh = 0;  // configuration from panel (high byte)
    uint8_t c;          // channel iterator
    uint8_t chanval;
    uint8_t retval;

    wdt_disable();

    leds_init();
    delay_ms(200);
    led_off(0);
    delay_ms(200);
    led_off(1);

    /* to read in configuration from panel's DIP switches, a single
     * pulse must be sent on the SCK line prior to enabling SPI
     * properly; this is due to 74166's mode of operation and is not
     * needed if 74165 ICs are used instead; consult datasheets
     */
    cfg_init();

    cfg_reset_disable();
    cfg_select();
    delay_ms(1);   // maybe redundant, should be one set-up time

    cfg_mode_parallel();
    delay_ms(1);

    // hack: pulse clock line for 74166 to read in bits from switches
    // (not needed for 74165?)
    output_low(SPI_PORT, SPI_SCK);
    delay_ms(1);
    output_high(SPI_PORT, SPI_SCK);
    delay_ms(1);

    cfg_mode_serial();
    delay_ms(1);

    // proceed with proper SPI operation
    spi_master_init();
    cfg_select();
    /* SPCR |= _BV(CPOL);  // otherwise can't read first bit */

    /* read in configuration (address and 6 binary settings)
     * reading is performed by pushing bogus data to dumb shift
     * registers; those push back the real deal
     */
    confh = spi_master_transmit(SPI_TRANSMIT_DUMMY);
    confl = spi_master_transmit(SPI_TRANSMIT_DUMMY);

    /* output_high(SPI_PORT, SPI_SCK);  // TODO: is this needed? */
    cfg_deselect();
    cfg_reset_enable();

    // push in address' low byte
    dmx.address = (uint16_t)confl;
    // hack: hardware bug, address pins 9 and 10 are in reversed order
    // address pin 10 sets address >= 512, good for soft-blocking device
    if (confh & 0b10) dmx.address |= 0b0100000000;
    if (confh & 0b01) dmx.address |= 0b1000000000;

    /* adc_init(); */

    usart_init();
    sei();

    /* TODO: make sure master starts up later than slave
     * otherwise slave doesn't get first interrupt
     * perhaps this won't happen when master takes time to read settings
     */
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);

    while (1) {
	// see if preheat/maxval on panel changed
	/* if ( !(adc_running()) ) { */
	/*     adc_channel_toggle(); */
	/*     adc_start(); */
	/* } */

	// iterate over DMX channels
	for (c = 0; c < DMX_CHANNELS; c++) {
	    // skip if slave is busy
	    /* while (slave_is_ready(c/4)); */
	    if ( !(slave_is_available(c/4)) ) continue;

	    // check if new data present for this channel
	    if (dmx.dataisnew & ((uint16_t)1 << c)) {
		/* even newer data may come while transmitting current
		 * value, so unset the channel's flag asap
		 */
		dmx.dataisnew &= ~((uint16_t)1 << c);

		/* TODO: make sure chanval is in [preheat; maxval] range, but
		 * use a temporary var for this - otherwise the USART
		 * interrupt might change it back again
		 */
		chanval = dmx.chanval[c];
		/* if (chanval < dmx.preheat) */
		/*     chanval = dmx.preheat; */
		/* if (chanval > dmx.maxval) */
		/*     chanval = dmx.maxval; */

		/* led_on(0); */

		// for any slave, set which of the 4 dmx channels t'is for
		spi_chan_select(c%4);

		// select one of three slaves (TODO: remove hard-coded 4?)
		spi_request_interrupt(c/4);

		// wait till slave says OK (marks itself busy/unavailable)
		led_on(0);  // debug
		if (c == 11) led_on(1);
		while (slave_is_available(c/4));
		if (c == 11) led_off(1);
		led_off(0); // debug
		
		// transmit channel's value
		retval = spi_master_transmit(chanval);

		/* led_off(1); */

		// pull-ups on other end, reduce power consumption
		spi_chan_select(SPI_CHAN_RESET);

		// check if transmission not successful or even newer
		// data arrived
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

    /* led_on(1); */

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

    /* led_off(1); */
}

// interrupt: ADC conversion complete
/* ISR (ADC_vect, ISR_NOBLOCK) { */
/*     if (adc_channel_get()) */
/* 	dmx.maxval = ADCH/2 + 128; */
/*     else */
/* 	dmx.preheat = ADCH/2; */
/* } */
