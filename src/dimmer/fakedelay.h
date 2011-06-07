// fakedelay.h
// uncalibrated delay functions

#ifndef _FAKEDELAY_H_
#define _FAKEDELAY_H_

#include <inttypes.h>

// FIXME: kill time in a calibrated way (NOT!)
void delay_ms (uint16_t ms);

// FIXME: what if wdt is disabled?
void delay_ns (uint16_t ns);

#endif /* _FAKEDELAY_H_ */
