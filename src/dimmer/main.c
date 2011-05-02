// dmx dimmer

#define F_CPU 6000000  // 6 MHz

#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>  // TODO: use or remove
#include <avr/interrupt.h>


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

void delay_ns (uint16_t ns) {
    uint16_t delay_count = 1;  // cpu cycles in one ms?
    volatile uint16_t i;

    while (ns != 0) {
	for (i = 0; i != delay_count; i++)
	    wdt_reset();  // FIXME: what if wdt is disabled?
	ns--;
    }
}

// push old value of zc duration towards new value
uint16_t zc_calibrate (uint16_t old, uint16_t new) {
    if (old == new) return old;
    if (old > new)  return (old - (old-new)/8);
    if (old < new) return (old + (new-old)/8);
    return new;  // should never happen
}

// zero crossing
#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC      PD2

// 
inline void zc_init (void) {
    // interrupt input pin
    set_input(ZC_DDR, ZC);     // redundant
    output_high(ZC_PORT, ZC);  // pullup

    MCUCR |= _BV(ISC01);       // int on falling edge
    GIMSK |= _BV(INT0);        // enable INT0 interrupt
}

uint16_t zc_dur = 60000;  // cycles between two ZCs (ideal @ 50 Hz)
uint8_t deg_dur = 234;    // cycles b/w two firing angles (ideal @ 50 Hz)
uint8_t angle = 255;      // dmx firing angle (counted backwards!)

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint16_t old_zc_dur = zc_dur;
    uint8_t tcntl;
    uint8_t tcnth;

    // disable counter0 ASAP
    TCCR0B &= ~(_BV(CS00));

    // read counter (used later)
    // low byte must be read first
    tcntl = TCNT1L;
    tcnth = TCNT1H;

    // reset counter1
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    // turn off outputs
    output_high(PORTB, PB4);  // TODO: don't use debug led for this

    // 
    angle = 255;

    // read two ints into long int
    zc_dur = tcnth;
    zc_dur = zc_dur << 8;
    zc_dur += tcntl;

    // push old zc_dur towards new
    //zc_dur = zc_calibrate(old_zc_dur, zc_dur);

    // determine degree duration (there are 256 degrees between 2 ZCs)
    deg_dur = zc_dur/256;
    OCR0A = deg_dur;

    // re-enable counter0
    TCCR0B |= _BV(CS00);
}

// degree duration counter
inline void counter0_init (void) {
    TCNT0 = 0;            // reset counter
    OCR0A = deg_dur;
    TIMSK |= _BV(OCIE0A); // Output Compare A int
    TCCR0B |= _BV(CS00);  // no prescaler
}

uint8_t channel[4];

// interrupt: new firing angle reached
// TODO: fire '255' from ZC int
ISR (TIMER0_COMPA_vect, ISR_NOBLOCK) {
    // fire appropriate channels
    if (channel[0] >= angle) {
	output_low(PORTB, PB4); // TODO: don't use debug led for this
    }

    if (angle != 0) angle--;

    TCNT0 = 0;  // reset counter
}

// zc duration counter
inline void counter1_init (void) {
    // reset counter
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    TIMSK |= _BV(TOIE1);  // interrupt on overflow
    TCCR1B |= _BV(CS10);  // no prescaler
}

// interrupt: maximum counter value reached, time interval between two
// ZCs too long
ISR (TIMER1_OVF_vect, ISR_NOBLOCK) {
    /* while (1) { */
    /* 	output_low(PORTB, PB4); */
    /* 	delay_ms(500); */
    /* 	output_high(PORTB, PB4); */
    /* 	delay_ms(500); */
    /* } */
    if (PORTD & 0b00010000) {
	output_low(PORTD, PD5);
    }
    else {
	output_high(PORTD, PD5);
    }
}


int main (void) {
    wdt_disable();

    // debug leds
    set_output(DDRB, DDB4);
    set_output(DDRD, DDD5);

    // channel selector
    set_input(DDRD, DDD0);
    set_input(DDRD, DDD1);
    output_high(PORTD, PD0);
    output_high(PORTD, PD1);

    // blink: devboard ok
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */


    counter0_init();
    counter1_init();
    zc_init();
    sei();

    while (1) {
	// TODO: receive channel values from Master
	channel[0] = (PIND & 0b00000011) * 84 + 3;

	/* if (zc_count < 128) { */
	/*     output_low(PORTB, PB4); */
	/* } */
	/* else { */
	/*     output_high(PORTB, PB4); */
	/* } */
    }

    return 1;
}
