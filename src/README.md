Two types of microcontroller are used in this schematic, both Atmel AVR
architecture.

The (more powerful) transceiver module is ATmega168. It receives DMX data
using RS485 (USART) protocol and then passes them through SPI.

The (less powerful) dimmer module uses ATtiny2313. It receives SPI data,
detects zero crossing on one of mains' three phases, and controls four
dimming channels.

ATmega168 is an SPI Master. Three ATtiny2313 (one for each mains phase)
are SPI Slaves.

Since dimmers' operation is rather time-critical, a slave selection
setup is in place. (Currently, just two unmultiplexed lines.) It's
purpose is to pick a proper phase's slave and set transmitted channel's
address in-software. An interrupt is sent from Master to Slave to
indicate that data is ready. Slave sets up its hardware and accepts data.

This way, data transmission is done using pure hardware modules on both
sides. The Slave can go on with its time-critical software business.
