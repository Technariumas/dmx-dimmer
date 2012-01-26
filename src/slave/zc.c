// zc.c

#include "zc.h"

#include <inttypes.h>
#include <avr/io.h>

#include "slave.h"
#include "iocontrol.h"

inline uint16_t zc_calibrate (uint16_t old, uint16_t new) {
    if (old == new) return old;
    /* if (old > new)  return (old - (old-new)/8); */
    /* if (old < new) return (old + (new-old)/8); */
    if (old > new) return --old;
    if (old < new) return ++old;
    return new;  // should never happen
}

inline void zc_init (void) {
    set_input(ZC_DDR, ZC);     // interrupt input pin
    output_high(ZC_PORT, ZC);  // pullup

    MCUCR |= _BV(ISC01);       // int on falling edge
    GIMSK |= _BV(INT0);        // enable INT0 interrupt
}

inline void counter0_init (uint8_t deg_dur) {
    TCNT0 = 0;            // reset counter
    OCR0A = deg_dur;
    TIMSK |= _BV(OCIE0A); // Output Compare A int
    TCCR0B |= _BV(CS00);  // no prescaler
}

inline void counter1_init (void) {
    // reset counter
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    TIMSK |= _BV(TOIE1);  // interrupt on overflow
    TCCR1B |= _BV(CS10);  // no prescaler
}
