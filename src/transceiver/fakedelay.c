// fakedelay.c

#include "fakedelay.h"

#include <avr/wdt.h>

void delay_ms (uint16_t ms) {
    uint16_t delay_count = 1000;  // cpu cycles in one ms?
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
	for (i = 0; i != delay_count; i++) wdt_reset();
	ns--;
    }
}
