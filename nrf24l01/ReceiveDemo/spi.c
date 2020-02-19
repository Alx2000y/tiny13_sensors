#include <avr/io.h>
#include "spi.h"
#define nop asm volatile ("nop\n\t")
void spi_init (void) {
  deselect_chip ();
#ifdef USE_SPI_CS
  SPI_DDR |= (MOSI_PIN) | (SCK_PIN) | (CS_PIN);
#else
  SPI_DDR |= (MOSI_PIN) | (SCK_PIN);
#endif
  SPI_SCK_SHIFT ();
}

uint8_t spi_transmit_receive (uint8_t c) {
  uint8_t i;
  for (i = 0; i < 8; i++)
    {
      if (c & 0x80)
	{
	  SPI_MOSI_HIGH ();
	}
      else
	{
	  SPI_MOSI_LOW ();
	}
      SPI_SCK_LATCH ();
      c <<= 1;
      if (read_miso ())
	{
	  c |= 1;
	}
      SPI_SCK_SHIFT ();
    }
  return c;
}

void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
// Shift full array through target device
{
    deselect_chip ();
    select_chip ();

       uint8_t i;      
       for (i = 0; i < len; i++) {
             datain[i] = spi_transmit_receive(dataout[i]);
       }
    deselect_chip ();
}

void spi_transmit_sync (uint8_t * dataout, uint8_t len)
// Shift full array to target device without receiving any byte
{
    deselect_chip ();
    select_chip ();
       uint8_t i;      
       for (i = 0; i < len; i++) {
             spi_transmit_receive(dataout[i]);
       }
    deselect_chip ();
}
uint8_t spi_fast_shift (uint8_t c) {
  deselect_chip ();
  select_chip ();
  c=spi_transmit_receive (c);
  deselect_chip ();
  return c;
}
