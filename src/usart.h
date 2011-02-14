// TODO: check if this works with UBRRH instead of UBRR0H, etc.

/*
 * usart.h
 *
 *  Created on: 27-jan-2009
 *      Author: Jan
 */

#ifndef USART_H_
#define USART_H_


#include <avr/io.h>  // --mmcu= compiler switch, say, atmega168


// TODO: function autodoc
/* @param (unsigned int) (ubr) usart baud rate */
void usart_init (unsigned int ubr)
{
    /* Set baud rate */
    UBRR0H = (unsigned char) (ubr>>8);
    UBRR0L = (unsigned char) ubr;


    /* Enable receiver and interrupt */
    UCSR0B = (1<<RXEN) | (1<<RXCIE);

    /* Set frame format: asynchronous, 8-2 (default) */
    //UCSR0C |= (1<<UMSEL00) | (1<<UCSZ00) | (1<<UCSZ01) | (1<<USBS0);
}

/* @param (unsigned char) (data) one byte of data to transmit*/
void usart_transmit (unsigned char data)
{
    /* Wait for empty transmit buffer */
    while ( !( UCSR0A & (1<<UDRE0)) )
	;

    /* Put data into buffer, send */
    UDR0 = data;
}

// currently only USART frames with 8 bits of data are supported
// see datasheet on how to implement 9th bit retrieval
unsigned char usart_receive (void)
{
    unsigned char status, resl;

    /* Wait for data to be received */
    while ( !(UCSR0A & (1<<RXC0)) )
	;

    /* Get status and 9th bit, then data */
    /* from buffer */
    status = UCSR0A;
    resl = UDR0;

    /* Check for Frame Error, Data OverRun, Parity Error */
    if ( status & (1<<FE0)|(1<<DOR0)|(1<<UPE0) )
	return -1;

    /* Filter the 9th bit, then return */
    return (resl);
}

#endif /* USART_H_ */
