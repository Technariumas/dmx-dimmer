// usart.h
// receive dmx data

#ifndef _USART_H_
#define _USART_H_

#include <inttypes.h>

enum dmx_state_t {IDLE, BREAK, SKIP, DATA};

typedef struct {
    enum dmx_state_t state;    // state machine
    uint16_t         address;  // dmx address (of first channel)
    uint16_t         slot;     // slot number counter
    uint8_t          status;   // usart status byte
    uint8_t          data;     // usart data byte
} dmx_t;

// 
void usart_init (void);

#endif /* _USART_H_ */
