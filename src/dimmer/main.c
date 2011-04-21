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
    uint16_t delay_count = 1000;  // cpu cycles in one ms?
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


#define SPI_DDR      DDRB
#define SPI_SS_DDR   DDB2  // ~ slave select (atm unused)
#define SPI_MOSI_DDR DDB3  // 
#define SPI_MISO_DDR DDB4  // 
#define SPI_SCK_DDR  DDB5  // clock

#define SPI_PORT     PORTB
#define SPI_SS       PB2
#define SPI_MOSI     PB3
#define SPI_MISO     PB4
#define SPI_SCK      PB5

#define SPI_SLAVES_DDR     DDRD  // ~ slave select (real)
#define SPI_OUT_ENABLE_DDR DDD0
#define SPI_OUT_STORE_DDR  DDD1  // serial to parallel
#define SPI_CFG_MODE_DDR   DDD4
#define SPI_CFG_RESET_DDR  DDD5
#define SPI_CFG_SS_DDR     DDD6  // parallel to serial

#define SPI_SLAVES_PORT PORTD
#define SPI_OUT_ENABLE  PD0   // 0: output enable; 1: z-state
#define SPI_OUT_STORE   PD1
#define SPI_CFG_MODE    PD4   // 0: parallel in; 1: serial out
#define SPI_CFG_RESET   PD5
#define SPI_CFG_SS      PD6


void spi_master_init (void) {
    // MOSI, SCK, ~SS are outputs, MISO is left input
    SPI_DDR |= (1<<SPI_MOSI_DDR) | (1<<SPI_SCK_DDR) | (1<<SPI_SS_DDR);
    output_high(SPI_PORT, SPI_SS);  // ~SS inactive

    // Enable SPI, Master, SCK=fosc/128 
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | (1<<SPR1);
}

// data to transmit if you only really want to read data
#define SPI_TRANSMIT_DUMMY 0b01010101

// send and/or receive data
char spi_master_transmit (char data) {
    SPDR = data;                  // start transmission
    while( !(SPSR & (1<<SPIF)) ); // wait for transmission complete 
    return SPDR;
}

// zero crossing
#include <avr/interrupt.h>

#define ZC_DDR  DDRD
#define ZC_PORT PORTD
#define ZC      PD3

inline void zc_init (void) {
    set_input(ZC_DDR, ZC);
    output_high(ZC_PORT, ZC);  // pullup
    EIMSK |= _BV(INT1);
    //EICRA |= _BV(ISC10);       // int on any change
    EICRA |= _BV(ISC11);       // int on falling change
}

/* uint8_t zc_count = 0; */
/* uint8_t outcount = 0; */

// interrupt service routine: action to take on zero crossing
// FIXME: non-blocking interrupt resets everything
ISR (INT1_vect, ISR_NOBLOCK) {
    /* zc_count++; */
    /* if (zc_count >= 100) { */
    /* 	//outcount++; */
    /* 	zc_count = 0; */
    /* } */

    TCNT1 = 0;
    TCNT1 = 0;
}

// timer with prescaling F_CPU/256 gives 62500 ticks per second, 625
// ticks between two zero crossings
// ideally, 256 ticks (between two zc) are needed, but this is impossible
// with internal prescaler
inline void timer_init (void) {
    // clear int flags
    TIFR1  = (1<<OCF1B) | (1<<OCF1A) | (1<<TOV1);
    // between two zc, count up to 625
    OCR1BH = 0;
    OCR1BL = 625;
    // enable comparator interrupts
    TIMSK1 = (1<<OCIE1B) | (1<<OCIE1A);
    // start ticking (clock = F_CPU/256)
    TCCR1B = (1<<CS12);
}

// set next firing angle
inline void timer_cmp_set (uint8_t angle8) {
    // degree = F_CPU / 100 / 256 (instructions between firing angles)
    uint16_t angle = angle8 * (F_CPU / 100 / 256);
    OCR1AH = angle /256;
    OCR1AL = (angle>>8);
}

// interrupt: 625 ticks passed, zero crossing now, reset counter
ISR (TIMER1_COMPB_vect, ISR_NOBLOCK) {
    TCNT1L = 0;
}

// interrupt: desired firing angle reached
ISR (TIMER1_COMPA_vect, ISR_NOBLOCK) {
    spi_master_transmit(0b11111111);
    out_store();
    out_enable();
    //delay_degrees(1);
    delay_ns(10);
    out_disable();
}

// interrupt: counter overflow
// this should happen at zero crossing or equidistantly between two zero
// crossings
ISR (TIMER1_OVF_vect, ISR_NOBLOCK) {
    timer_cmp_set(128);
}


int main (void) {
    while (1) {
	wdt_reset();
    }

    return 1;
}
