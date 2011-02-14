/* led.h: debug led control */

/* The following defines must be made in some board-specific header:
 * #define LED0_DDR    DDRn
 * #define LED0_PORT   PORTn
 * #define LED0        PDn
 * #define LED1_DDR  DDRn
 * #define LED1_PORT PORTn
 * #define LED1      PDn   */

#include <avr/io.h>

#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

void led_init (void) {
    set_output(LED0_DDR, LED0);
    set_output(LED1_DDR, LED1);
}

void led_red_blink (void) {
    output_high(LED0_DDR, LED0);
    delay_ms(50);
    output_low(LED0_DDR, LED0);
    delay_ms(50);
}

void led_green_blink (void) {
    output_high(LED1_DDR, LED1);
    delay_ms(50);
    output_low(LED1_DDR, LED1);
    delay_ms(50);
}
