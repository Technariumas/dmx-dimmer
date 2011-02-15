// xomg's DMX dimmer
// main.c: 

#include "xomg_dmx_dimmer.h"  // board-specific
#include "usart.h"            // dmx

#define DMX_START_CODE 0      // aka DMX_ADDRESS: first slot's data

struct dmx_t {
    bool error = false;       // last frame had error
    unsigned int channels = 8;
    unsigned int slot = 0;    // slot counter
    unsigned char status;
    unsigned char data;
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
    dmx.status = UCSR0A;
    dmx.data = UDR0;

    // check for USART errors
    if (status & (1<<DOR0)|(1<<FE0)) {
	dmx.error = true;
	//dmx.slot = 0;
    }
    else {  // no USART errors
	if (dmx.error == false) {
	    if (dmx.slot == 0) {
		if (dmx.data == DMX_START_CODE) {
		    // flash happily
		}
		else dmx.error = true;
	    }
	    else {
		// write data to buffer[dmx.slot-1]
	    }
	}
	else {  // dmx.error == true
	    if ((dmx.slot == 0) & (dmx.data == DMX_START_CODE)) {
		// new dmx frame
		dmx.error = false;
	    }
	}
    }

    dmx.slot++;
    if (dmx.slot > dmx.channels) dmx.slot = 0;
}

// interrupt: zc
// non-blocking (otherwise will miss usart frame)
// send data out
// prepare new data immediately?
