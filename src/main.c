// xomg's DMX dimmer
// main.c: 

#include "xomg_dmx_dimmer.h"  // board-specific
#include "led.h"
#include "thermo.h"
#include "zc.h"
#include "stp.h"
#include "usart.h"            // dmx

#define DMX_BAUD 250000                 // Hz
#define USART_UBRR F_CPU/16/DMX_BAUD-1  // USART clock gen prescale

void main (void)
{
    // ...

    led_init();              // debugging
    thermo_init();           // temperature control
    zc_init();               // zero crossing
    stp_init();              // serial to parallel
    usart_init(USART_UBRR);  // dmx signals

    // ...

    while (1) {
	// ...
    }
}
