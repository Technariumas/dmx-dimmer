// dmx dimmer, transceiver module

#include "transceiver.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>      // TODO: disable with fuses and remove refs
//#include <util/delay.h>  // TODO: use or remove
#include <avr/interrupt.h>

#include "iocontrol.h"
#include "fakedelay.h"  // TODO: remove?
#include "metaboard.h"  // TODO: remove
#include "dmx.h"
#include "spi.h"
#include "adc.h"
#include "usart.h"

dmx_t    dmx = {IDLE, 1, 0, 0, 0, 0, 0, 255};


int main (void) {
    /* uint8_t confl = 0; */
    /* uint8_t confh = 0; */
    uint8_t c;      // channel iterator
    uint8_t retval;

    wdt_disable();

    // leds are outputs
    leds_init();

    // led blink: devboard ok
    ledon(0); ledon(1); ledon(2);
    delay_ms(500);
    ledoff(0); ledoff(1); ledoff(2);

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
    /* ledon(0); */
    /* confl = spi_master_transmit(SPI_TRANSMIT_DUMMY); */
    /* ledoff(0); */

    /* // read in second octet */
    /* ledon(0); */
    /* confh = spi_master_transmit(SPI_TRANSMIT_DUMMY); */
    /* ledoff(0); */

    /* output_high(SPI_PORT, SPI_SCK);  // TODO: is this needed? */
    /* cfg_deselect(); */
    /* cfg_reset_enable(); */

    adc_init();
    usart_init();
    sei();

    while (1) {
	ledon(2);  // debug: start transmit

	// see if preheat/maxval on panel changed
	if (!adc_running()) {
	    adc_channel_toggle();
	    adc_start();
	}

	// iterate over DMX channels
	for (c = 0; c < DMX_CHANNELS; c++) {
	    // check if new data present for this channel
	    if (dmx.dataisnew & ((uint16_t)1 << c)) {
		/* even newer data may come in the time frame between
		 * transmission and checking if it was successful, so
		 * unset the channel's flag now and set it back on later
		 * if needed
		 */
		dmx.dataisnew &= ~((uint16_t)1 << c);

		// for any slave, set which of the 4 dmx channels t'is for
		spi_chan_select(c);

		// select one of three slave arbiters
		spi_request_interrupt(c/4);

		// wait 'till slave ready
		while ( !(SPI_SLAVES_PIN & _BV(SPI_OUT_OK_PIN)) );

		// transmit channel's value
		retval = spi_master_transmit(dmx.chanval[c]);

		// pull-ups on other end, reduce power consumption
		spi_chan_select(SPI_CHAN_RESET);

		// check if transmission not successful
		if (retval != SPI_TRANSMIT_DUMMY) {
		    dmx.dataisnew |= ((uint16_t)1 << c);
		    ledtoggle(1);
		}
	    }  // if (new chan data)
	}  // for (channel iterate)

	ledoff(2);  // debug: transmitted
    }  // while (1)

    return 1;
}

// interrupt: usart receive complete
ISR (USART_RX_vect, ISR_BLOCK) {
    ledon(0);  // debug: int start

    // reading data clears status flags, so read status first
    dmx.status = UCSR0A;
    dmx.data = UDR0;

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

    ledoff(0);  // debug: int stop
}

// interrupt: ADC conversion complete
ISR (ADC_vect, ISR_NOBLOCK) {
    if (adc_channel_get()) {  // was getting maxval
	dmx.maxval = ADCH/2 + 128;
    }
    else {                    // was getting preheat
	dmx.preheat = ADCH/2;
    }
}
