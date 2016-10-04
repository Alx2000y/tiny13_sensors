#include <avr/io.h>
#include <avr/wdt.h> // здесь организована работа с ватчдогом
#include <avr/sleep.h> // здесь описаны режимы сна
#include <avr/interrupt.h> // работа с прерываниями
#include "timeout.h"
//#include <util/delay.h>     /* for _delay_us() */
#include "i2cmaster.h"
//#include "tinybmp085.h"




#define RC_BIT         3 // pin 2 -  TRANSMITPIN
#define periodusec 400 // mcs период

#define ACCESSORIESPOWERPIN 2   //pin 7
#define DS_BIT         4 // pin 3
#define BMP180_ADDR   0x77              // 7 bit I2C address for sensor
#define keyT	16000

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

int bmp180read() {// если в программе имеются прерывания,то не забывайте их отлючать перед чтением датчика

      uint8_t SignBit;

    //tinyBMP085_begin(BMP085_STANDARD);
    int32_t pres = 0;//tinyBMP085_readPressure();
    int16_t temp = 0;//tinyBMP085_readTemperature10C();

    if( temp != 0 && pres != 0 ){
        PORTB&=~(1<<ACCESSORIESPOWERPIN); // turn off the accessories power  //digitalWrite(ACCESSORIESPOWERPIN, LOW);
		DDRB&=~(1<<ACCESSORIESPOWERPIN); //change pin mode to reduce power consumption  //pinMode(ACCESSORIESPOWERPIN, INPUT);

        int TReading = temp;

        SignBit = TReading & 0x8000; 
        if (SignBit) TReading = (TReading ^ 0xffff) + 1;

        sendRC(((6 * TReading) + TReading / 4)/10+500+keyT); // отправляем данные
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
	i2c_init();                                // init I2C interface
      
	for(;;) {
        f=bmp180read();
		if(f==0) {
            for (f = 0; f < SLEEPDURATION; f++) {
                sleepFor8Secs(1);
                //_delay_ms(800);
            }
		}
	}
}

