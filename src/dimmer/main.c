// dmx dimmer

#define F_CPU 6000000  // 6 MHz

#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>    // TODO: use or remove
#include <avr/interrupt.h>

#define set_input(portdir,pin)  portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |=  (1<<pin)
#define output_low(port,pin)    port &= ~(1<<pin)
#define output_high(port,pin)   port |=  (1<<pin)
#define output_toggle(port,pin) port ^=  (1<<pin)

// FIXME: kill time in a calibrated way (NOT!)
void delay_ms (uint16_t ms) {
    uint16_t delay_count = 333;  // FIXME
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
	for (i = 0; i != delay_count; i++)
	    wdt_reset();  // FIXME: what if wdt is disabled?
	ns--;
    }
}

// push old value of zc duration towards new value
inline uint16_t zc_calibrate (uint16_t old, uint16_t new) {
    if (old == new) return old;
    /* if (old > new)  return (old - (old-new)/8); */
    /* if (old < new) return (old + (new-old)/8); */
    if (old > new) return --old;
    if (old < new) return ++old;
    return new;  // should never happen
}

// zero crossing
#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC      PD2

// 
inline void zc_init (void) {
    // interrupt input pin
    set_input(ZC_DDR, ZC);     // redundant
    output_high(ZC_PORT, ZC);  // pullup

    MCUCR |= _BV(ISC01);       // int on falling edge
    GIMSK |= _BV(INT0);        // enable INT0 interrupt
}

uint16_t old_zc_dur = 60000;
uint16_t zc_dur = 60000;  // cycles between two ZCs (ideal @ 50 Hz)
uint8_t deg_dur = 234;    // cycles b/w two firing angles (ideal @ 50 Hz)
uint8_t angle = 255;      // dmx firing angle (counted backwards!)

// interrupt: action to take on zero crossing
ISR (INT0_vect, ISR_NOBLOCK) {
    uint8_t tcntl;
    uint8_t tcnth;

    // disable counter0 ASAP
    TCCR0B &= ~(_BV(CS00));
    TIMSK &= ~(_BV(OCIE0A));

    // turn off outputs
    output_low(PORTD, PD3);
    output_high(PORTB, PB4);  // debug led

    // read counter (used later)
    // low byte must be read first
    tcntl = TCNT1L;
    tcnth = TCNT1H;

    // reset counter1
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    // 
    angle = 255;

    //
    old_zc_dur = zc_dur;

    // read two ints into long int
    zc_dur = tcnth;
    zc_dur = zc_dur << 8;
    zc_dur += tcntl;

    // push old zc_dur towards new
    zc_dur = zc_calibrate(old_zc_dur, zc_dur);

    // determine degree duration (there are 256 degrees between 2 ZCs)
    deg_dur = zc_dur/256;
    OCR0A = deg_dur;

    // reset timer0
    TCNT0 = 0;

    // re-enable counter0
    TIMSK |= _BV(OCIE0A);
    TCCR0B |= _BV(CS00);
}

// degree duration counter
inline void counter0_init (void) {
    TCNT0 = 0;            // reset counter
    OCR0A = deg_dur;
    TIMSK |= _BV(OCIE0A); // Output Compare A int
    TCCR0B |= _BV(CS00);  // no prescaler
}

uint8_t channel[4];

// interrupt: new firing angle reached
// TODO: fire '255' from ZC int
ISR (TIMER0_COMPA_vect, ISR_BLOCK) {
    // fire appropriate channels
    if (channel[0] >= angle) {
	output_high(PORTD, PD3);  // pulse start
	output_low(PORTB, PB4); // debug led
    }

    if (angle > 1) angle--;

    TCNT0 = 0;  // reset counter

    //output_low(PORTD, PD3);  // pulse stop
}

// zc duration counter
inline void counter1_init (void) {
    // reset counter
    // high byte must be written first
    TCNT1H = 0;
    TCNT1L = 0;

    TIMSK |= _BV(TOIE1);  // interrupt on overflow
    TCCR1B |= _BV(CS10);  // no prescaler
}

// interrupt: maximum counter value reached, time interval between two
// ZCs too long
ISR (TIMER1_OVF_vect, ISR_NOBLOCK) {
    /* while (1) { */
    /* 	output_low(PORTB, PB4); */
    /* 	delay_ms(500); */
    /* 	output_high(PORTB, PB4); */
    /* 	delay_ms(500); */
    /* } */

    /* output_toggle(PORTD, PD5); */
}


