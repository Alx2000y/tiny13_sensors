# makefile, written by guido socher
#
# Please select according to the type of board you are using:
#MCU=atmega88
#DUDECPUTYPE=m88
#MCU=attiny13
#DUDECPUTYPE=m168
MCU=attiny13
DUDECPUTYPE=attiny13
#MCU=atmega644
#DUDECPUTYPE=m644
#
# === Edit this and enter the correct device/com-port:
# linux (plug in the avrusb500 and type dmesg to see which device it is):
#LOADCMD=avrdude -P /dev/ttyUSB0
LOADCMD=avrdude
LOADARG=-p $(DUDECPUTYPE) -c usbasp -b 19200 -U flash:w:

# mac (plug in the programer and use ls /dev/tty.usbserial* to get the name):
#LOADCMD=avrdude -P /dev/tty.usbserial-A9006MOb

# windows (check which com-port you get when you plugin the avrusb500):
#LOADCMD=avrdude -P COM4

# All operating systems: if you have set the default_serial paramter 
# in your avrdude.conf file correctly then you can just use this
# and you don't need the above -P option:
#LOADCMD=avrdude
# === end edit this
#
#LOADARG=-p $(DUDECPUTYPE) -c stk500v2 -e -U flash:w:

# If you have a tuxgraphics microcontroller mail account 
# then you can uncommet the following line to compile the
# email client. You will also need to edit test_emailnotify.c
# and add your account ID there.
#EMAIL=haveaccount
# 
#
CC=avr-gcc
OBJCOPY=avr-objcopy
# optimize for size:
CFLAGS=-g -mmcu=$(MCU) -Wall -W -Os -mcall-prologues -DF_CPU=1200000L
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -Wno-deprecated-declarations -D__PROG_TYPES_COMPAT__

#-------------------
.PHONY: all main
#
remake: clean tiny13.hex

all: tiny13.hex
#
tiny13: tiny13.hex
	@echo "done"
#
main: tiny13.hex
	@echo "done"
#
#-------------------
tiny13.hex: tiny13.elf 
	$(OBJCOPY) -R .eeprom -O ihex tiny13.elf tiny13.hex 
	avr-objdump -h -S tiny13.elf >tiny13.lss 
	avr-size --mcu=$(MCU) tiny13.elf
	@echo " "
	@echo "Expl.: data=initialized data, bss=uninitialized data, text=code"
	@echo " "

tiny13.elf: main.o mirf_tx.o spi.o
	$(CC) $(CFLAGS) -o tiny13.elf -Wl,-Map,tiny13.map main.o mirf_tx.o spi.o
spi.o: spi.c
	$(CC) $(CFLAGS) -Os -c spi.c
mirf_tx.o: mirf_tx.c mirf_tx.h nRF24L01.h
	$(CC) $(CFLAGS) -Os -c mirf_tx.c
main.o: main.c
	$(CC) $(CFLAGS) -Os -c main.c

load: tiny13.hex
	$(LOADCMD) $(LOADARG)tiny13.hex
verify: tiny13.hex
	$(LOADCMD) -p $(DUDECPUTYPE) -c avrisp -P COM8 -b 19200 -U flash:v:tiny13.hex
#
#-------------------
# Check this with make rdfuses
rdfuses:
	avrdude -p $(DUDECPUTYPE) -c avrisp -P COM8 -b 19200 -v
#
fuse:
	@echo "warning: run this command only if you have an external clock on xtal1"
	@echo "The is the last chance to stop. Press crtl-C to abort"
	@sleep 2
	avrdude -p  $(DUDECPUTYPE)  -c avrisp -P COM8 -b 19200 -u -v -U lfuse:w:0x6A:m

#-------------------
clean:
	rm -f *.o *.map *.elf test*.hex tiny13.hex *.lss
#-------------------