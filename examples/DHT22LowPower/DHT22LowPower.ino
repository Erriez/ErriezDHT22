/*
 * MIT License
 *
 * Copyright (c) 2018-2020 Erriez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*!
 * \brief DHT22 - AM2302/AM2303 temperature and relative humidity sensor for Arduino
 * \details
 *      This example works only on low-power AVR targets.
 *
 *      Arduino Pro or Pro Mini at 8MHz is recommended with power LED removed. The power consumption
 *      is around <70uA in sleep.
 *
 *      Source:         https://github.com/Erriez/ErriezDHT22
 *      Documentation:  https://erriez.github.io/ErriezDHT22
 *      Required libraries: https://github.com/rocketscream/Low-Power
 */


#include <LowPower.h>
#include <ErriezDHT22.h>

// Connect DTH22 DAT pin to Arduino DIGITAL pin
#if defined(ARDUINO_ARCH_AVR)
#define DHT22_PIN      2
#else
#error "Unsupported target"
#endif

// Create DHT22 sensor object
DHT22 dht22 = DHT22(DHT22_PIN);


void setup()
{
    // Initialize serial port
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println(F("DHT22 temperature and humidity sensor duration test\n"));

    // Initialize sensor
    dht22.begin();
}

void loop()
{
    // Read temperature and humidity
    dht22.readSensorData();

    // Print temperature
    printTemperature(readTemperature());

    // Print humidity
    printHumidity(readHumidity());

    // Sleep at least 2 seconds
    Serial.flush();
    LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_ON);
}

int16_t readTemperature()
{
    int16_t temperature;

    // Read temperature from sensor (blocking)
    temperature = dht22.readTemperature();

    // Check valid temperature value
    if (temperature == ~0) {
        // Temperature error (Check hardware connection)
        Serial.println(F("Temperature read error"));
    }

    return temperature;
}

int16_t readHumidity()
{
    int16_t humidity;

    // Read humidity from sensor (blocking)
    humidity = dht22.readHumidity();

    // Check valid humidity value
    if (humidity == ~0) {
        // Humidity error (Check hardware connection)
        Serial.println(F("Humidity read error"));
    }

    return humidity;
}

void printTemperature(int16_t temperature)
{
    // Check valid temperature value
    if (temperature == ~0) {
        // Temperature error (Check hardware connection)
        Serial.println(F("Temperature: Error"));
    } else {
        // Print temperature
        Serial.print(F("Temperature: "));
        Serial.print(temperature / 10);
        Serial.print(F("."));
        Serial.print(temperature % 10);

        // Print degree Celsius symbols
        // Choose if (1) for normal or if (0) for extended ASCII degree symbol
        if (1) {
            // Print *C characters which are displayed correctly in the serial
            // terminal of the Arduino IDE
            Serial.println(F(" *C"));
        } else {
            // Note: Extended characters are not supported in the Arduino IDE and
            // displays ?C. This is displayed correctly with other serial terminals
            // such as Tera Term.
            // Degree symbol is ASCII code 248 (decimal).
            char buf[4];
            snprintf_P(buf, sizeof(buf), PSTR(" %cC"), 248);
            Serial.println(buf);
        }
    }
}

void printHumidity(int16_t humidity)
{
    // Check valid humidity value
    if (humidity == ~0) {
        // Humidity error (Check hardware connection)
        Serial.println(F("Humidity: Error"));
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
