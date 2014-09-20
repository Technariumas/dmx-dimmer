// spi.c

#include "spi.h"

#include "master.h"

uint8_t spi_master_transmit (uint8_t data) {
    SPDR = data;                  // start transmission
    while( !(SPSR & _BV(SPIF)) ); // wait for transmission complete 
    return SPDR;
}

