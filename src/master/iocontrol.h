// iocontrol.h
// defines for I/O control readability

#ifndef _IOCONTROL_H_
#define _IOCONTROL_H_

// portdir: DDRx, pin: DDxn
#define set_input(portdir,pin)  portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |=  (1<<pin)

// port: PORTx, pin: Pxn
#define input_nopullup(port,pin) port &= ~(1<<pin)
#define input_pullup(port,pin)   port |= (1<<pin)

// port: PINx, pin: PINxn
#define output_low(port,pin)    port &= ~(1<<pin)
#define output_high(port,pin)   port |= (1<<pin)
#define output_toggle(port,pin) port ^= (1<<pin)

#endif /* _IOCONTROL_H_ */
