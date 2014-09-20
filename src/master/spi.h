// spi.h
// SPI communications
// read settings from dipswitches on panel
// send dmx data to four-channel dimmer arbiters

#ifndef _SPI_H_
#define _SPI_H_

#include <inttypes.h>

#include "iocontrol.h"
#include "master.h"

// data to transmit if you only really want to read data
#define SPI_TRANSMIT_DUMMY 0b01010101

// pin mask to reduce power consumption (pull-ups on other end)
#define SPI_CHAN_RESET 0b11

#define cfg_reset_enable()  output_low(SPI_CFG_PORT, SPI_CFG_RESET)
#define cfg_reset_disable() output_high(SPI_CFG_PORT, SPI_CFG_RESET)
#define cfg_select()        output_low(SPI_PORT, SPI_SS)
#define cfg_deselect()      output_high(SPI_PORT, SPI_SS)
#define cfg_mode_parallel() output_low(SPI_CFG_PORT, SPI_CFG_MODE)
#define cfg_mode_serial()   output_high(SPI_CFG_PORT, SPI_CFG_MODE)

// configure i/o for proper spi transmissions
inline void spi_master_init (void) {
    // debug: pull up so no fake interrupt occurs
    output_high(SPI_SLAVES_PORT, SPI_OUT_SS1);
    output_high(SPI_SLAVES_PORT, SPI_OUT_SS2);
    output_high(SPI_SLAVES_PORT, SPI_OUT_SS3);

    // set up slave select, channel select and "ok i'm ready" lines
    set_output(SPI_SLAVES_DDR, SPI_OUT_CHAN0_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_CHAN1_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_SS1_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_SS2_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_SS3_DDR);
    set_input(SPI_SLAVES_DDR, SPI_OUT_OK1_DDR);
    set_input(SPI_SLAVES_OUTOK_DDR, SPI_OUT_OK2_DDR);
    set_input(SPI_SLAVES_OUTOK_DDR, SPI_OUT_OK3_DDR);

    // pull-ups on inputs (hi-z otherwise)
    input_pullup(SPI_SLAVES_PORT, SPI_OUT_OK1);
    input_pullup(SPI_SLAVES_OUTOK_PORT, SPI_OUT_OK2);
    input_pullup(SPI_SLAVES_OUTOK_PORT, SPI_OUT_OK3);

    // there are pull-ups on slave end, pull high here, too, so no
    // current flows pointlessly
    output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
    output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN1);

    // MOSI, SCK, ~SS are outputs, MISO remains input
    SPI_DDR |= _BV(SPI_MOSI_DDR) | _BV(SPI_SCK_DDR) | _BV(SPI_SS_DDR);
    /* set_output(SPI_DDR, SPI_MOSI_DDR); */
    /* set_output(SPI_DDR, SPI_SCK_DDR); */
    /* set_output(SPI_DDR, SPI_SS_DDR); */
    output_high(SPI_PORT, SPI_SS);  // ~SS inactive

    // Enable SPI, Master, SCK=fosc/128 (TODO: increase SCK)
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
}

// send and/or receive data
uint8_t spi_master_transmit (uint8_t data);

// set slave's addressed dmx channel
inline void spi_chan_select (uint8_t c) {
    switch (c) {
    case 0:
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN1);
	break;
    case 1:
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN1);
	break;
    case 2:
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN1);
	break;
    case 3:
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN1);
	break;
    default: break;
    }
}

// request interrupt on proper phase arbiter
inline void spi_request_interrupt (uint8_t slave) {
    switch (slave) {
    case 0: output_toggle(SPI_SLAVES_PORT, SPI_OUT_SS1); break;
    case 1: output_toggle(SPI_SLAVES_PORT, SPI_OUT_SS2); break;
    case 2: output_toggle(SPI_SLAVES_PORT, SPI_OUT_SS3); break;
    default: break;
    }
}

// this is not proper SPI, but a workaround for 74166's operation mode
inline void cfg_init (void) {
    set_output(SPI_DDR, SPI_SCK_DDR);
    set_output(SPI_DDR, SPI_SS_DDR);
    set_output(SPI_CFG_DDR, SPI_CFG_RESET_DDR);
    set_output(SPI_CFG_DDR, SPI_CFG_MODE_DDR);

    /* cfg_reset_disable(); */
    /* cfg_deselect(); */
}

#endif /* _SPI_H_ */
