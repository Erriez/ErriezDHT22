# DHT22 temperature and humidity sensor library for Arduino

This is a AM2303 temperature and humidity sensor on a DHT22 breakout.

![AM2302 DHT22 sensor](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/AM2302_DHT22_sensor.png)



## Library features

- Synchronous 16-bit temperature read
- Synchronous 16-bit humidity read



## Hardware

![Schematic DHT22 and Arduino UNO](https://raw.githubusercontent.com/Erriez/ErriezDHT22/master/extras/DHT22_Arduino_UNO.png)

**Connection DHT22 - Arduino UNO**

| DHT22 | Arduino UNO  |
| :---: | :----------: |
|  GND  |     GND      |
|  VCC  | 5V (or 3.3V) |
|  DAT  |      D2      |



## Documentation

[AM2303 datasheet](http://www.aosong.com/asp_bin/Products/en/AM2303.pdf)

[DHT22 datasheet](https://www.google.com/search?q=DHT22+datasheet)



## AM2303 specifications

- Voltage: 3.3 .. 5V
- Ultra-low power:
  - Typical 15uA dormancy
  - Typical 500uA measuring
- Single wire serial interface
- Humidity:
  - Range: 0 .. 99.9 %RH (Relative Humidity)
  - Resolution:  0.1 %RH
  - Accuracy: +/-2 %RH (at 25 degree celsius)
- Temperature:
  - Range: -40 .. +125 degree celsius
  - Resolution: 0.1 degree celsius
  - Accuracy: +/- 0.4 degree celsius
- Minimum read interval: 2000 ms



## Examples

Examples | ErriezDH22:

* [Example](https://github.com/Erriez/ErriezDHT22/blob/master/examples/Example/Example.ino)



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

