// usart.c

#include "usart.h"

#include <avr/io.h>

#include "transceiver.h"
#include "iocontrol.h"

void usart_init (void) {
    // init databuf (TODO: in function?)
    /* int i; */
    /* for (i = 0; i < sizeof(databuf); i++) databuf[i] = 0; */

    // ~RE is on by default
    set_output(USART_DDR, USART_NRE);
    output_low(USART_PORT, USART_NRE);

    // set baud rate (250 kHz)
    UBRR0H = 0;
    UBRR0L = 3;

    // set frame format: asynchronous, 8 data bits, 2 stop bits, no parity
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01) | _BV(USBS0);

    // enable receiver and interrupt
    UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
}
