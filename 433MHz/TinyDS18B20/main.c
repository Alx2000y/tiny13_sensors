#include <avr/io.h>
#include <avr/wdt.h> // здесь организована работа с ватчдогом
#include <avr/sleep.h> // здесь описаны режимы сна
#include <avr/interrupt.h> // работа с прерываниями
//#include "timeout.h"
#include <util/delay.h>     /* for _delay_us() */

#define RC_BIT         3 // pin 2 -  TRANSMITPIN
#define periodusec 400 // mcs период

#define ACCESSORIESPOWERPIN 2   //pin 7
#define DS_BIT         4 // pin 3  pullup резистор не нужен
#define keyT	14000

#define SLEEPDURATION 8//0 //sleep duration in seconds/8, shall be a factor of 8

// подпрограмма отправки данных
void sendRC(unsigned long data) { // Отправка данных по радиоканалу.
    DDRB |= _BV(RC_BIT);
    data = data & 0xfffff; //truncate to 20 bit
    unsigned long dataBase4 = 0;
    uint8_t i;
    for (i=0; i<12; i++) {
        dataBase4<<=2;
        dataBase4|=(data%3);
        data/=3;
    }
    unsigned short int j;
    for (j=0;j<8;j++) {
        data=dataBase4;
        for (i=0; i<12; i++) {
            switch (data & 3) {
                case 0:
                    PORTB |= _BV(RC_BIT);
                    _delay_us(periodusec);
                    PORTB &= ~_BV(RC_BIT);
                    _delay_us(periodusec*3);
                    PORTB |= _BV(RC_BIT);
                    _delay_us(periodusec);
                    PORTB &= ~_BV(RC_BIT);
                    _delay_us(periodusec*3);
                    break;
                case 1:
                    PORTB |= _BV(RC_BIT);
                    _delay_us(periodusec*3);
                    PORTB &= ~_BV(RC_BIT);
                    _delay_us(periodusec);
                    PORTB |= _BV(RC_BIT);
                    _delay_us(periodusec*3);
                    PORTB &= ~_BV(RC_BIT);
                    _delay_us(periodusec);
                    break;
                case 2: //AKA: X or float
                    PORTB |= _BV(RC_BIT);
                    _delay_us(periodusec);
                    PORTB &= ~_BV(RC_BIT);
                    _delay_us(periodusec*3);
                    PORTB |= _BV(RC_BIT);
                    _delay_us(periodusec*3);
                    PORTB &= ~_BV(RC_BIT);
                    _delay_us(periodusec);
                    break;
            }
            data>>=2;
        }
        PORTB |= _BV(RC_BIT);
        _delay_us(periodusec);
        PORTB &= ~_BV(RC_BIT);
        _delay_us(periodusec*31);
    }
}

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

 DDRB |= _BV(DS_BIT);
   for(n=8; n!=0; n--)
   {
      if ((d & 0x01) == 1) 
      {

     PORTB &= ~_BV(DS_BIT);
     //DDRB |= _BV(DS_BIT);
         _delay_us(5);
     //DDRB &= ~_BV(DS_BIT);
	 PORTB |= _BV(DS_BIT);
         _delay_us(60);
      }
      else
      {
     PORTB &= ~_BV(DS_BIT);
     //DDRB |= _BV(DS_BIT);
         _delay_us(60);
     //DDRB &= ~_BV(DS_BIT);
	 PORTB |= _BV(DS_BIT);
      }
      d=d>>1; 
   }
}
uint8_t OneWireInByte() {
    uint8_t d=0, n,b;

    for (n=0; n<8; n++)
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
void ds18b20read () {// если в программе имеются прерывания,то не забывайте их отлючать перед чтением датчика

	DDRB|=(1<<ACCESSORIESPOWERPIN); //pin as output //pinMode(ACCESSORIESPOWERPIN, OUTPUT);
	PORTB|=(1<<ACCESSORIESPOWERPIN); //turn on the DHT sensor and the transmitter //digitalWrite(ACCESSORIESPOWERPIN, HIGH);


    // установка режима для ds
    //DDRB |= _BV(DS_BIT);
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

      uint8_t SignBit;
      uint8_t DSdata[3];
      DSdata[0] = OneWireInByte();
      DSdata[1] = OneWireInByte();
     for( SignBit =2; SignBit < 9; SignBit++){
         DSdata[2] = OneWireInByte();
     }
    if( DSdata[0]!=0 || DSdata[1]!=0 || DSdata[2]!=0) {
    	int TReading = (int)(DSdata[1] << 8) + DSdata[0];
        SignBit = TReading & 0x8000; 
        if (SignBit) TReading = (TReading ^ 0xffff) + 1;
        sendRC(((6 * TReading) + TReading/4)/10+500+keyT); // отправляем данные
        PORTB=0; // turn off the accessories power  //digitalWrite(ACCESSORIESPOWERPIN, LOW);
		DDRB=0; //change pin mode to reduce power consumption  //pinMode(ACCESSORIESPOWERPIN, INPUT);
	}

     /*
      uint8_t SignBit;
      uint8_t DSdata;

        int TReading;
		 uint8_t crc=0;
         DSdata = OneWireInByte();
         TReading = DSdata;
         DSdata = OneWireInByte();
         TReading += (DSdata << 8);
		
     for( SignBit =2; SignBit < 9; SignBit++){
         DSdata = OneWireInByte();
     }

    if( DSdata!=0 || TReading!=0 ) {
        SignBit = TReading & 0x8000; 
        if (SignBit) TReading = (TReading ^ 0xffff) + 1;
        sendRC(((6 * TReading) + TReading/4)/10+500+keyT); // отправляем данные
        PORTB=0; // turn off the accessories power  //digitalWrite(ACCESSORIESPOWERPIN, LOW);
		DDRB=0; //change pin mode to reduce power consumption  //pinMode(ACCESSORIESPOWERPIN, INPUT);
	}
	*/
}
int main() {
    int f;
    PORTB&=0; // turn off the pin power
    DDRB&=0; //change pin mode to reduce power consumption

	ADCSRA &= ~(1<<ADEN);                     //turn off ADC
	ACSR |= _BV(ACD);                         //disable the analog comparator
	for(;;) {
        ds18b20read();
            for (f = 0; f < SLEEPDURATION; f++) {
                sleepFor8Secs(1);
                //_delay_ms(800);
            }
	}
}

