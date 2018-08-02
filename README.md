# DHT22 - AM2303 temperature and humidity sensor library for Arduino
[![Build Status](https://travis-ci.org/Erriez/ErriezDHT22.svg?branch=master)](https://travis-ci.org/Erriez/ErriezDHT22)

This is a calibrated AM2303 digital temperature and relative humidity sensor on a DHT22 breakout PCB.

![AM2302 DHT22 sensor](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/AM2302_DHT22_sensor.png)


## Library features

- Read 16-bit temperature (synchronous blocking)
- Read 16-bit relative humidity (synchronous blocking)


## AM2303 sensor specifications

- Voltage: 3.3 .. 5V
- Ultra-low power:
  - Typical 15uA dormancy
  - Typical 500uA measuring
- Single wire digital serial interface
- Calibrated digital signal
- Outstanding long term stability
- No additional electronic components needed
- Humidity:
  - Range: 0 .. 99.9 %RH (Relative Humidity)
  - Resolution:  0.1 %RH
  - Accuracy: +/-2 %RH (at 25 degree Celsius)
- Temperature:
  - Range: -40 .. +125 degree Celsius
  - Resolution: 0.1 degree Celsius
  - Accuracy: +/- 0.4 degree Celsius
- Minimum read interval: 2000 ms
- ~31ms to synchronous read humidity, temperature and parity data from sensor (5 Bytes)


## Safety warning

According to the datasheet, the AM2302 is a low cost consumer temperature sensor. It may not be
used in safety critical applications, emergency stop devices or any other occasion that failure of
AM2303 may cause personal injury.


## Hardware

![Schematic DHT22 and Arduino UNO](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/DHT22_Arduino_UNO.png)

**Pull-up resistor DAT pin**

* Connect an external ```3k3..10k``` pull-up resistor between the ```DAT``` and ```VCC``` pins only when:
  * Using a AM2302 sensor without a DT22 breakout PCB **and** the MCU IO pin has no built-in or external pull-up resistor.
* The DHT22 breakout PCB contains a ```3k3``` pull-up resistor between ```DAT``` and ```VCC```.
* Please refer to the MCU datasheet or board schematic for more information about IO pin pull-up resistors.

**External capacitor**

* Tip: Connect a ```100nF``` capacitor between the sensor pins ```VCC``` and ```GND``` when read errors occurs. This may stabilize the power supply.

**Connection DHT22 - Arduino**

| DHT22 | Arduino UNO / Nano / Pro Mini / Leonardo / Mega2560 |
| :---: | :-------------------------------------------------- |
|  GND  | GND                                                 |
|  VCC  | 5V (or 3.3V)                                        |
|  DAT  | 2 (DIGITAL pin)                                     |

**Connection DHT22 - ESP8266**

Some ESP8266 boards uses Arduino pin 2 -> GPIO4 which is D4 text on the board. Make sure you're using the right pin.

| DHT22 | ESP8266 / WeMos D1 R2 / ESP12E / NodeMCU |
| :---: | ---------------------------------------- |
|  GND  | GND                                      |
|  VCC  | 3.3V                                     |
|  DAT  | D4                                       |

**Connection DHT22 - WeMos LOLIN32**

WeMos LOLIN32 requires an additional 100nF capacitor over the GND - VCC pins to prevent parity errors.

Use pin 0 to prevent flash problems.

| DHT22 | WeMos Lolin32 |
| :---: | ------------- |
|  GND  | GND           |
|  VCC  | 3.3V          |
|  DAT  | 0             |

Other MCU's may work, but are not tested.


## Examples

Arduino IDE | Examples | Erriez DH22 Temperature & Humidity:

* [DHT22](https://github.com/Erriez/ErriezDHT22/blob/master/examples/DHT22/DHT22.ino)


## Documentation

* [Doxygen online HTML](https://erriez.github.io/ErriezDHT22)
* [Doxygen PDF](https://github.com/Erriez/ErriezDHT22/raw/gh-pages/latex/ErriezDHT22.pdf)
* [AM2303 datasheet](http://www.aosong.com/asp_bin/Products/en/AM2303.pdf)
* [DHT22 datasheet](https://www.google.com/search?q=DHT22+datasheet)


## Usage

**Initialization**

```c++
#include <DHT22.h>
  
// Connect DTH22 DAT pin to Arduino board

// Arduino DIGITAL pin
#define DHT22_PIN      2
// Some ESP8266 boards uses D2 instead of 2
// #define DHT22_PIN   D2
// LOLIN32 uses another pin
// #define DHT22_PIN   0
  
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
  
        if (temperature == ~0) {
            // Print error (Check hardware connection)
            Serial.print(F("Temperature: Error"));
        } else {
            // Print temperature
            Serial.print(F("Temperature: "));
            Serial.print(temperature / 10);
            Serial.print(F("."));
            Serial.print(temperature % 10);
            Serial.println(F(" *C"));
        }
  
        if (humidity == ~0) {
            // Print error (Check hardware connection)
            Serial.print(F("Humidity: Error"));
        } else {
            // Print humidity
            Serial.print(F("Humidity: "));
            Serial.print(humidity / 10);
            Serial.print(F("."));
            Serial.print(humidity % 10);
            Serial.println(F(" %"));
        }
        
        Serial.println();
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


## Library dependencies

* None


## Library installation

Please refer to the [Wiki](https://github.com/Erriez/ErriezArduinoLibrariesAndSketches/wiki) page.


## Other Arduino Libraries and Sketches from Erriez

* [Erriez Libraries and Sketches](https://github.com/Erriez/ErriezArduinoLibrariesAndSketches)