#define SPI_DDR      DDRB
#define SPI_DI_DDR   DDB5  // data in  (MOSI)
#define SPI_DO_DDR   DDB6  // data out (MISO)
#define SPI_USCK_DDR DDB7  // clock    (SCK)

#define SPI_PORT PORTB
#define SPI_DI   PB5
#define SPI_DO   PB6
#define SPI_USCK PB7

#define SPI_OUT_DDR       DDRB  // link to SPI Master
#define SPI_OUT_CHAN0_DDR DDB0
#define SPI_OUT_CHAN1_DDR DDB1
#define SPI_OUT_SS_DDR    DDB2  // PCINT2
#define SPI_OUT_OK_DDR    DDB3  // ok to transmit

#define SPI_OUT_PORT  PORTB
#define SPI_OUT_CHAN0 PB0
#define SPI_OUT_CHAN1 PB1
#define SPI_OUT_SS    PB2
#define SPI_OUT_OK    PB3


//
inline void spi_slave_init (void) {
    // slave select, dmx channel select, transmit ok line
    set_input(SPI_OUT_DDR, SPI_OUT_CHAN0_DDR);
    set_input(SPI_OUT_DDR, SPI_OUT_CHAN1_DDR);
    set_input(SPI_OUT_DDR, SPI_OUT_SS_DDR);
    set_output(SPI_OUT_DDR, SPI_OUT_OK_DDR);
    output_high(SPI_OUT_PORT, SPI_OUT_CHAN0);
    output_high(SPI_OUT_PORT, SPI_OUT_CHAN1);

    // enable external interrupt PCINT2 on PB2
    PCMSK |= _BV(PCINT2);
    GIMSK |= _BV(PCIE);

    // DI and USCK are inputs, DO is output
    set_input(SPI_DDR, SPI_DI_DDR);
    set_input(SPI_DDR, SPI_USCK_DDR);
    set_output(SPI_DDR, SPI_DO_DDR);  // not used

    // three-wire mode (SPI), external clock (SPI mode 0)
    USICR = _BV(USIWM0) | _BV(USICS1) /* | _BV(USICS0) */;
}

// data to transmit when you only want to receive
#define SPI_TRANSMIT_DUMMY 0b01010101

// TODO: remove if unused
char spi_slave_receive (void) {
    USIDR = SPI_TRANSMIT_DUMMY;
    USISR = _BV(USIOIF);  // clear overflow flag (start transmission?)
    while ( !(USISR & _BV(USIOIF)) );  // wait for reception complete
    return USIDR;
}

// interrupt: master has new dmx data
ISR (PCINT_vect, ISR_NOBLOCK) {
    output_toggle(PORTD, PD5);
    // TODO: read CHAN0/CHAN1
    USIDR = SPI_TRANSMIT_DUMMY;
    USISR = _BV(USIOIF);                   // clear overflow flag
    output_high(SPI_OUT_PORT, SPI_OUT_OK); // i'm ready!
    while ( !(USISR & _BV(USIOIF)) );      // wait for reception complete
    output_low(SPI_OUT_PORT, SPI_OUT_OK);
    channel[0] = USIDR;  // TODO: proper channel selection
}


int main (void) {
    uint8_t i = 0;
    channel[0] = 255; // TODO: proper init

    wdt_disable();

    // debug leds
    set_output(DDRB, DDB4);
    set_output(DDRD, DDD5);
    output_high(PORTB, PB4);
    output_high(PORTD, PD5);

    // output channels
    set_output(DDRD, DDD3);  // TODO: pretty defines
    /* set_output(DDRD, DDD4); */
    /* set_output(DDRD, DDD5); */
    /* set_output(DDRD, DDD6); */

    // blink: devboard ok
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_low(PORTB, PB4); */
    /* delay_ms(200); */
    /* output_high(PORTB, PB4); */
    /* delay_ms(200); */


    spi_slave_init();
    counter0_init();
    counter1_init();
    zc_init();
    sei();

    while (1) {
	// wank
	/* output_low(PORTD, PD5); */
	i++;
	/* output_high(PORTD, PD5); */
	/* delay_ms(500); */
    }

    return 1;
}
