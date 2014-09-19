// dmx dimmer, slave module

#include "slave.h"

#include <inttypes.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>    // TODO: use or remove
#include <avr/interrupt.h>

#include "iocontrol.h"
#include "fakedelay.h"
#include "spi.h"
#include "zc.h"

#define LED_GREEN 0
#define LED_RED 1

// uC cycles between: two zero crossings, two firing angles
#define CYCLES_ZC  F_CPU/(2*F_MAINS)
#define CYCLES_ANG CYCLES_ZC/256

// {dur, old_dur, deg_dur, angle}
volatile zc_t zc = {CYCLES_ZC, CYCLES_ZC, CYCLES_ANG, 255};

#undef CYCLES_ZC
#undef CYCLES_ANG

// DMX values received from Master
volatile uint8_t chanval[DMX_CHANNELS];


inline void fire_channels (uint8_t angle) {
    if (chanval[0] >= angle) dimmer_on(0);
    if (chanval[1] >= angle) dimmer_on(1);
    if (chanval[2] >= angle) dimmer_on(2);
    if (chanval[3] >= angle) dimmer_on(3);
}

int get_selected_state(void) {
    return SPI_OUT_PIN & _BV(SPI_OUT_SS_PIN);
}

int main (void) {
    uint8_t i = 0;

    wdt_disable();

    // debug leds
    leds_init();
    led_off(LED_RED);  // red - error (TODO: use #define)
    led_off(LED_GREEN);   // green - data transmission (TODO: use #define)

    // default channel values to zero
    for (i = 0; i < DMX_CHANNELS; i++) chanval[i] = 0;

    // output channels
    dimmers_init();

    // submit to slavery
    spi_slave_init();

    // 
    degree_duration_counter_init(zc.deg_dur);
    zc_duration_counter_init();
    zc_init();

    sei();
    
    int previous_selected_state;
    while (1) {
        led_on(LED_RED);

        // Data transmission is started when 'selected state' changes.
        previous_selected_state = get_selected_state();
        while (previous_selected_state == get_selected_state());

        led_off(LED_RED);

        // read CHAN0/CHAN1
        uint8_t chan0 = !!(SPI_OUT_PIN & _BV(SPI_OUT_CHAN0_PIN));
        uint8_t chan1 = !!(SPI_OUT_PIN & _BV(SPI_OUT_CHAN1_PIN));
        uint8_t chan = (chan1 << 1) | chan0;

        // chosen slave gets to talk on MISO line
        set_output(SPI_DDR, SPI_DO_DDR);

        led_on(LED_GREEN);

        USIDR = SPI_TRANSMIT_DUMMY;
        USISR = _BV(USIOIF);                   // clear overflow flag
        output_high(SPI_OUT_PORT, SPI_OUT_OK); // i'm ready!
        while ( !(USISR & _BV(USIOIF)) );      // wait for reception complete

        led_off(LED_GREEN);

        chanval[chan] = USIDR;

        // release MISO line
        set_input(SPI_DDR, SPI_DO_DDR);
        input_hiz(SPI_PORT, SPI_DO);

        output_low(SPI_OUT_PORT, SPI_OUT_OK);
    }

    return 1;
}

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint8_t tcntl;
    uint8_t tcnth;

    // time-critical: disable degree duration counter
    degree_duration_counter_stop();

    // time-critical: turn off all outputs
    dimmer_off(0);
    dimmer_off(1);
    dimmer_off(2);
    dimmer_off(3);

    // see how many cycles passed since previous ZC (used later)
    // low byte must be read first
    tcntl = TCNT1L;
    tcnth = TCNT1H;

    // reset counter
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    // save old value for calibration
    zc.old_dur = zc.dur;

    // read two ints into long int
    // typeof(tcnth) == uint8_t
    zc.dur = tcnth;
    zc.dur = zc.dur << 8;
    zc.dur += tcntl;

    // push old zc_dur towards new
    zc.dur = zc_calibrate(zc.old_dur, zc.dur);

    /* determine degree duration (there are 256 degrees between 2 ZCs);
     * if some low-value angles were not reached due to duration being
     * too long, take them into account and reduce the duration further
     */
    zc.deg_dur = zc.dur/256;

    // reset current-angle counter
    zc.angle = 255;

    // reset and re-enable degree duration timer/counter
    degree_duration_counter_init(zc.deg_dur);

    // first angle interrupt will happen on '254', so do '255' now
    fire_channels(255);
}

// interrupt: new firing angle reached
ISR (TIMER0_COMPA_vect, ISR_BLOCK) {
    if (zc.angle != 0) fire_channels(zc.angle);
    if (zc.angle > 0) zc.angle--;
}

// interrupt: maximum counter value reached, time interval between two
// ZCs too long
ISR (TIMER1_OVF_vect, ISR_NOBLOCK) {
    // this is an unwanted situation, turn red led on
    /* dimmer_off(0); */
    /* dimmer_off(1); */
    /* dimmer_off(2); */
    /* dimmer_off(3); */
}
