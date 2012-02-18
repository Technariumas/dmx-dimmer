// spi.c

#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "slave.h"
#include "iocontrol.h"

inline void spi_slave_init (void) {
    // slave select, dmx channel select, transmit ok line
    set_input(SPI_OUT_DDR, SPI_OUT_CHAN0_DDR);
    set_input(SPI_OUT_DDR, SPI_OUT_CHAN1_DDR);
    set_input(SPI_OUT_DDR, SPI_OUT_SS_DDR);
    set_output(SPI_OUT_DDR, SPI_OUT_OK_DDR);
    output_high(SPI_OUT_PORT, SPI_OUT_CHAN0);
    output_high(SPI_OUT_PORT, SPI_OUT_CHAN1);

    // pull-up to remove noise on interrupt pin
    output_high(SPI_OUT_PORT, SPI_OUT_SS);

    // enable external interrupt PCINT3 on PB3
    PCMSK |= _BV(PCINT3);
    GIMSK |= _BV(PCIE);

    // DI and USCK are inputs, DO is output
    set_input(SPI_DDR, SPI_DI_DDR);
    set_input(SPI_DDR, SPI_USCK_DDR);
    set_output(SPI_DDR, SPI_DO_DDR);  // not used

    // three-wire mode (SPI), external clock (SPI mode 0)
    USICR = _BV(USIWM0) | _BV(USICS1) /* | _BV(USICS0) */;
}

char spi_slave_receive (void) {
    USIDR = SPI_TRANSMIT_DUMMY;
    USISR = _BV(USIOIF);  // clear overflow flag (start transmission?)
    while ( !(USISR & _BV(USIOIF)) );  // wait for reception complete
    return USIDR;
}
