// adc.c

#include "adc.h"

#include <avr/io.h>

inline void adc_init (void) {
    // AVCC /w cap @ AREF, left-adjust result
    ADMUX = _BV(REFS0) | _BV(ADLAR);

    // select default channel
    adc_channel_preheat();

    // disable unused digital inputs (lower power consumption)
    DIDR0 = _BV(ADC5D) | _BV(ADC4D) | _BV(ADC3D) |
	_BV(ADC2D) | _BV(ADC1D) | _BV(ADC0D);

    /* slow ADC down 128 times (compared to system clock) - if trigger
     * source in ADCSRB is left default (i.e. free running mode), this
     * will happen as often as possible; and no one will be a two-knob VJ,
     * so ADC values won't be updated often
     */
    //ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

    // enable auto trigger (according to settings in ADCSRB)
    //ADCSRA |= _BV(ADATE);

    // enable ADC and interrupt
    ADCSRA |= _BV(ADEN) | _BV(ADIE);
}
