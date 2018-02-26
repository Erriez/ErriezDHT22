# DHT22 temperature and humidity sensor library for Arduino

This is an optimized AM2303 temperature and humidity sensor on a DHT22 breakout.

![AM2302 DHT22 sensor](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/AM2302_DHT22_sensor.png)



## Library features

- Synchronous 16-bit temperature read
- Synchronous 16-bit humidity read




## AM2303 specifications

- Voltage: 3.3 .. 5V
- Ultra-low power:
  - Typical 15uA dormancy
  - Typical 500uA measuring
- Single wire serial interface
- Humidity:
  - Range: 0 .. 99.9 %RH (Relative Humidity)
  - Resolution:  0.1 %RH
  - Accuracy: +/-2 %RH (at 25 degree Celsius)
- Temperature:
  - Range: -40 .. +125 degree Celsius
  - Resolution: 0.1 degree Celsius
  - Accuracy: +/- 0.4 degree Celsius
- Minimum read interval: 2000 ms



## Hardware

![Schematic DHT22 and Arduino UNO](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/DHT22_Arduino_UNO.png)

**Connection DHT22 - Arduino**

| DHT22 | Arduino UNO/Nano/Leonardo/Mega2560 |
| :---: | :--------------------------------: |
|  GND  |                GND                 |
|  VCC  |            5V (or 3.3V)            |
|  DAT  |                 D2                 |

**Connection DHT22 - ESP8266**

| DHT22 | ESP8266 / WeMos D1 R2 / ESP12E / NodeMCU |
| :---: | ---------------------------------------- |
|  GND  | GND                                      |
|  VCC  | 3.3V                                     |
|  DAT  | Arduino pin 2 -> GPIO4 = D4              |

**Note:** Some ESP8266 boards uses Arduino pin 2 -> GPIO4 which is D4 text on the board. Make sure you're using the right pin.


**Connection DHT22 - Lolin32**

| DHT22 | WeMos Lolin32 |
| :---: | ------------- |
|  GND  | GND           |
|  VCC  | 3.3V          |
|  DAT  | 2             |

## Supported Arduino Boards

- All ATMega328P MCU:
  - Arduino UNO
  - Arduino Nano
- All ATMega32U4 MCU's:
  - Arduino Leonardo
  - Pro Micro
- All ATMega2560 MCU's:
  - Arduino Mega2560
- All ESP8266 boards:
  - WeMos D1 R2
  - NodeMCU
- All Lolin32 boards:
  - WeMos Lolin32


- Other MCU's may work, but are not tested.



## Library dependencies

* None



## Documentation

[Doxygen](https://github.com/Erriez/ErriezDHT22/raw/master/doc/latex/refman.pdf)

[AM2303 datasheet](http://www.aosong.com/asp_bin/Products/en/AM2303.pdf)

[DHT22 datasheet](https://www.google.com/search?q=DHT22+datasheet)



## Examples

Examples | ErriezDH22 | [Example](https://github.com/Erriez/ErriezDHT22/blob/master/examples/Example/Example.ino)



## Usage

**Initialization**

```c++
#include <DHT22.h>
  
// Connect DTH22 data pin to Arduino DIGITAL pin
#define DHT22_PIN   2
  
DHT22 sensor = DHT22(DHT22_PIN);
  
void setup()
{
    // Initialize serial port
    Serial.begin(115200);
    Serial.println(F("DHT22 temperature and humidity sensor example\n"));
    
    // Initialize sensor
    sensor.begin();
}
```



**Read temperature and humidity**

```c++
void loop()
{
  // Check minimum interval of 2000 ms between sensor reads
  if (sensor.available()) {
    // Read temperature from sensor
    int16_t temperature = sensor.readTemperature();
  
    // Read humidity from sensor
    int16_t humidity = sensor.readHumidity();
  
    // Print temperature
    Serial.print(F("Temperature: "));
    Serial.print(temperature / 10);
    Serial.print(F("."));
    Serial.print(temperature % 10);
    Serial.println(F(" *C"));
  
    // Print humidity
    Serial.print(F("Humidity: "));
    Serial.print(humidity / 10);
    Serial.print(F("."));
    Serial.print(humidity % 10);
    Serial.println(F(" %\n"));
  }
}
```



**Serial output**

```
DHT22 temperature and humidity sensor example
  
Temperature: 17.7 *C
Humidity: 41.0 %
  
Temperature: 17.8 *C
Humidity: 41.1 %
  
...
```

