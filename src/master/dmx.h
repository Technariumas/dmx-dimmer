// dmx.h
// DMX-related, independent of USART or SPI

#ifndef _DMX_H_
#define _DMX_H_

#include <inttypes.h>

#define DMX_CHANNELS 1

// DMX state machine possible states
enum dmx_state_t {IDLE, BREAK, SKIP, DATA};

// information about DMX data (but not the data itself)
typedef struct {
    enum dmx_state_t state;      // state machine
    uint16_t         address;    // dmx address (of first channel)
    uint16_t         slot;       // slot number counter
    uint16_t         dataisnew;  // flag new data
    uint8_t          status;     // usart status byte
    uint8_t          data;       // usart data byte
    uint8_t          preheat;
    uint8_t          maxval;
    uint8_t          chanval[DMX_CHANNELS];
} dmx_t;

#endif /* _DMX_H_ */
