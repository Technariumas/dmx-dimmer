// dmx dimmer, transceiver module
// TODO: file header

#define F_CPU 16000000  // 16 MHz

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>  // TODO: use or remove
#include <avr/interrupt.h>

#define set_input(portdir,pin)  portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |=  (1<<pin)
#define output_low(port,pin)  port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define output_toggle(port,pin) port ^= (1<<pin)

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

// indicate 8 bits using 3 leds
void blink8 (uint8_t data) {
    uint8_t i;

    for (i = 0; i < 8; i++) {
	//indicate bit
	if (data & 0b00000001) output_high(PORTC, PC4);
	else output_high(PORTC, PC5);
	delay_ms(500);

	// indication complete
	output_low(PORTC, PC4);
	output_low(PORTC, PC5);
	delay_ms(100);

	data = data >> 1;
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

#define SPI_SLAVES_DDR    DDRD  // ~ slave select (real)
#define SPI_OUT_CHAN0_DDR DDD0
#define SPI_OUT_CHAN1_DDR DDD1
#define SPI_OUT_SS1_DDR   DDD2
/* #define SPI_OUT_SS2_DDR   DDD3 */
/* #define SPI_OUT_SS3_DDR   DDD4 */
#define SPI_OUT_OK_DDR    DDD3
#define SPI_CFG_MODE_DDR  DDD4
#define SPI_CFG_RESET_DDR DDD5
#define SPI_CFG_SS_DDR    DDD6  // parallel to serial

#define SPI_SLAVES_PORT PORTD
#define SPI_OUT_CHAN0   PD0
#define SPI_OUT_CHAN1   PD1
#define SPI_OUT_SS1     PD2
/* #define SPI_OUT_SS2     PD3 */
/* #define SPI_OUT_SS3     PD4 */
#define SPI_OUT_OK      PD3
#define SPI_CFG_MODE    PD4   // 0: parallel in; 1: serial out
#define SPI_CFG_RESET   PD5
#define SPI_CFG_SS      PD6

#define SPI_SLAVES_PIN PIND
#define SPI_OUT_OK_PIN PIND3

#define cfg_reset_enable()  output_low(SPI_SLAVES_PORT, SPI_CFG_RESET)
#define cfg_reset_disable() output_high(SPI_SLAVES_PORT, SPI_CFG_RESET)
#define cfg_select()        output_low(SPI_SLAVES_PORT, SPI_CFG_SS)
#define cfg_deselect()      output_high(SPI_SLAVES_PORT, SPI_CFG_SS)
#define cfg_mode_parallel() output_low(SPI_SLAVES_PORT, SPI_CFG_MODE)
#define cfg_mode_serial()   output_high(SPI_SLAVES_PORT, SPI_CFG_MODE)


inline void spi_master_init (void) {
    // 
    set_output(SPI_SLAVES_DDR, SPI_OUT_CHAN0_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_CHAN1_DDR);
    set_output(SPI_SLAVES_DDR, SPI_OUT_SS1_DDR);
    /* set_output(SPI_SLAVES_DDR, SPI_OUT_SS2_DDR); */
    /* set_output(SPI_SLAVES_DDR, SPI_OUT_SS3_DDR); */
    set_input(SPI_SLAVES_DDR, SPI_OUT_OK_DDR);

    // MOSI, SCK, ~SS are outputs, MISO is left input
    SPI_DDR |= _BV(SPI_MOSI_DDR) | _BV(SPI_SCK_DDR) | _BV(SPI_SS_DDR);
    output_high(SPI_PORT, SPI_SS);  // ~SS inactive

    // Enable SPI, Master, SCK=fosc/128
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR1) | _BV(SPR0);
}

// data to transmit if you only really want to read data
#define SPI_TRANSMIT_DUMMY 0b01010101

// send and/or receive data
char spi_master_transmit (char data) {
    SPDR = data;                  // start transmission
    while( !(SPSR & _BV(SPIF)) ); // wait for transmission complete 
    return SPDR;
}

// RS485 chip SN75176B got a Read Enable and Transmit Enable pins
#define USART_PORT PORTD  // RS485 PORT for Read Enable (Receiving)
#define USART_DDR  DDRD   // RS485 DDR  for Read Enable (Receiving)
#define USART_NRE  PD2    // Active Low Read Enable

typedef struct {
    uint8_t  error;           // last frame had an error
    uint16_t startaddr;
    uint16_t slot;            // slot counter
    uint8_t  status;
    uint8_t  data;
} dmx_t;

dmx_t dmx = {0, 100, 0, 0, 0};
#define DMX_CHANNELS 12
uint8_t databuf[DMX_CHANNELS];

// 
void usart_init (void) {
    // init databuf (TODO: in function?)
    int i;
    for (i = 0; i < sizeof(databuf); i++) databuf[i] = 0;

    // ~RE is on by default
    set_output(USART_DDR, USART_NRE);
    output_low(USART_PORT, USART_NRE);

    // set baud rate (250 kHz)
    UBRR0H = 0;
    UBRR0L = 3;

    // set frame format: asynchronous, 1-8-2, no parity
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01) | _BV(USBS0);

    // enable receiver and interrupt
    UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
}

