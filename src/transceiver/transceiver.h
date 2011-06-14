// dmx dimmer, transceiver module

#ifndef _TRANSCEIVER_H_
#define _TRANSCEIVER_H_

#include <avr/io.h>

// ===== CONFIG =====

#define F_CPU 16000000  // 16 MHz

// ===== PINOUT =====

#define SPI_DDR      DDRB
#define SPI_SS_DDR   DDB2  // ~ slave select (atm unused)
#define SPI_MOSI_DDR DDB3  // 
#define SPI_MISO_DDR DDB4  // 
#define SPI_SCK_DDR  DDB5  // clock

#define SPI_PORT     PORTB
#define SPI_SS       PB2
#define SPI_MOSI     PB3
#define SPI_MISO     PB4
#define SPI_SCK      PB5

#define SPI_SLAVES_DDR    DDRD  // ~ slave select (real)
#define SPI_OUT_CHAN0_DDR DDD3
#define SPI_OUT_CHAN1_DDR DDD4
#define SPI_OUT_SS1_DDR   DDD5
/* #define SPI_OUT_SS2_DDR   DDD3 */
/* #define SPI_OUT_SS3_DDR   DDD4 */
#define SPI_OUT_OK_DDR    DDD6
/* #define SPI_CFG_MODE_DDR  DDD4 */
/* #define SPI_CFG_RESET_DDR DDD5 */
/* #define SPI_CFG_SS_DDR    DDD6  // parallel to serial */

#define SPI_SLAVES_PORT PORTD
#define SPI_OUT_CHAN0   PD3
#define SPI_OUT_CHAN1   PD4
#define SPI_OUT_SS1     PD5
/* #define SPI_OUT_SS2     PD3 */
/* #define SPI_OUT_SS3     PD4 */
#define SPI_OUT_OK      PD6
/* #define SPI_CFG_MODE    PD4   // 0: parallel in; 1: serial out */
/* #define SPI_CFG_RESET   PD5 */
/* #define SPI_CFG_SS      PD6 */

#define SPI_SLAVES_PIN PIND
#define SPI_OUT_OK_PIN PIND6

#define USART_DDR     DDRD   // RS485 DDR for Read Enable (Receiving)
#define USART_RXD_DDR DDD0   // receive
#define USART_TXD_DDR DDD1   // drive/transmit
#define USART_NRE_DDR DDD2   // Active Low Read Enable

#define USART_PORT PORTD
#define USART_TXD  PD1
#define USART_NRE  PD2

#define USART_PIN     PIND
#define USART_RXD_PIN PIND0

#endif /* _TRANSCEIVER_H_ */
