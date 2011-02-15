## TODO: revise, cleanup

# Name: Makefile
# Project: xomg's DMX Dimmer
# Author: xomg (based on Christian Starkjohann's USBaspLoader)
# Creation Date: 
# Tabsize: 4
# Copyright:
# License: GNU GPL v3 (see LICENSE)
# This Revision: 

###############################################################################
# Configure the following variables according to your AVR.
# Program the device with
#     make fuse    # to set the clock generator, boot section size etc.
#     make flash   # to load the boot loader into flash

F_CPU = 16000000
DEVICE = atmega168

FUSEOPT = -U hfuse:w:0xd6:m -U lfuse:w:0xdf:m -U efuse:w:0x07:m

# You may have to change the order of these -U commands.

#---------------------------------------------------------------------
# ATMega88, ATMega168
#---------------------------------------------------------------------
# Fuse extended byte:
# 0x07 = 0 0 0 0   0 1 1 1 <-- BOOTRST (1: boot reset vector at 0x0000)
#                    \+/
#                     +------- BOOTSZ (11 = 256 bytes)
# Fuse high byte:
# 0xd6 = 1 1 0 1   0 1 1 0
#        ^ ^ ^ ^   ^ \-+-/
#        | | | |   |   +------ BODLEVEL 0..2 (110 = 1.8 V)
#        | | | |   + --------- EESAVE (preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (0: watchdog always on)
#        | | +---------------- SPIEN (0: allow serial programming)
#        | +------------------ DWEN (1: debug wire disable)
#        +-------------------- RSTDISBL (1: reset pin enabled)
# Fuse low byte:
# 0xdf = 1 1 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ CKOUT (if 0: Clock output enabled)
#        +-------------------- CKDIV8 (if 0: divide by 8)
#---------------------------------------------------------------------

##########################################################################

# Tools:
PROGRAMMER = -c usbasp #-P avrdoper
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
CC = avr-gcc

# Options:
DEFINES = #-DDEBUG_LEVEL=2
# Remove the -fno-* options when you use gcc 3, it does not understand them
CFLAGS = -Wall -Os -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions -I. -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) $(DEFINES)
LDFLAGS = -Wl,--relax,--gc-sections -Wl #,--section-start=.text=$(BOOTLOADER_ADDRESS)

OBJECTS =  main.o

# symbolic targets:
all: main.hex

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.S.o:
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(CC) $(CFLAGS) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

readflash:
	$(AVRDUDE) -U flash:r:read.hex:i

fuse:
	$(AVRDUDE) $(FUSEOPT)

# doesn't work with avrdude
#read_fuses:
#	$(UISP) --rd_fuses

clean:
	rm -f main.hex main.bin *.o main.s

# file targets:
main.bin:	$(OBJECTS)
	$(CC) $(CFLAGS) -o main.bin $(OBJECTS) $(LDFLAGS)

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex
	avr-size main.hex

disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(CC) $(CFLAGS) -E main.c
