// dmx dimmer, transceiver module
// TODO: file header

#define F_CPU 16000000  // 16 MHz
#define DMX_CHANNELS 12

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>      // TODO: disable with fuses and remove refs
//#include <util/delay.h>  // TODO: use or remove
#include <avr/interrupt.h>

#include "pinout.h"
#include "iocontrol.h"
#include "fakedelay.h"  // TODO: remove?
#include "metaboard.h"  // TODO: remove
#include "spi.h"
#include "usart.h"

dmx_t   dmx = {IDLE, 1, 0, 0, 0};  // TODO: move as static to ISR?
uint8_t databuf[DMX_CHANNELS];


// configure i/o on spi pins to read in settings once
inline void cfg_init (void) {
    /* set_output(SPI_SLAVES_DDR, SPI_CFG_RESET_DDR); */
    /* set_output(SPI_DDR, SPI_SCK_DDR); */
    /* set_output(SPI_SLAVES_DDR, SPI_CFG_SS_DDR); */
    /* set_output(SPI_SLAVES_DDR, SPI_CFG_MODE_DDR); */

    /* cfg_reset_disable(); */
    /* cfg_deselect(); */
}


int main (void) {
    /* uint8_t confl = 0; */
    /* uint8_t confh = 0; */
    uint8_t dmx_channel = 0;  // TODO: not needed, fill array from dmx
    uint8_t tmp;

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

    /* cfg_select();
    /* delay_ms(1);   // maybe redundant, should be one set-up time */

    /* cfg_mode_parallel(); */
    /* delay_ms(1); */

    /* // pulse to read in bits */
    /* output_low(SPI_PORT, SPI_SCK); */
    /* output_high(SPI_PORT, SPI_SCK); */

    /* cfg_mode_serial(); */
    /* delay_ms(1); */

    spi_master_init();
    //SPCR |= (1<<CPOL);  // otherwise can't read first bit

    /* // read in first octet */
    /* ledon(0); */
    /* confl = spi_master_transmit(SPI_TRANSMIT_DUMMY); */
    /* ledoff(0); */

    /* // read in second octet */
    /* ledon(0); */
    /* confh = spi_master_transmit(SPI_TRANSMIT_DUMMY); */
    /* ledoff(0); */

    /* output_high(SPI_PORT, SPI_SCK); */
    /* cfg_deselect(); */
    /* cfg_reset_enable(); */

    usart_init();
    sei();

    while (1) {
	ledon(2);  // debug: start transmit

	// TODO: proper dmx_channel set
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN1);

        // TODO: interrupt on proper phase arbiter
	output_toggle(SPI_SLAVES_PORT, SPI_OUT_SS1);

        // wait 'till slave is ready
	while ( !(SPI_SLAVES_PIN & _BV(SPI_OUT_OK_PIN)) );

	// TODO: transmit proper channel's value
	tmp = spi_master_transmit(databuf[0]);

	// FIXME: what's this? pull-ups on other end?
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN1);

	// TODO: check if transmission successful
	if (tmp != SPI_TRANSMIT_DUMMY) {
	    ledoff(1);
	    delay_ms(990);
	    ledoff(1);
	}
	else delay_ms(40);

	ledoff(2);  // debug: transmitted
    }

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
		if (dmx.slot == 0) dmx.state = DATA; // address == 1
		else dmx.state = SKIP;
	    }
	    break;
	case SKIP:
	    if (--dmx.slot == 0) dmx.state = DATA;
	    break;
	case DATA:
	    databuf[dmx.slot++] = dmx.data;
	    if (dmx.slot == DMX_CHANNELS) dmx.state = IDLE;
	    break;
	case IDLE:
	    break;
	}
    }

    ledoff(0);  // debug: int stop
}
