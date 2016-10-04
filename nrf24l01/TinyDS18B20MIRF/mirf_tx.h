#ifndef _MIRF_TX_H_
#define _MIRF_TX_H_

#include <avr/io.h>

// Mirf settings
#define mirf_CH         13
#define mirf_PAYLOAD    16
#define mirf_CONFIG     ( (1<<MASK_RX_DR) | (1<<EN_CRC) | (0<<CRCO) )

#define CSN PB4

#define mirf_CSN_hi     PORTB |=  (1<<CSN);
#define mirf_CSN_lo     PORTB &= ~(1<<CSN);

extern void mirf_init();
extern void mirf_config();
extern void mirf_set_TADDR(uint8_t * adr);
extern void mirf_config_register(uint8_t reg, uint8_t value);
extern void mirf_read_register(uint8_t reg, uint8_t * value, uint8_t len);
extern void mirf_write_register(uint8_t reg, uint8_t * value, uint8_t len);
extern void mirf_send(uint8_t * value, uint8_t len);

#endif /* _MIRF_TX_H_ */
