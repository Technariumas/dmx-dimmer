// adc.h
// analog-digital converter, used to set preheat and maxval on panel

#ifndef _ADC_H_
#define _ADC_H_

#include <avr/io.h>

// MUX3..0 in ADMUX selects ADC channel
// as you see, only one bit differs, this fact can be used in toggling
#define ADMUX_MAXVAL    0b0110
#define ADMUX_PREHEAT   0b0111
#define ADMUX_CHAN_MASK 0b0110
#define ADMUX_CHAN      0b1

// select channel
#define adc_channel_set_maxval()  ADMUX &= ~ADMUX_CHAN
#define adc_channel_set_preheat() ADMUX |=  ADMUX_CHAN
#define adc_channel_toggle()      ADMUX ^=  ADMUX_CHAN

// see which channel was read (0: maxval 1: preheat)
#define adc_channel_which() ADMUX & ADMUX_CHAN

// get adc value
#define adc_get_value() ADCH

// either convert once; or start free running mode
#define adc_start() ADCSRA |= _BV(ADSC)

// 
#define adc_is_running() ADCSRA & _BV(ADSC)

// set up ADC hardware
inline void adc_init (void);

#endif /* _ADC_H_ */
