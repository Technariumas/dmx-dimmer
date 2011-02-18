// xomg's DMX dimmer
// xomg_dmx_dimmer.h:
// settings specific to the board hardware and used components

#define DMX_CHANNELS 8

#define F_CPU 16000000              // in case not set in Makefile
#define BAUD 250000                 // Hz
#define USART_UBR F_CPU/16/BAUD-1   // USART clock gen prescale
// FIXME: it is also possible to set baud rate using util/setbaud.h

// leds for debugging
#define LEDR_DDR  DDRB
#define LEDR_PORT PORTB
#define LEDR      PB0
#define LEDB_DDR  DDRB
#define LEDB_PORT PORTB
#define LEDB      PB1

// dipswitches to select dmx_start_address
#define DSA0_DDR DDRB
#define DSA1_DDR DDRB
#define DSA2_DDR DDRB
#define DSA3_DDR DDRB
#define DSA4_DDR DDRC
#define DSA5_DDR DDRC
#define DSA6_DDR DDRC
#define DSA7_DDR DDRC
#define DSA8_DDR DDRC
#define DSA9_DDR DDRC
#define DSA0_PORT PORTB
#define DSA1_PORT PORTB
#define DSA2_PORT PORTB
#define DSA3_PORT PORTB
#define DSA4_PORT PORTC
#define DSA5_PORT PORTC
#define DSA6_PORT PORTC
#define DSA7_PORT PORTC
#define DSA8_PORT PORTC
#define DSA9_PORT PORTC
#define DSA0 PB2
#define DSA1 PB3
#define DSA2 PB4
#define DSA3 PB5
#define DSA4 PC0
#define DSA5 PC1
#define DSA6 PC2
#define DSA7 PC3
#define DSA8 PC4
#define DSA9 PC5

// zero crossing
#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC_PIN  PIND   // not used
#define ZC      PD3

// serial to parallel
#define STP_DATA_DDR    DDRB
#define STP_DATA_PORT   PORTB
#define STP_DATA        PB1
#define STP_CLK_DDR     DDRB
#define STP_CLK_PORT    PORTB
#define STP_CLK         PB0
#define STP_NOUTEN_DDR  DDRD  // ~OUTEN
#define STP_NOUTEN_PORT PORTD
#define STP_NOUTEN      PD4

// DMX/USART
#define USART_DDR  DDRD
#define USART_PORT PORTD
#define USART_NRE   PD2  // ~RE pin


// data structures
typedef struct {
    bool     error;           // last frame had an error
    uint16_t startaddr;
    uint16_t slot;            // slot counter
    uint8_t  status;
    uint8_t  data;
} dmx_t;


// comfortables
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usart.h"


// init hardware
void init_board (void) {
    wdt_disable();

    init_leds(); // debug leds
    init_dsa();  // dmx_start_address retrieval
    init_zc();   // zero crossing
    init_stp();  // serial to parallel (output to controlled lights)
    init_dmx();  // dmx (usart) signals

    sei();
}

// retrieve software configuration from hardware interfaces
void get_conf (void) {
}

inline void init_pins (void) {
    set_output(LEDR_DDR, LEDR);
    set_output(LEDB_DDR, LEDB);
}

inline void init_dsa (void) {
    set_input(DSA0_DDR, DSA0);
    set_input(DSA1_DDR, DSA1);
    set_input(DSA2_DDR, DSA2);
    set_input(DSA3_DDR, DSA3);
    set_input(DSA4_DDR, DSA4);
    set_input(DSA5_DDR, DSA5);
    set_input(DSA6_DDR, DSA6);
    set_input(DSA7_DDR, DSA7);
    set_input(DSA8_DDR, DSA8);
    set_input(DSA9_DDR, DSA9);
    output_high(DSA0_PORT, DSA0);
    output_high(DSA1_PORT, DSA1);
    output_high(DSA2_PORT, DSA2);
    output_high(DSA3_PORT, DSA3);
    output_high(DSA4_PORT, DSA4);
    output_high(DSA5_PORT, DSA5);
    output_high(DSA6_PORT, DSA6);
    output_high(DSA7_PORT, DSA7);
    output_high(DSA8_PORT, DSA8);
    output_high(DSA9_PORT, DSA9);
}

inline void init_zc (void) {
    set_input(ZC_DDR, ZC);
    output_high(ZC_PORT, ZC);
    EIMSK |= _BV(INT1);
    EICRA |= _BV(ISC11) /*| _BV(ISC10)*/;  // int on falling edge
}

inline void init_stp (void) {
    set_output(STP_DATA_DDR, STP_DATA);
    set_output(STP_CLK_DDR, STP_CLK);
    set_output(STP_NOUTEN_DDR, STP_NOUTEN);
    output_low(STP_DATA_PORT, STP_DATA);
    output_low(STP_CLK_PORT, STP_CLK);
    output_high(STP_NOUTEN_PORT, STP_NOUTEN);
}

inline void init_dmx (void) {
    set_output(USART_DDR, USART_NRE);
    output_low(USART_DDR, USART_NRE);
    usart_init(USART_UBR);
}

void blinkr (void) {
    output_high(LEDR_DDR, LEDR);
    _delay_ms(100); // FIXME: these waits block!
    output_low(LEDR_DDR, LEDR);
    _delay_ms(100);
}

void blinkg (void) {
    output_high(LEDB_DDR, LEDB);
    _delay_ms(100);
    output_low(LEDB_DDR, LEDB);
    _delay_ms(100);
}
