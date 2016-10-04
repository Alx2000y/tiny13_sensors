#define MOSI_PIN	(1<<PB0)
#define MISO_PIN 	(1<<PB1)
#define SCK_PIN 	(1<<PB2)

#define CE  PB3
#define CSN PB4

#define DHT_BIT         3 // pin 2  pullup резистор не нужен


nc   -|    |- VCC CE
DHT  -|    |- SCK
CSN  -|    |- MISO
GND  -|    |- MOSI

