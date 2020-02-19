#ifndef _SPI_H_
#define _SPI_H_


#include <avr/io.h>


#define SPI_MODE	0  // Support mode 0 and 3
//#define USE_SPI_CS	1  // AUto CS 

#define SPI_PORT	PORTD
#define SPI_DDR		DDRD
#define SPI_PIN		PIND
#ifdef USE_SPI_CS
#define CS_PIN		(1<<PD6)
#endif
#define MOSI_PIN	(1<<PD3)
#define MISO_PIN 	(1<<PD4)
#define SCK_PIN 	(1<<PD5)

#define read_miso()		(SPI_PIN & (MISO_PIN))
#ifdef USE_SPI_CS
#define select_chip()	(SPI_PORT &= ~(CS_PIN))
#define deselect_chip()	(SPI_PORT |= (CS_PIN))
#else
#define select_chip()
#define deselect_chip()
#endif

#if SPI_MODE == 0
#define SPI_SCK_SHIFT()	(SPI_PORT &= ~SCK_PIN)
#define SPI_SCK_LATCH()	(SPI_PORT |= SCK_PIN)
#else
#define SPI_SCK_SHIFT()	(SPI_PORT |= SCK_PIN)
#define SPI_SCK_LATCH()	(SPI_PORT &= ~SCK_PIN)
#endif

#define SPI_MOSI_HIGH()	(SPI_PORT |= MOSI_PIN)
#define SPI_MOSI_LOW()	(SPI_PORT &= ~MOSI_PIN)

extern uint8_t spi_transmit_receive (uint8_t c);
extern void spi_init();
extern void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
extern void spi_transmit_sync (uint8_t * dataout, uint8_t len);
extern uint8_t spi_fast_shift (uint8_t data);

#endif /* _SPI_H_ */
