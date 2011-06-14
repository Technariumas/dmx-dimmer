// adc.h
// analog-digital converter, used to set preheat and maxval on panel

#ifndef _ADC_H_
#define _ADC_H_

#include <avr/io.h>

#define ADC_MUX_PREHEAT ADC4D  // 0b100
#define ADC_MUX_MAXVAL  ADC5D  // 0b101

// select channel
#define adc_channel_preheat() ADMUX |= ADC_MUX_PREHEAT
#define adc_channel_maxval()  ADMUX |= ADC_MUX_MAXVAL
#define adc_channel_toggle()  ADMUX ^= (ADC_MUX_PREHEAT ^ ADC_MUX_MAXVAL)

// see which channel was actually read (0: preheat, 1: maxval)
#define adc_channel_get() ADMUX & 1

// either convert once; or
// start free running mode (if enabled in adc_init())
#define adc_start() ADCSRA |= _BV(ADSC)

// 
#define adc_running() ADCSRA & _BV(ADSC)

// set up ADC hardware
inline void adc_init (void);

#endif /* _ADC_H_ */
