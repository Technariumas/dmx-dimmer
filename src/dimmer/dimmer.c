// dmx dimmer, slave module

#include "dimmer.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>       // TODO: disable in hardware and remove
#include <util/delay.h>    // TODO: use or remove
#include <avr/interrupt.h>

#include "iocontrol.h"
#include "fakedelay.h"
#include "spi.h"
#include "zc.h"

// uC cycles between: two zero crossings, two firing angles
#define CYCLES_ZC  F_CPU/(2*F_MAINS)
#define CYCLES_ANG CYCLES_ZC/256

zc_t zc = {CYCLES_ZC, CYCLES_ZC, CYCLES_ANG, 255};

#undef CYCLES_ZC
#undef CYCLES_ANG

// DMX values received from Master
uint8_t chanval[DMX_CHANNELS];


int main (void) {
    uint8_t i = 0;

    wdt_disable();

    // debug leds
    set_output(DDRB, DDB4);
    set_output(DDRD, DDD5);
    output_high(PORTB, PB4);
    output_high(PORTD, PD5);

    // init 
    for (i = 0; i < DMX_CHANNELS; i++) chanval[i] = 0;

    // output channels
    set_output(DDRD, DDD3);  // TODO: pretty pin defines
    /* set_output(DDRD, DDD4); */
    /* set_output(DDRD, DDD5); */
    /* set_output(DDRD, DDD6); */

    // blink: devboard ok
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */


    spi_slave_init();
    counter0_init(zc.deg_dur);
    counter1_init();
    zc_init();
    sei();

    while (1) {
	// wank
	/* output_low(PORTD, PD5); */
	i++;
	/* output_high(PORTD, PD5); */
	/* delay_ms(500); */
    }

    return 1;
}

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint8_t tcntl;
    uint8_t tcnth;

    // disable counter0 ASAP
    TCCR0B &= ~(_BV(CS00));
    TIMSK &= ~(_BV(OCIE0A));

    // turn off outputs
    output_low(PORTD, PD3);
    output_high(PORTB, PB4);  // debug led

    // read counter (used later)
    // low byte must be read first
    tcntl = TCNT1L;
    tcnth = TCNT1H;

    // reset counter1
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    // 
    zc.angle = 255;

    //
    zc.old_dur = zc.dur;

    // read two ints into long int
    zc.dur = tcnth;
    zc.dur = zc.dur << 8;
    zc.dur += tcntl;

    // push old zc_dur towards new
    zc.dur = zc_calibrate(zc.old_dur, zc.dur);

    // determine degree duration (there are 256 degrees between 2 ZCs)
    zc.deg_dur = zc.dur/256;
    OCR0A = zc.deg_dur;

    // reset timer0
    TCNT0 = 0;

    // re-enable counter0
    TIMSK |= _BV(OCIE0A);
    TCCR0B |= _BV(CS00);
}

// interrupt: new firing angle reached
// TODO: fire '255' from ZC int
ISR (TIMER0_COMPA_vect, ISR_BLOCK) {
    uint8_t i;

    // TODO: fire appropriate channels
    for (i = 0; i < DMX_CHANNELS; i++) {
	if (chanval[i] >= zc.angle) {
	    output_high(PORTD, PD3);  // pulse start
	    output_low(PORTB, PB4); // debug led
	}
    }

    if (zc.angle > 1) zc.angle--;  // FIXME: had glitches with ">0"

    TCNT0 = 0;  // reset counter

    //output_low(PORTD, PD3);  // pulse stop
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

    /* output_toggle(PORTD, PD5); */
}

// interrupt: master has new dmx data
ISR (PCINT_vect, ISR_NOBLOCK) {
    output_toggle(PORTD, PD5);
    // TODO: read CHAN0/CHAN1
    USIDR = SPI_TRANSMIT_DUMMY;
    USISR = _BV(USIOIF);                   // clear overflow flag
    output_high(SPI_OUT_PORT, SPI_OUT_OK); // i'm ready!
    while ( !(USISR & _BV(USIOIF)) );      // wait for reception complete
    output_low(SPI_OUT_PORT, SPI_OUT_OK);
    chanval[0] = USIDR;  // TODO: proper channel selection
}
