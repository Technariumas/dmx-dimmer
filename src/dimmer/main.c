// dmx dimmer

#define F_CPU 6000000  // 6 MHz

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>


#define output_low(port,pin)  port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin)  portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |=  (1<<pin)


// FIXME: kill time in a calibrated way (NOT!)
void delay_ms (uint16_t ms) {
    uint16_t delay_count = 333;  // FIXME
    volatile uint16_t i;

    while (ms != 0) {
	for (i = 0; i != delay_count; i++) wdt_reset();
	ms--;
    }
}

// FIXME: what if wdt is disabled?
void delay_ns (uint16_t ns) {
    uint16_t delay_count = 1;  // cpu cycles in one ms?
    volatile uint16_t i;

    while (ns != 0) {
	for (i = 0; i != delay_count; i++)
	    wdt_reset();
	ns--;
    }
}

// zero crossing
#include <avr/interrupt.h>

#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC      PD2

inline void zc_init (void) {
    set_input(ZC_DDR, ZC);     // redundant
    output_high(ZC_PORT, ZC);  // pullup

    MCUCR |= _BV(ISC01);       // int on falling edge
    GIMSK |= _BV(INT0);        // enable INT0 interrupt
}

uint8_t zc_count = 0;
/* uint8_t outcount = 0; */

// interrupt service routine: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    zc_count++;  // overflow no problem
    /* if (zc_count >= 100) { */
    /* 	//outcount++; */
    /* 	zc_count = 0; */
    /* } */
}

inline void timer1_init (void) {
    OCR1AL = 100;
    //OCR1AH = 0;

    TCCR1B |= _BV(CS10);  // no prescaler
}

// interrupt: just for timer testing
ISR (TIMER1_COMPA_vect, ISR_NOBLOCK) {
    TCNT1L = 0;  // FIXME: can be done using TCCR1B instead
}


int main (void) {
    wdt_disable();

    // debug led
    set_output(DDRB, PB4);

    output_low(PORTB, PB4);
    delay_ms(200);
    output_high(PORTB, PB4);
    delay_ms(200);
    output_low(PORTB, PB4);
    delay_ms(200);
    output_high(PORTB, PB4);
    delay_ms(200);

    zc_init();
    timer1_init();
    sei();

    while (1) {
	/* wdt_reset(); */

	if (zc_count < 128) {
	    output_low(PORTB, PB4);
	}
	else {
	    output_high(PORTB, PB4);
	}
    }

    return 1;
}
