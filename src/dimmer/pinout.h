// pinout.h
// pin connections around Slave (ATtiny2313)

#ifndef _PINOUT_H_
#define _PINOUT_H_

// zero crossing
#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC      PD2

// SPI
#define SPI_DDR      DDRB
#define SPI_DI_DDR   DDB5  // data in  (MOSI)
#define SPI_DO_DDR   DDB6  // data out (MISO)
#define SPI_USCK_DDR DDB7  // clock    (SCK)

#define SPI_PORT PORTB
#define SPI_DI   PB5
#define SPI_DO   PB6
#define SPI_USCK PB7

#define SPI_OUT_DDR       DDRB  // link to SPI Master
#define SPI_OUT_SS_DDR    DDB0  // PCINT0
#define SPI_OUT_CHAN0_DDR DDB1
#define SPI_OUT_CHAN1_DDR DDB2
#define SPI_OUT_OK_DDR    DDB3  // ok to transmit

#define SPI_OUT_PORT  PORTB
#define SPI_OUT_SS    PB0
#define SPI_OUT_CHAN0 PB1
#define SPI_OUT_CHAN1 PB2
#define SPI_OUT_OK    PB3

#endif /* _PINOUT_H_ */
