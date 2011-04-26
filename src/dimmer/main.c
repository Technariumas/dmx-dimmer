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
    set_input(ZC_DDR, ZC);     // redundant
    output_high(ZC_PORT, ZC);  // pullup

    MCUCR |= _BV(ISC01);       // int on falling edge
    GIMSK |= _BV(INT0);        // enable INT0 interrupt
}

uint8_t zc_count = 0;
uint16_t zc_dur = 0;           // cycles between two ZCs
uint8_t deg_dur = 0;           // cycles b/w two firing angles
uint8_t angle = 255;           // dmx firing angle (counted backwards!)

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint16_t tmp_zc_dur = 0;   // degree duration (temporary)
    uint16_t old_zc_dur = zc_dur;
    uint8_t tcntl;
    uint8_t tcnth;

    // read counter (used later)
    // low byte must be read first
    tcntl = TCNT1L;
    tcnth = TCNT1H;

    // reset counters (as soon as possible)
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;
    angle = 255;

    // turn off outputs
    output_high(PORTB, PB4);  // TODO: don't use debug led for this

    // read two ints into long int from input capture register
    // low must be read first
    tmp_zc_dur = tcntl;
    zc_dur = tcnth;
    zc_dur = zc_dur << 8;
    zc_dur += tmp_zc_dur;

    // get average of previous and current
    zc_dur = zc_calibrate(old_zc_dur, zc_dur);

    // determine degree duration (there are 256 degrees between 2 ZCs)
    // high byte must be written first
    deg_dur = zc_dur/256;
    OCR1AH = 0;
    OCR1AL = deg_dur;

    zc_count++;  // overflow no problem (TODO: remove)
}

// 
inline void counter1_init (void) {
    // set longest possible degree duration (but only one byte is used)
    // high byte must be written first
    OCR1AH = 0;
    OCR1AL = 0xff;

    TIMSK |= _BV(TOIE1) | _BV(OCIE1A);  // interrupt on overflow
    TCCR1B |= _BV(CS10);                // no prescaler
}

uint8_t channel[4];

// interrupt: on every firing angle, fire matching channels and update
// Output Compare A accordingly
// NOTE: this must take less cycles than deg_dur
ISR (TIMER1_COMPA_vect, ISR_NOBLOCK) {
    uint8_t tmpl;
    uint8_t tmph;
    uint16_t cycles;  // cycles since zc
    
    // fire appropriate channels
    if (channel[0] >= angle) {
	output_low(PORTB, PB4); // TODO: don't use debug led for this
    }
    angle--;  // ASSERT angle != 0

    // get current value of Output Compare
    // low byte must be read first
    tmpl = OCR1AL;
    tmph = OCR1AH;
    cycles = 256*tmph + tmpl;  // cycles since zc

    // calculate new value
    cycles += deg_dur;
    tmph = cycles/256;
    tmpl = cycles & 0x00ff;

    // set calculated value
    // high byte must be written first
    OCR1AH = tmph;
    OCR1AL = tmpl;
}

// interrupt: maximum counter value reached, time interval between two
// ZCs too long
ISR (TIMER1_OVF_vect, ISR_BLOCK) {
    while (1) {
	output_low(PORTB, PB4);
	delay_ms(500);
	output_high(PORTB, PB4);
	delay_ms(500);
    }
}


int main (void) {
    wdt_disable();

    // debug led
    set_output(DDRB, PB4);

    // blink: devboard ok
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */

    zc_init();
    counter1_init();
    sei();

    while (1) {
	// TODO: receive from Master
	channel[0] = 255;

	/* wdt_reset(); */

	/* if (zc_count < 128) { */
	/*     output_low(PORTB, PB4); */
	/* } */
	/* else { */
	/*     output_high(PORTB, PB4); */
	/* } */
    }

    return 1;
}
