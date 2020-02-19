#include <avr/wdt.h>
#include "mirf.h"
#include "mirf.c"
#include "spi.h"
#include "spi.c"
#include "nRF24L01.h"


uint8_t DSdata[16];
void reset() {
    wdt_disable();
    wdt_enable(WDTO_15MS);
    while (1) {}
}
void setup() {
    Serial.begin(57600);
    delay(2);
    Serial.println("Mirf Init");
    mirf_init();
    mirf_config();
    //mirf_config_register(SETUP_RETR, 0 ); // no retransmit 
    mirf_config_register(EN_AA, 0 ); // no auto-ack 
    //mirf_config_register(RF_SETUP, (1<<RF_DR_LOW) ); // low spd & power 
    mirf_set_TADDR((uint8_t *)"1Serv");
    mirf_set_RADDR((uint8_t *)"1Sens");
    mirf_config_register(RX_ADDR_P2,0x32);
    mirf_config_register(RX_PW_P2, mirf_PAYLOAD);
    mirf_config_register(RX_ADDR_P3,0x33);
    mirf_config_register(RX_PW_P3, mirf_PAYLOAD);
    mirf_config_register(RX_ADDR_P4,0x34);
    mirf_config_register(RX_PW_P4, mirf_PAYLOAD);
    mirf_config_register(RX_ADDR_P5,0x35);
    mirf_config_register(RX_PW_P5, mirf_PAYLOAD);
    mirf_config_register(EN_RXADDR, 0x7F);

    Serial.println("Setup OK");
    LastTime = millis();
    wdt_enable(WDTO_4S);

}
void loop() {
    wdt_reset();

    if(mirf_data_ready())  {
        mirf_read_register( STATUS, &rf_setup, sizeof(rf_setup));
        mirf_get_data(DSdata);

        Serial.print("#NRF@");
        Serial.print(( rf_setup >> 1 ) & 0x07, DEC);
        Serial.print("Raw@");
        Serial.print(DSdata[0], HEX);
        Serial.print(" ");
        Serial.print(DSdata[1], HEX);
        Serial.print(" ");
        Serial.print(DSdata[2], HEX);
        Serial.print(" ");
        Serial.print(DSdata[3], HEX);
        Serial.print(" ");
        Serial.print(DSdata[4], HEX);
        Serial.print(" ");
        Serial.print(DSdata[5], HEX);
        Serial.print(" ");
        Serial.print(DSdata[6], HEX);
        Serial.print(" ");
        Serial.print(DSdata[7], HEX);
        Serial.print(" ");
        Serial.print(DSdata[8], HEX);
        Serial.print(" ");
        Serial.print(DSdata[9], HEX);
        Serial.print(" ");
        Serial.print(DSdata[10], HEX);
        Serial.print(" ");
        Serial.print(DSdata[11], HEX);
        Serial.print(" ");
        Serial.print(DSdata[12], HEX);
        Serial.print(" ");
        Serial.print(DSdata[13], HEX);
        Serial.print(" ");
        Serial.print(DSdata[14], HEX);
        Serial.print(" ");
        Serial.print(DSdata[15], HEX);
        Serial.print("#");
        Serial.println();
    }
}
