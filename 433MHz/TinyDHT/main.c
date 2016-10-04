#include <avr/io.h>
#include <avr/wdt.h> // здесь организована работа с ватчдогом
#include <avr/sleep.h> // здесь описаны режимы сна
#include <avr/interrupt.h> // работа с прерываниями
#include "timeout.h"
//#include <util/delay.h>     /* for _delay_us() */


#define ACCESSORIESPOWERPIN 2   //pin 7
#define RC_BIT         3 // pin 2 -  TRANSMITPIN
#define periodusec 400 // mcs период

#define DHT_BIT         4 // pin 3 -  TRANSMITPIN
#define DHT_PORT        PORTB // порт
#define DHT_DDR         DDRB
#define DHT_PIN         PINB

#define SLEEPDURATION 8//0 //sleep duration in seconds/8, shall be a factor of 8

#define key1  11000
#define key2  12000

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

void sleepFor8Secs() {
	cli();
    MCUSR = 0;  // clear various "reset" flags
    //инициализация ватчдога
    wdt_reset(); // сбрасываем
    wdt_enable(WDTO_8S); // разрешаем ватчдог 8 сек
	//WDTCR |= _BV(WDCE);
	WDTCR &= ~_BV(WDE);
    WDTCR |= _BV(WDTIE); // разрешаем прерывания по ватчдогу. Иначе будет резет.
	sei(); // разрешаем прерывания
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu ();
    sleep_disable(); // cancel sleep as a precaution
}

int dhtread () {// если в программе имеются прерывания,то не забывайте их отлючать перед чтением датчика
    uint8_t  j = 0, i = 0;
    uint8_t datadht[5];
    //datadht[0] = datadht[1] = datadht[2] = datadht[3] = datadht[4] = 0;  
	DDRB|=(1<<ACCESSORIESPOWERPIN); //pin as output //pinMode(ACCESSORIESPOWERPIN, OUTPUT);
	PORTB|=(1<<ACCESSORIESPOWERPIN); //turn on the DHT sensor and the transmitter //digitalWrite(ACCESSORIESPOWERPIN, HIGH);
	_delay_ms(2000); //sleep till the intermediate processes in the accessories are settled down

    DHT_DDR|=(1<<DHT_BIT); //pin as output
    DHT_PORT&=~(1<<DHT_BIT);
    _delay_ms(18);
    DHT_PORT|=(1<<DHT_BIT);
    DHT_DDR&=~(1<<DHT_BIT);
    _delay_us(50); // +1 для attiny(коррекция без кварца)
    if (DHT_PIN&(1<<DHT_BIT)) return 3;
    _delay_us(80); // +1 для attiny(коррекция без кварца)
    if (!(DHT_PIN&(1<<DHT_BIT))) return 4;
    while (DHT_PIN&(1<<DHT_BIT));
    for (j=0; j<5; j++) {
        datadht[j]=0;
        for(i=0; i<8; i++) {
            while (!(DHT_PIN&(1<<DHT_BIT)));
            _delay_us (30);
            if (DHT_PIN&(1<<DHT_BIT)) 
                datadht[j]|=1<<(7-i);
            while (DHT_PIN&(1<<DHT_BIT));
            }
    }
	if( datadht[4] == ( (datadht[0] + datadht[1] + datadht[2] + datadht[3]) & 0xFF ) ){
        int f;
        PORTB&=~(1<<ACCESSORIESPOWERPIN); // turn off the accessories power  //digitalWrite(ACCESSORIESPOWERPIN, LOW);
		DDRB&=~(1<<ACCESSORIESPOWERPIN); //change pin mode to reduce power consumption  //pinMode(ACCESSORIESPOWERPIN, INPUT);

        f=(datadht[2] & 0x7F)* 256 + datadht[3];
        if (datadht[2] & 0x80)  f *= -1;
        sendRC(f+key1+500);
        _delay_ms(1000);
        f=datadht[0] * 256 + datadht[1];
        sendRC(f+key2);
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
	for(;;) {
        f=dhtread();
		if(f==0) {
            for (f = 0; f < SLEEPDURATION; f++) {
                sleepFor8Secs();
            }
		}
	}
}

