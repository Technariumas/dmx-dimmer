// dmx dimmer, transceiver module

#include "master.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>      // TODO: disable with fuses and remove refs
#include <avr/interrupt.h>

#include "iocontrol.h"
#include "fakedelay.h"    // TODO: replace with util/delay.h?
#include "dmx.h"
#include "spi.h"
#include "usart.h"

volatile dmx_t dmx = {
    .state = IDLE,
    .address = 1,
    .slot = 0,
    .status = 0,
    .data = 0,
    .preheat = 0,
    .maxval = 255,
    .chanval = { 0 }
};


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

    wdt_disable();

    leds_init();
    delay_ms(200);
    led_off(LED_RED);
    delay_ms(200);
    led_off(LED_GREEN);

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

    // start talking to slaves
    usart_init();
    sei();

    /* make sure master starts up later than slave
     * otherwise slave doesn't get first interrupt
     */
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);
    delay_ms(200);

    while (1) {
	// iterate over DMX channels
	for (uint8_t dmx_channel = 0; dmx_channel < DMX_CHANNELS; dmx_channel++) {
            uint8_t slave = dmx_channel / 4;
            uint8_t slave_channel = dmx_channel % 4;

	    // skip if slave is busy
	    /* while (slave_is_ready(dmx_channel/4)); */
	    if ( !(slave_is_available(slave)) ) continue;

            // use a local var: dmx structure is global, USART
            // interrupt has access to it and might change it
            uint8_t chanval = dmx.chanval[dmx_channel];

            // TODO: make sure chanval is in [preheat; maxval] range
            /* if (chanval < dmx.preheat) chanval = dmx.preheat; */
            /* if (chanval > dmx.maxval) chanval = dmx.maxval; */

            // for any slave, set which of the 4 dmx channels t'is for
            spi_chan_select(slave_channel);

            // select one of three slaves (TODO: remove hard-coded 4?)
            spi_request_interrupt(slave);

            // wait till slave says OK (marks itself busy/unavailable)
            led_on(LED_RED);
            while (slave_is_available(slave));
            led_off(LED_RED);
            
            // transmit channel's value
            spi_master_transmit(chanval);

            // pull-ups on other end, reduce power consumption
            spi_chan_select(SPI_CHAN_RESET);
	}  // for (channel iterate)
    }  // while (1)

    return 1;
}

// interrupt: usart receive complete
ISR (USART_RX_vect, ISR_BLOCK) {
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
	    }
	    if (++dmx.slot == DMX_CHANNELS) dmx.state = IDLE;
	    break;
	case IDLE:
	    break;
	}
    }
}
