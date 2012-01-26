// spi.h
// serial communications (slave mode)

#ifndef _SPI_H_
#define _SPI_H_

// data to transmit when you only want to receive
#define SPI_TRANSMIT_DUMMY 0b01010101

//
inline void spi_slave_init (void);

// TODO: remove if unused
char spi_slave_receive (void);

#endif /* _SPI_H_ */