// interrupt: usart receive complete
ISR (USART_RX_vect, ISR_NOBLOCK) {
    // reading data clears errors from status, so read status first
    dmx.status = UCSR0A;
    dmx.data = UDR0;

    // data overrun or frame error (break condition?)
    if ( dmx.status & (_BV(DOR0)|_BV(FE0)) ) {
	//dmx.error = 1;
	dmx.slot = 0;
    }
    else {
	if ((dmx.slot > dmx.startaddr) &&
	    (dmx.slot <= dmx.startaddr + DMX_CHANNELS))
	    databuf[dmx.slot-1] = dmx.data;
	dmx.slot++;
    }

    output_toggle(PORTC, PC3);  // debug
}


int main (void) {
    /* uint8_t confl = 0; */
    /* uint8_t confh = 0; */
    uint8_t dmx_channel = 0;  // TODO: not needed, fill array from dmx
    uint8_t dmx_value;        // TODO: get array from dmx
    uint8_t tmp;
    uint8_t inc = 1;  // debug - 0: down, 1: up

    wdt_disable();

    // leds are outputs
    set_output(DDRC, DDC3);
    set_output(DDRC, DDC4);
    set_output(DDRC, DDC5);

    // led blink: devboard ok
    output_high(PORTC, PC3);
    output_high(PORTC, PC4);
    output_high(PORTC, PC5);
    delay_ms(500);
    output_low(PORTC, PC3);
    output_low(PORTC, PC4);
    output_low(PORTC, PC5);

    // borrow some spi pins to read in cfg
    /* set_output(SPI_SLAVES_DDR, SPI_CFG_RESET_DDR); */
    /* set_output(SPI_DDR, SPI_SCK_DDR); */
    /* set_output(SPI_SLAVES_DDR, SPI_CFG_SS_DDR); */
    /* set_output(SPI_SLAVES_DDR, SPI_CFG_MODE_DDR); */
    /* cfg_reset_disable(); */

    /* cfg_select();  // redundant, already low */
    /* delay_ms(1);   // maybe redundant, should be one set-up time */

    /* cfg_mode_parallel(); */
    /* delay_ms(1); */

    /* // pulse clock to read in bits (not needed with 74165) */
    /* output_high(SPI_PORT, SPI_SCK); */

    /* cfg_mode_serial(); */
    /* delay_ms(1); */

    spi_master_init();
    //SPCR |= (1<<CPOL);  // otherwise can't read first bit

    /* // read in first octet */
    /* output_high(PORTC, PC3); */
    /* confl = spi_master_transmit(SPI_TRANSMIT_DUMMY); */
    /* output_low(PORTC, PC3); */

    /* // read in second octet */
    /* output_high(PORTC, PC3); */
    /* confh = spi_master_transmit(SPI_TRANSMIT_DUMMY); */
    /* output_low(PORTC, PC3); */

    /* output_high(SPI_PORT, SPI_SCK); */
    /* cfg_deselect(); */
    /* cfg_reset_enable(); */

    usart_init();

    sei();

    dmx_value = 0;  // debug
    while (1) {
	wdt_reset();

	// TODO: proper dmx_channel set
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_low(SPI_SLAVES_PORT, SPI_OUT_CHAN1);
	output_toggle(SPI_SLAVES_PORT, SPI_OUT_SS1);  // interrupt
	while ( !(SPI_SLAVES_PIN & _BV(SPI_OUT_OK_PIN)) );  // wait
	tmp = spi_master_transmit(databuf[0]);
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN0);
	output_high(SPI_SLAVES_PORT, SPI_OUT_CHAN1);

	// led: transmitted
	output_high(PORTC, PC5);
	delay_ms(10);
	output_low(PORTC, PC5);	

	// debug
	if (tmp != 0b01010101) {
	    output_high(PORTC, PC4);
	    delay_ms(990);
	    output_low(PORTC, PC4);
	}
	else delay_ms(40);

	/* if (dmx_value == 0) inc = 1; */
	/* if (dmx_value == 255) inc = 0; */
	/* if (inc == 1) dmx_value++; */
	/* if (inc == 0) dmx_value--; */
    }

    return 1;
}
