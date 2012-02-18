// dmx dimmer, slave module

#include "slave.h"

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

// {dur, old_dur, deg_dur, angle}
zc_t zc = {CYCLES_ZC, CYCLES_ZC, CYCLES_ANG, 255};

#undef CYCLES_ZC
#undef CYCLES_ANG

// DMX values received from Master
uint8_t chanval[DMX_CHANNELS];


inline void fire_channels (uint8_t angle) {
    uint8_t c;

    for (c = 0; c < DMX_CHANNELS; c++) {
	if (chanval[c] >= angle) {
	    // dimmer_on is a define, not a function, can't use variable
	    switch (c) {
	    case 0: output_high(DIMMERS_PORT, DIMMER0); break;
	    case 1: output_high(DIMMERS_PORT, DIMMER1); break;
	    case 2: output_high(DIMMERS_PORT, DIMMER2); break;
	    case 3: output_high(DIMMERS_PORT, DIMMER3); break;
	    }
	}
    }
}

int main (void) {
    uint8_t i = 0;

    wdt_disable();

    // debug leds
    leds_init();
    led_off(0);
    led_off(1);

    // default channel values to zero
    for (i = 0; i < DMX_CHANNELS; i++) chanval[i] = 0;

    // output channels
    //dimmers_init();
    set_output(DIMMERS_DDR, DIMMER0_DDR);
    set_output(DIMMERS_DDR, DIMMER1_DDR);
    set_output(DIMMERS_DDR, DIMMER2_DDR);
    set_output(DIMMERS_DDR, DIMMER3_DDR);

    // submit to slavery
    spi_slave_init();

    // 
    degree_duration_counter_init(zc.deg_dur);
    zc_duration_counter_init();
    zc_init();

    sei();

    while (1) i++;  // everything else is interrupt-driven

    return 1;
}

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint8_t tcntl;
    uint8_t tcnth;

    // time-critical: disable counter0 ASAP and turn off all outputs
    TCCR0B &= ~(_BV(CS00));
    TIMSK &= ~(_BV(OCIE0A));

    output_low(DIMMERS_PORT, DIMMER0);
    output_toggle(DIMMERS_PORT, DIMMER1);
    output_low(DIMMERS_PORT, DIMMER2);
    output_low(DIMMERS_PORT, DIMMER3);

    // even if some ZCs were missed before, they aren't now
    led_off(0);

    // read counter (used later)
    // low byte must be read first
    tcntl = TCNT1L;
    tcnth = TCNT1H;

    // reset counter1
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    // reset current angle counter 
    zc.angle = 255;

    // save old value for calibration
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

    // first angle interrupt will happen on '254', so do '255' now
    fire_channels(255);
}

// interrupt: new firing angle reached
ISR (TIMER0_COMPA_vect, ISR_BLOCK) {
    fire_channels(zc.angle);
    if (zc.angle > 1) zc.angle--;

    TCNT0 = 0;  // reset counter
}

// interrupt: maximum counter value reached, time interval between two
// ZCs too long
ISR (TIMER1_OVF_vect, ISR_NOBLOCK) {
    // this is an unwanted situation, turn red led on
    led_on(0);
}

// interrupt: master has new dmx data
ISR (PCINT_vect, ISR_NOBLOCK) {
    uint8_t chan = 0;
    uint8_t tmp = 0;

    led_on(1);

    // read CHAN0/CHAN1
    //chan = _BV(SPI_OUT_CHAN0_PIN) + (_BV(SPI_OUT_CHAN1_PIN) << 1);
    /* tmp = SPI_OUT_PIN; */
    /* if (tmp & _BV(SPI_OUT_CHAN0_PIN)) chan += 0b01; */
    /* if (tmp & _BV(SPI_OUT_CHAN1_PIN)) chan += 0b10; */

    USIDR = SPI_TRANSMIT_DUMMY;
    USISR = _BV(USIOIF);                   // clear overflow flag
    output_high(SPI_OUT_PORT, SPI_OUT_OK); // i'm ready!
    while ( !(USISR & _BV(USIOIF)) );      // wait for reception complete
    output_low(SPI_OUT_PORT, SPI_OUT_OK);

    chanval[chan] = USIDR;

    led_off(1);
}
