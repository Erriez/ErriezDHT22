# DHT22 - AM2302/AM2303 temperature and humidity sensor library for Arduino
[![Build Status](https://travis-ci.org/Erriez/ErriezDHT22.svg?branch=master)](https://travis-ci.org/Erriez/ErriezDHT22)

This is an Arduino library for the calibrated AM2302/AM2303 digital temperature and relative humidity sensor on a DHT22 breakout PCB.

![DHT22 temperature and humidity sensor](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/AM2302_DHT22_sensor.png)


## Library features

- Read 16-bit temperature (synchronous blocking)
- Read 16-bit relative humidity (synchronous blocking)
- Configurable number of read retries when a read error occurs (default is 1 read + 2 retries)
- Long time duration example
- Temperature and humidity average with a configurable number of samples to remove jitter


## AM2302/AM2303 sensor specifications

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

According to the datasheet, the AM2302/AM2303 is a low cost consumer temperature sensor. It may not
be used in safety critical applications, emergency stop devices or any other occasion that failure
of AM2302/AM2303 may cause personal injury.


## Hardware

![Schematic DHT22 and Arduino UNO](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/DHT22_Arduino_UNO.png)

**Pull-up resistor DAT pin**

* Connect an external ```3k3..10k``` pull-up resistor between the ```DAT``` and ```VCC``` pins only when:
  * Using a AM2302/AM2303 sensor without a DT22 breakout PCB **and** the MCU IO pin has no built-in or external pull-up resistor.
* The DHT22 breakout PCB contains a ```3k3``` pull-up resistor between ```DAT``` and ```VCC```.
* Please refer to the MCU datasheet or board schematic for more information about IO pin pull-up resistors.

**External capacitor**

* Tip: Connect a ```100nF``` capacitor between the sensor pins ```VCC``` and ```GND``` when read errors occurs. This may stabilize the power supply.

| Board - DHT22 pins                                |     VCC     | GND  |       DAT       |
| ------------------------------------------------- | :---------: | :--: | :-------------: |
| Arduino UNO / Nano / Micro (ATMega328 boards)     | 5V (or 3V3) | GND  | 2 (DIGITAL pin) |
| Arduino Leonardo                                  | 5V (or 3V3) | GND  | 2 (DIGITAL pin) |
| Arduino Mega2560                                  | 5V (or 3V3) | GND  | 2 (DIGITAL pin) |
| Arduino DUE (ATSAM3X8E)                           |     3V3     | GND  | 2 (DIGITAL pin) |
| ESP8266 (ESP12E / WeMos D1 R2 / NodeMCU v2 or v3) |     3V3     | GND  |   GPIO4 (D2)    |
| ESP32 (WeMos Lolin32 OLED / WeMos LOLIN D32)      |     3V3     | GND  |      GPIO4      |

Notes: 

* ```GPIO4``` uses sketch pin number ```4``` and is labeled as ```D2``` on some WeMos ESP8266 boards.
* Other MCU's may work, but are not tested.


## Examples

Arduino IDE | Examples | Erriez DHT22 Temperature & Humidity:

* [DHT22](https://github.com/Erriez/ErriezDHT22/blob/master/examples/DHT22/DHT22.ino) Getting started example.
* [DHT22Average](https://github.com/Erriez/ErriezDHT22/blob/master/examples/DHT22Average/DHT22Average.ino) Calculate average temperature and humidity.
* [DHT22DurationTest](https://github.com/Erriez/ErriezDHT22/blob/master/examples/DHT22DurationTest/DHT22DurationTest.ino) Test reliability connection.
* [DHT22Logging](https://github.com/Erriez/ErriezDHT22/blob/master/examples/DHT22Logging/DHT22Logging.ino) Write temperature and humidity every 10 minutes to .CSV file on SD-card with DS3231 RTC.
* [DHT22LowPower](https://github.com/Erriez/ErriezDHT22/blob/master/examples/DHT22LowPower/DHT22LowPower.ino) LowPower AVR targets only. Arduino Pro or Pro Mini at 8MHz is recommended.


## Documentation

* [Doxygen online HTML](https://erriez.github.io/ErriezDHT22)
* [Doxygen PDF](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/ErriezDHT22.pdf)
* [AM2303 datasheet](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/AM2303_datasheet.pdf)
* [DHT22 datasheet](https://www.google.com/search?q=DHT22+datasheet)


## Usage

**Initialization**

```c++
#include <ErriezDHT22.h>

// Connect DTH22 DAT pin to Arduino board

// Connect DTH22 DAT pin to Arduino DIGITAL pin
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_SAM_DUE)
#define DHT22_PIN      2
#elif defined(ESP8266) || defined(ESP32)
#define DHT22_PIN      4 // GPIO4 (Labeled as D2 on some ESP8266 boards)
#else
#error "May work, but not tested on this target"
#endif
  
DHT22 dht22 = DHT22(DHT22_PIN);
  
void setup()
{
    // Initialize serial port
    Serial.begin(115200);
    Serial.println(F("DHT22 temperature and humidity sensor example\n"));
    
    // Initialize sensor
    dht22.begin();
}
```


**Read temperature and humidity**

```c++
void loop()
{
    // Check minimum interval of 2000 ms between sensor reads
    if (dht22.available()) {
        // Read temperature from sensor
        int16_t temperature = dht22.readTemperature();
  
        // Read humidity from sensor
        int16_t humidity = dht22.readHumidity();
  
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

* ```LowPower``` library for ```DHT22LowPower.ino```.


## Library installation

Please refer to the [Wiki](https://github.com/Erriez/ErriezArduinoLibrariesAndSketches/wiki) page.


## Other Arduino Libraries and Sketches from Erriez

* [Erriez Libraries and Sketches](https://github.com/Erriez/ErriezArduinoLibrariesAndSketches)
