// spi.h
// SPI communications
// read settings from dipswitches on panel
// send dmx data to four-channel dimmer arbiters

#ifndef _SPI_H_
#define _SPI_H_

#include <inttypes.h>

#include "iocontrol.h"
#include "transceiver.h"

// data to transmit if you only really want to read data
#define SPI_TRANSMIT_DUMMY 0b01010101

// FIXME: what's this? pull-ups on other end?
#define SPI_CHAN_RESET 3

#define cfg_reset_enable()  output_low(SPI_SLAVES_PORT, SPI_CFG_RESET)
#define cfg_reset_disable() output_high(SPI_SLAVES_PORT, SPI_CFG_RESET)
#define cfg_select()        output_low(SPI_SLAVES_PORT, SPI_CFG_SS)
#define cfg_deselect()      output_high(SPI_SLAVES_PORT, SPI_CFG_SS)
#define cfg_mode_parallel() output_low(SPI_SLAVES_PORT, SPI_CFG_MODE)
#define cfg_mode_serial()   output_high(SPI_SLAVES_PORT, SPI_CFG_MODE)

// configure i/o for proper spi transmissions
inline void spi_master_init (void);

// send and/or receive data
uint8_t spi_master_transmit (uint8_t data);

// set slave's addressed dmx channel
inline void spi_chan_select (uint8_t c);

// request interrupt on proper phase arbiter
inline void spi_request_interrupt (uint8_t slave);

// this is not proper SPI, but a workaround for 74166's operation mode
inline void cfg_init (void);

#endif /* _SPI_H_ */
