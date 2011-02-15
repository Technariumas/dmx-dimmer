// xomg's DMX dimmer
// main.c: 

#include "xomg_dmx_dimmer.h"  // board-specific
#include "usart.h"            // dmx

#define DMX_START_CODE 0      // aka DMX_ADDRESS: first slot's data

struct dmx_t {
    bool error = false;
    unsigned int slot = 0;    // slot counter
} dmx;


void main (void)
{
    board_init();  // internals and peripherals

    while (1) {
	// ...
    }
}

// interrupt service routine: usart receive complete
ISR (USART_RX_vect) {
    // reading data clears errors from status, so read status first
    unsigned char status = UCSR0A;
    unsigned char data = UDR0;

    // data overrun: too much data and new coming in, dmx.slot lost; or
    // frame error: bad data
    if ( status & (1<<DOR0)|(1<<FE0) ) {
	dmx.error = true;
	dmx.slot = 0;
    }
    else {
	dmx.error = false;

	if (dmx.slot == 0) {
	    // flash led: new dmx frame
	}
	else {
	    // write data to buffer
	}
	
	dmx.slot++;
    }
}

// interrupt: zc
// send data out
// prepare new data immediately?
