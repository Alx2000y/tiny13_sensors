#include <avr/io.h>
#include <avr/wdt.h> // здесь организована работа с ватчдогом
#include <avr/sleep.h> // здесь описаны режимы сна
#include <avr/interrupt.h> // работа с прерываниями
#include <stdint.h>
#include <util/delay.h>     /* for _delay_us() */
#include "mirf_tx.h"
#include "nRF24L01.h"




//#define ACCESSORIESPOWERPIN 2   //pin 7
#define DS_BIT         3 // pin 2 attiny 13 pullup резистор не нужен
#define SLEEPDURATION 8//0 //sleep duration in seconds/8, shall be a factor of 8

// watchdog interrupt
ISR (WDT_vect) {
        WDTCR |= _BV(WDTIE); // разрешаем прерывания по ватчдогу. Иначе будет резет.
}

void sleepFor8Secs(int oct) {
	cli();
    MCUSR = 0;  // clear various "reset" flags
    //инициализация ватчдога
    wdt_reset(); // сбрасываем
    if(oct==1) {
        wdt_enable(WDTO_8S); // разрешаем ватчдог 8 сек
    }else{
        wdt_enable(WDTO_1S); // разрешаем ватчдог 1 сек
    }
	//WDTCR |= _BV(WDCE);
	WDTCR &= ~_BV(WDE);
    WDTCR |= _BV(WDTIE); // разрешаем прерывания по ватчдогу. Иначе будет резет.
	sei(); // разрешаем прерывания
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu ();
    sleep_disable(); // cancel sleep as a precaution
}
// OneWire функции:

void OneWireReset() {
     PORTB &= ~_BV(DS_BIT);
     DDRB |= _BV(DS_BIT);
     _delay_us(500);
     DDRB &= ~_BV(DS_BIT);
	 PORTB |= _BV(DS_BIT);
     _delay_us(500);
}
void OneWireOutByte(uint8_t d) {
   uint8_t n;

   for(n=8; n!=0; n--)
   {
      if ((d & 0x01) == 1) 
      {

     PORTB &= ~_BV(DS_BIT);
     DDRB |= _BV(DS_BIT);
         _delay_us(5);
     DDRB &= ~_BV(DS_BIT);
	 PORTB |= _BV(DS_BIT);
         _delay_us(60);
      }
      else
      {
     PORTB &= ~_BV(DS_BIT);
     DDRB |= _BV(DS_BIT);
         _delay_us(60);
     DDRB &= ~_BV(DS_BIT);
	 PORTB |= _BV(DS_BIT);
      }
      d=d>>1; 
   }
}
uint8_t OneWireInByte() {
    uint8_t d=0, n,b;

    for (n=8; n; n--)
    {
    PORTB &= ~_BV(DS_BIT);
    DDRB |= _BV(DS_BIT);
        _delay_us(5);
    DDRB &= ~_BV(DS_BIT);
	PORTB |= _BV(DS_BIT);
        _delay_us(5);
    b = ((PINB & _BV(DS_BIT)) != 0);
        _delay_us(50);
        d = (d >> 1) | (b<<7);
    }
    return(d);
}
/*
uint8_t Crc8(uint8_t inbyte, uint8_t crc)
{	
	uint8_t i;
		for (i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	return crc;
}
*/
int ds18b20read () {// если в программе имеются прерывания,то не забывайте их отлючать перед чтением датчика

    // установка режима для ds
   // DDRB |= _BV(DS_BIT);
    //PORTB &= ~_BV(DS_BIT);
    //DDRB &= ~_BV(DS_BIT);
	//PORTB |= _BV(DS_BIT);

   // ds              
       
     OneWireReset();
     OneWireOutByte(0xcc);
     OneWireOutByte(0x44);
     
       PORTB |= _BV(DS_BIT);
       DDRB |= _BV(DS_BIT);
       //delay(1000); // если хотим ждать когда датчик посчитает температуру.
       sleepFor8Secs(0);
     
     OneWireReset();
     OneWireOutByte(0xcc);
     OneWireOutByte(0xbe);


     uint8_t DSdata[8];
    uint8_t SignBit;
	DSdata[0]=OneWireInByte();
	DSdata[1]=OneWireInByte();
     for( SignBit =2; SignBit<9; SignBit++){
     	DSdata[2]=OneWireInByte();
     }
     if( DSdata[0] != 0 || DSdata[1] != 0 || DSdata[2] != 0){
		
		int TReading = (int)(DSdata[1] << 8) + DSdata[0];
        SignBit = TReading & 0x8000; 
        if (SignBit) TReading = (TReading ^ 0xffff) + 1;
        TReading = ((6 * TReading) + (TReading >> 2))+10000;
		DSdata[6] = ((TReading >> 8) & 0xff);
		DSdata[7] = ((TReading) & 0xff);
        mirf_CSN_lo;
        _delay_ms(100);
        mirf_CSN_hi;

        // need time to come out of power down mode s. 6.1.7, table 16  datasheet says 1.5ms max, tested as low as 600us
        mirf_config_register(CONFIG, mirf_CONFIG | (1<<PWR_UP) );
        _delay_ms(1);

        mirf_send(DSdata, mirf_PAYLOAD);
        _delay_us(10);
        mirf_config_register(CONFIG, mirf_CONFIG );
        sleepFor8Secs(0);
        // Reset status register for further interaction 
        mirf_config_register(STATUS,(1<<TX_DS)); // Reset status register

        return 0;
	}
    return 1;
}

int main() {
    int f;
    PORTB&=0; // turn off the pin power
    DDRB&=0; //change pin mode to reduce power consumption

	ADCSRA &= ~(1<<ADEN);                     //turn off ADC
	ACSR |= _BV(ACD);                         //disable the analog comparator

    mirf_init();

    // wait for mirf - is this necessary?
    //_delay_ms(50);

    mirf_config();

    mirf_config_register(SETUP_RETR, 0 ); // no retransmit 
    // disable enhanced shockburst
    mirf_config_register(EN_AA, 0 ); // no auto-ack 
    //mirf_config_register(RF_SETUP, (1<<RF_DR_LOW) ); // low spd & power 
    mirf_set_TADDR((uint8_t *)"1Sens");
    	
	for(;;) {
        f=ds18b20read();
		if(f==0) {
            for (f = 0; f < SLEEPDURATION; f++) {
                sleepFor8Secs(1);//1);
                //_delay_ms(800);
            }
		}

	}

}

