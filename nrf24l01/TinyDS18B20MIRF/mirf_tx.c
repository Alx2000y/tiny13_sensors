#include "mirf_tx.h"
#include "nRF24L01.h"
#include "spi.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define TX_POWERUP mirf_config_register(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) )

void mirf_init() 
{
    DDRB |= (1<<CSN);
    mirf_CSN_hi;
    spi_init();
}

void mirf_config() 
{
    mirf_config_register(RF_CH,mirf_CH);
    mirf_config_register(RX_PW_P0, mirf_PAYLOAD);
}

void mirf_set_TADDR(uint8_t * adr)
{
    mirf_write_register(TX_ADDR, adr,5);
}

void mirf_config_register(uint8_t reg, uint8_t value)
{
    mirf_CSN_lo;
    spi_fast_shift(W_REGISTER | (REGISTER_MASK & reg));
    spi_fast_shift(value);
    mirf_CSN_hi;
}

void mirf_read_register(uint8_t reg, uint8_t * value, uint8_t len)
{
    mirf_CSN_lo;
    spi_fast_shift(R_REGISTER | (REGISTER_MASK & reg));
    spi_transfer_sync(value,value,len);
    mirf_CSN_hi;
}

void mirf_write_register(uint8_t reg, uint8_t * value, uint8_t len) 
{
    mirf_CSN_lo;
    spi_fast_shift(W_REGISTER | (REGISTER_MASK & reg));
    spi_transmit_sync(value,len);
    mirf_CSN_hi;
}


void mirf_send(uint8_t * value, uint8_t len) 
{
    TX_POWERUP;                     // Power up
    mirf_CSN_lo;                    // Pull down chip select
    spi_fast_shift( FLUSH_TX );     // Write cmd to flush tx fifo
    mirf_CSN_hi;                    // Pull up chip select
    mirf_CSN_lo;                    // Pull down chip select
    spi_fast_shift( W_TX_PAYLOAD ); // Write cmd to write payload
    spi_transmit_sync(value,len);   // Write payload
    mirf_CSN_hi;                    // Pull up chip select
}
