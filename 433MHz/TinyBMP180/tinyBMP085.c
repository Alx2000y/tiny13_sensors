/*************************************************** 
 * This is a library for the BMP085 Barometric Pressure & Temp Sensor
 * 
 * Designed specifically to work with the Adafruit BMP085 Breakout 
 * ----> https://www.adafruit.com/products/391
 * 
 * These displays use I2C to communicate, 2 pins are required to  
 * interface
 * Adafruit invests time and resources providing this open source code, 
 * please support Adafruit and open-source hardware by purchasing 
 * products from Adafruit!
 * 
 * Written by Limor Fried/Ladyada for Adafruit Industries.  
 * BSD license, all text above must be included in any redistribution
 * 
 * -~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
 * modified to work with the TinyWire Master library (TinyWireM)
 * by johngineer 10-31-2011 -- boo!
 * 
 * NOTE: to make use of the debug code herein you MUST include the
 * ATTiny serial library in your main program.
 * 
 * -------------------------------------------------------
 * modified by Michal Canecky/Cano 2013-05-05
 * -calculation of altitude without using pow() and math library
 * -calculation of altitude using only integers 
 * 	(fixed for standard sea level pressure)
 * 
 ****************************************************/
#include "tinybmp085.h"
    /*uint8_t oversampling;*/

    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;

uint8_t tinyBMP085_begin(uint8_t mode) {
    /*if (mode > BMP085_ULTRAHIGHRES) 
        mode = BMP085_ULTRAHIGHRES;
    oversampling = mode;
    */

	int ret=i2c_start(BMP085_I2CADDR);

    if (ret != 0x55) return 0;

    /* read calibration data */
    ac1 = tinyBMP085_read16(BMP085_CAL_AC1);
    ac2 = tinyBMP085_read16(BMP085_CAL_AC2);
    ac3 = tinyBMP085_read16(BMP085_CAL_AC3);
    ac4 = tinyBMP085_read16(BMP085_CAL_AC4);
    ac5 = tinyBMP085_read16(BMP085_CAL_AC5);
    ac6 = tinyBMP085_read16(BMP085_CAL_AC6);

    b1 = tinyBMP085_read16(BMP085_CAL_B1);
    b2 = tinyBMP085_read16(BMP085_CAL_B2);

    mb = tinyBMP085_read16(BMP085_CAL_MB);
    mc = tinyBMP085_read16(BMP085_CAL_MC);
    md = tinyBMP085_read16(BMP085_CAL_MD);

    return 1;
}

uint16_t tinyBMP085_readRawTemperature(void) {
    tinyBMP085_write8(BMP085_CONTROL, BMP085_READTEMPCMD);
    _delay_ms(5);
    return tinyBMP085_read16(BMP085_TEMPDATA);
}

uint32_t tinyBMP085_readRawPressure(void) {
    uint32_t raw;

    tinyBMP085_write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (BMP085_STANDARD << 6));

    /*if (oversampling == BMP085_ULTRALOWPOWER) 
        _delay_ms(5);
    else if (oversampling == BMP085_STANDARD) */
        _delay_ms(8);
    /*else if (oversampling == BMP085_HIGHRES) 
        _delay_ms(14);
    else 
        _delay_ms(26);
    */

    raw = tinyBMP085_read16(BMP085_PRESSUREDATA);
    raw <<= 8;
    raw |= tinyBMP085_read8(BMP085_PRESSUREDATA+2);
    raw >>= (8 - BMP085_STANDARD);
    return raw;
}


// returns pressure in Pa
int32_t tinyBMP085_readPressure(void) {
    /*int32_t UT, UP, p;
    UT = tinyBMP085_readRawTemperature();
    UP = tinyBMP085_readRawPressure();
    //see manual http://media.digikey.com/pdf/Data%20Sheets/Bosch/BMP085.pdf page 13
    
    int32_t B3, B5, B6, X1, X2, X3;
    uint32_t B4, B7;

    // do temperature calculations
    X1=((UT-(int32_t)(ac6))*((int32_t)(ac5))) >> 15;
    X2=((int32_t)mc << 11)/(X1+(int32_t)md);
    B5=X1 + X2;

    // do pressure calcs
    B6 = B5 - 4000;
    X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
    X2 = ((int32_t)ac2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) >> 2;

    X1 = ((int32_t)ac3 * B6) >> 13;
    X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
    B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

    if (B7 < 0x80000000) {
        p = (B7 << 1) / B4;
    } 
    else {
        p = (B7 / B4) << 1;
    }
    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;

    p = p + ((X1 + X2 + (int32_t)3791)>>4);
    return p;
    */
    return tinyBMP085_readRawPressure();
}

//return temperature in 0.1C
int16_t tinyBMP085_readTemperature10C(void) {
    /*int32_t UT;     // following ds convention

    UT = tinyBMP085_readRawTemperature();
    
    int32_t X1, X2, B5;
    X1 = ((UT - (int32_t)ac6) * ((int32_t)ac5)) >> 15;
    X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
    B5 = X1 + X2;
    return (B5 + 8) >> 4;
    */
    return tinyBMP085_readRawTemperature();
}


uint8_t tinyBMP085_read8(uint8_t a) {
    return (uint8_t) tinyBMP085_readwrite(a, 0, 1);
}

uint16_t tinyBMP085_read16(uint8_t a) {
    return tinyBMP085_readwrite(a, 0, 2);
}
uint16_t tinyBMP085_readwrite(uint8_t a, uint8_t d, uint8_t bytes) {
    uint16_t ret=0;
    i2c_start_wait( BMP085_I2CADDR+I2C_WRITE );   
    i2c_write(a);
    if(bytes==0)
        i2c_write(d);
    i2c_rep_start( BMP085_I2CADDR+I2C_READ); 
    if(bytes!=1) {
    ret = i2c_readAck();
    ret <<= 8;
    }
    ret |= i2c_readNak();
    i2c_stop();
    return ret;
}
void tinyBMP085_write8(uint8_t a, uint8_t d) {
    tinyBMP085_readwrite(a, d, 0);
}
