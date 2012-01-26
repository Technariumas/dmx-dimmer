// metaboard.h
// specific to the metaboard

#ifndef _METABOARD_H_
#define _METABOARD_H_

#include <avr/io.h>
#include "iocontrol.h"

#define LEDS_DDR DDRC
#define LED1_DDR DDC3
#define LED2_DDR DDC4
#define LED3_DDR DDC5

#define LEDS_PORT PORTC
#define LEDBASE   PC3

#define leds_init() LEDS_DDR  |= _BV(LED1_DDR)|_BV(LED2_DDR)|_BV(LED3_DDR)
#define ledon(led)     LEDS_PORT |=   _BV( LEDBASE+led )
#define ledoff(led)    LEDS_PORT &= ~(_BV( LEDBASE+led ))
#define ledtoggle(led) LEDS_PORT ^=   _BV( LEDBASE+led )

#endif /* _METABOARD_H_ */
