// slave.h
// 

#ifndef _DIMMER_H_
#define _DIMMER_H_

#include <avr/io.h>

// ===== CONFIG =====

#define F_CPU   6000000  // 6 MHz
#define F_MAINS 50

#define DMX_CHANNELS 4

// ===== PINOUT =====

// leds
#define LEDS_DDR DDRD
#define LED0_DDR DDD0
#define LED1_DDR DDD1

#define LEDS_PORT PORTD
#define LEDBASE   PD0

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
#define SPI_OUT_OK_DDR    DDB0  // ok to transmit
#define SPI_OUT_CHAN1_DDR DDB1
#define SPI_OUT_CHAN0_DDR DDB2
#define SPI_OUT_SS_DDR    DDB3  // PCINT3

#define SPI_OUT_PORT  PORTB
#define SPI_OUT_OK    PB0
#define SPI_OUT_CHAN1 PB1
#define SPI_OUT_CHAN0 PB2
#define SPI_OUT_SS    PB3

#define SPI_OUT_PIN       PINB
#define SPI_OUT_CHAN1_PIN PINB1
#define SPI_OUT_CHAN0_PIN PINB2

// channel dimming
#define DIMMERS_DDR DDRD
#define DIMMER0_DDR DDD3
#define DIMMER1_DDR DDD4
#define DIMMER2_DDR DDD5
#define DIMMER3_DDR DDD6

#define DIMMERS_PORT PORTD
#define DIMMER0      PD3
#define DIMMER1      PD4
#define DIMMER2      PD5
#define DIMMER3      PD6
#define DIMMERBASE   DIMMER0

// ===== FUNCTIONS =====

// leds
#define leds_init() LEDS_DDR |= _BV(LED0_DDR)|_BV(LED1_DDR)
#define led_on(led)     LEDS_PORT &= ~(_BV( LEDBASE+led ))
#define led_off(led)    LEDS_PORT |=   _BV( LEDBASE+led )
#define led_toggle(led) LEDS_PORT ^=   _BV( LEDBASE+led )

// dimmers
#define dimmers_init() DIMMERS_DDR |= _BV(DIMMER0_DDR)|_BV(DIMMER1_DDR)|_BV(DIMMER2_DDR)|_BV(DIMMER3_DDR)
#define dimmer_on(dim)  DIMMERS_PORT |=   _BV( DIMMERBASE+dim )
#define dimmer_off(dim) DIMMERS_PORT &= ~(_BV( DIMMERBASE+dim ))

// fire all channels with at least value 'angle'
inline void fire_channels (uint8_t angle);

#endif /* _DIMMER_H_ */
