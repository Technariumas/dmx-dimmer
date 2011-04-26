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

// calculate average of two long ints
uint16_t long_avg (uint16_t a, uint16_t b) {
    if (a == b) return a;
    if (a > b)	return (b + (a-b)/2);
    if (a < b) 	return (a + (b-a)/2);

    return b;  // should never happen
}

// zero crossing
#include <avr/interrupt.h>

#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC      PD2

// 
inline void zc_init (void) {
    set_input(ZC_DDR, ZC);     // redundant
    output_high(ZC_PORT, ZC);  // pullup

    MCUCR |= _BV(ISC01);       // int on falling edge
    GIMSK |= _BV(INT0);        // enable INT0 interrupt
}

uint8_t zc_count = 0;
uint16_t zc_dur = 0;           // cycles between two ZCs

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint16_t tmp_zc_dur = 0;   // degree duration (temporary)
    uint16_t old_zc_dur = zc_dur;

    // reset counter
    TCNT1L = 0;

    // read two ints into long int from input capture register
    // low must be read first
    tmp_zc_dur = ICR1L;
    zc_dur = ICR1H;
    zc_dur = zc_dur << 8;
    zc_dur += tmp_zc_dur;

    // get average of previous and current
    zc_dur = long_avg(old_zc_dur, zc_dur);

    // determine degree duration (there are 256 degrees between 2 ZCs)
    TCNT0 = zc_dur/256;

    zc_count++;  // overflow no problem (TODO: remove)
}

// 
inline void counter1_init (void) {
    TIMSK |= _BV(TOIE1);  // interrupt on overflow
    TCCR1B |= _BV(CS10);  // no prescaler
}

// interrupt: maximum counter value reached, time interval between two
// ZCs too long
ISR (TIMER1_OVF_vect, ISR_NOBLOCK) {
    // TODO: error handling
}


int main (void) {
    wdt_disable();

    // debug led
    set_output(DDRB, PB4);

    // blink: devboard ok
    output_low(PORTB, PB4);
    delay_ms(200);
    output_high(PORTB, PB4);
    delay_ms(200);
    output_low(PORTB, PB4);
    delay_ms(200);
    output_high(PORTB, PB4);
    delay_ms(200);

    zc_init();
    counter1_init();
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
