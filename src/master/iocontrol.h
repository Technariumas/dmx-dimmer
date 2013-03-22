// iocontrol.h
// defines for I/O control readability

#ifndef _IOCONTROL_H_
#define _IOCONTROL_H_

// portdir: DDRx, pindir: DDxn
#define set_input(portdir,pindir)  portdir &= ~(1<<pindir)
#define set_output(portdir,pindir) portdir |=  (1<<pindir)

// port: PORTx, pin: Pxn
#define input_hiz(port,pin)    port &= ~(1<<pin)
#define input_pullup(port,pin) port |= (1<<pin)

// port: PORTx, pin: Pxn
#define output_low(port,pin)    port &= ~(1<<pin)
#define output_high(port,pin)   port |= (1<<pin)
#define output_toggle(port,pin) port ^= (1<<pin)

#endif /* _IOCONTROL_H_ */
