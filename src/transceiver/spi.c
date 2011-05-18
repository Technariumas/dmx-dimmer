// spi.c

#include "spi.h"

inline void spi_master_init (void) {
    // 
    set_output(SPI_SLAVES_DDR, SPI_OUT_CHAN0_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_CHAN1_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_SS1_DDR);
    /* set_output(SPI_SLAVES_DDR, SPI_OUT_SS2_DDR); */
    /* set_output(SPI_SLAVES_DDR, SPI_OUT_SS3_DDR); */
    set_input(SPI_SLAVES_DDR, SPI_OUT_OK_DDR);

    // MOSI, SCK, ~SS are outputs, MISO is left input
    SPI_DDR |= _BV(SPI_MOSI_DDR) | _BV(SPI_SCK_DDR) | _BV(SPI_SS_DDR);
    output_high(SPI_PORT, SPI_SS);  // ~SS inactive

    // Enable SPI, Master, SCK=fosc/128 (TODO: increase SCK)
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
}

uint8_t spi_master_transmit (uint8_t data) {
    SPDR = data;                  // start transmission
    while( !(SPSR & _BV(SPIF)) ); // wait for transmission complete 
    return SPDR;
}
