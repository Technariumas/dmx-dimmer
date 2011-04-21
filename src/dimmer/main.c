// dmx dimmer

#define F_CPU 6000000  // 6 MHz

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>


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


int main (void) {
    set_output(DDRB, PB4);

    while (1) {
	wdt_reset();

	output_low(PORTB, PB4);
	delay_ms(500);
	output_high(PORTB, PB4);
	delay_ms(500);
    }

    return 1;
}
