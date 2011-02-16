// xomg's DMX dimmer
// main.c: 

#include <stdbool.h>
#include "xomg_dmx_dimmer.h"  // board-specific
#include "usart.h"            // dmx

#define DMX_START_CODE 0      // aka DMX_ADDRESS: first slot's data

typedef struct {
    bool     error;           // last frame had an error
    uint16_t channels;
    uint16_t slot;            // slot counter
    uint8_t  status;
    uint8_t  data;
} dmx_t;
dmx_t dmx = {false, 8, 0, 0, 0};

int main (void)
{
    board_init();  // internals and peripherals
    dmx.channels = 8 + 8 * (PINC & 0b00111111);

    while (1) {
	// ...
    }

    return 0;
}

// interrupt service routine: usart receive complete
ISR (USART_RX_vect, ISR_NOBLOCK) {
    // reading data clears errors from status, so read status first
    dmx.status = UCSR0A;
    dmx.data = UDR0;

    // check for USART errors
    if ( dmx.status & ((1<<DOR0)|(1<<FE0)) ) {
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
ISR (INT1_vect, ISR_NOBLOCK) {
    
}
