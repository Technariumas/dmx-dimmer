// adc.c

#include "adc.h"

#include <avr/io.h>

inline void adc_init (void) {
    // AVCC /w cap @ AREF, left-adjust result (we're using just 8 bits)
    ADMUX = _BV(REFS0) | _BV(ADLAR);

    // we're only using ADC6 and ADC7, set ADC channel mask
    ADMUX |= ADMUX_CHAN_MASK;

    // select default channel
    adc_channel_set_preheat();

    /* slow ADC down 128 times (compared to system clock) - if trigger
     * source in ADCSRB is left default (i.e. free running mode), this
     * will happen as often as possible; and no one will be a two-knob VJ,
     * so ADC values won't be updated often
     */
    /* ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); */

    // enable auto trigger
    // (according to default settings in ADCSRB - i.e. free-running mode)
    /* ADCSRA |= _BV(ADATE); */

    // enable ADC and interrupt
    ADCSRA |= _BV(ADEN) | _BV(ADIE);
}
