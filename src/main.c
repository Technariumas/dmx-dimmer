// xomg's DMX dimmer
// main.c: 

#include <stdbool.h>
#include "xomg_dmx_dimmer.h"  // board-specific
#include "usart.h"            // dmx

#define DMX_START_CODE 0      // first slot's data

dmx_t dmx = {false, 0, 0, 0, 0};
uint8_t databuf[DMX_CHANNELS];  // TODO: init to 0s
uint8_t zc_count = 0;


int main (void)
{
    init_board();  // internals and peripherals
    for (int i = 0; i < sizeof(databuf); i++) databuf[i] = 0;

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
		// careful with the oboes!
		databuf[dmx.slot-1] = dmx.data;
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
    // send current data to dimmers
    output_high(STP_NOUTEN_DDR, STP_NOUTEN);
    output_high(STP_CLK_DDR, STP_CLK);
    output_low(STP_CLK_DDR, STP_CLK);
    output_low(STP_NOUTEN_DDR, STP_NOUTEN);

    // fill with new data
    for (int i = 0; i < dmx.channels; i++) {
	if (databuf[i] >= zc_count) {
	    output_high(STP_DATA_DDR, STP_DATA);
	}
	else {
	    output_low(STP_DATA_DDR, STP_DATA);
	}
	output_high(STP_CLK_DDR, STP_CLK);
	output_low(STP_CLK_DDR, STP_CLK);
    }

    zc_count++;  // ok to overflow 255->0
}
