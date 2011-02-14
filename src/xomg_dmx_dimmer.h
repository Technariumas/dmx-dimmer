// xomg's DMX dimmer
// xomg_dmx_dimmer.h:
// settings specific to the board and used components

#define F_CPU 16000000  // in case not set in Makefile
#define DMX_BAUD 250000                 // Hz
#define USART_UBRR F_CPU/16/DMX_BAUD-1  // USART clock gen prescale

// leds for debugging
#define LED0_DDR  DDRD
#define LED0_PORT PORTD
#define LED0      PD7
#define LED1_DDR  DDRD
#define LED1_PORT PORTD
#define LED1      PD6

// comfortables
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)


#include <avr/io.h>
#include "usart.h"


void board_init (void) {
    wdt_disable();

    // debug leds
    set_output(LED0_DDR, LED0);
    set_output(LED1_DDR, LED1);

    //thermo_init();           // temperature control
    //zc_init();               // zero crossing
    //stp_init();              // serial to parallel

    // dmx signals
    usart_init(USART_UBRR);

    sei();
}

void blink (void) {
    output_high(LED0_DDR, LED0);
    delay_ms(50);
    output_low(LED0_DDR, LED0);
    delay_ms(50);
}
