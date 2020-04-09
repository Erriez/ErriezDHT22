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
 * \brief DHT22 - AM2302/AM2303 temperature and relative humidity sensor duration test for Arduino
 * \details
 *      Source:         https://github.com/Erriez/ErriezDHT22
 *      Documentation:  https://erriez.github.io/ErriezDHT22
 *
 *      Example output:
 *          Temperature: 23.8 *C
 *          Humidity: 57.9 %
 *          Read retries: 0
 *          Num reads: 124649
 *          Temp errors: 0
 *          Humidity errors: 0
 *          Temp min: 22.1 *C
 *          Temp max: 24.4 *C
 *          Humidity min: 50.8 %
 *          Humidity max: 59.3 %
 */

#if !defined(ARDUINO_SAM_DUE)
#include <EEPROM.h> // EEPROM is not available on DUE

#include <ErriezDHT22.h>

// Connect DTH22 DAT pin to Arduino DIGITAL pin
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_SAM_DUE)
#define DHT22_PIN      2
#elif defined(ESP8266) || defined(ESP32)
#define DHT22_PIN      4 // GPIO4 (Labeled as D2 on some ESP8266 boards)
#else
#error "May work, but not tested on this target"
#endif

// Create DHT22 sensor object
DHT22 dht22 = DHT22(DHT22_PIN);

// EEPROM sensor data signature
#define DHT22_DATA_SIGNATURE        "DHT22"

// Number of temperature and humidity samples for average calculation
#define DHT22_NUM_SAMPLES           10

typedef struct {
    char     signature[6];
    uint32_t numReads;
    uint16_t tempErrors;
    uint16_t humidityErrors;

    int16_t tempMin;
    int16_t tempMax;

    int16_t humidityMin;
    int16_t humidityMax;
} SensorData;

SensorData sensorData;

// Set to true to reset EEPROM statistics
const bool eraseEEPROM = false;

// Function prototypes
void EEPROM_Write(const void *buf, uint8_t bufLength);
void EEPROM_Read(void *buf, uint8_t bufLength);
int16_t readTemperature();
int16_t readHumidity();
void printTemperature(int16_t temperature);
void printHumidity(int16_t humidity);
void printStatus(int16_t temperature, int16_t humidity);
void printDec32(uint32_t val);


void setup()
{
    // Initialize serial port
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println(F("DHT22 temperature and humidity sensor duration test\n"));

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
    // EEPROM initialize is only needed for ESP8622 and ESP32
    EEPROM.begin(sizeof(SensorData));
#endif

    // Initialize sensor
    dht22.begin(DHT22_NUM_SAMPLES);

    // Read status from EEPROM
    EEPROM_Read(&sensorData, sizeof(sensorData));

    // Check if EEPROM statistics should be erased
    if (eraseEEPROM ||
        (memcmp(&sensorData.signature, DHT22_DATA_SIGNATURE, sizeof(sensorData.signature)) != 0))
    {
        // Clear sensor data
        memset(&sensorData, 0, sizeof(sensorData));

        // Set signature
        memcpy(&sensorData.signature, DHT22_DATA_SIGNATURE, sizeof(sensorData.signature));

        // Set minimum temperature and humidity to maximum unsigned value
        sensorData.tempMin = ((~(uint16_t)0)>>1);
        sensorData.humidityMin = ((~(uint16_t)0)>>1);

        // Write data to EEPROM
        EEPROM_Write(&sensorData, sizeof(sensorData));
    }
}

void loop()
{
    int16_t temperature;
    int16_t humidity;

    // Check minimum interval of 2000 ms between sensor reads
    if (dht22.available()) {
        sensorData.numReads++;

        // Read temperature
        temperature = readTemperature();

        // Read humidity
        humidity = readHumidity();

        // Write statistics to EEPROM once a while to increase EEPROM lifetime
        if (((sensorData.numReads % 100) == 0)) {
            EEPROM_Write(&sensorData, sizeof(sensorData));
        }

        // Print status
        printStatus(temperature, humidity);
    }
}

void EEPROM_Write(const void *buf, uint8_t bufLength)
{
    // Write buffer to EEPROM
    for (uint8_t i = 0; i < bufLength; i++) {
        // Check if Byte is changed to increase EEPROM lifetime
        if (EEPROM.read(i) != ((uint8_t *)buf)[i]) {
            Serial.print(F("W "));
            Serial.print(i);
            Serial.print(F(": "));
            Serial.println(((uint8_t *)buf)[i]);
            EEPROM.write(i, ((uint8_t *)buf)[i]);

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
            // Write cached RAM to EEPROM is only needed for ESP8266 and ESP32
            EEPROM.commit();
#endif
        }
    }
}

void EEPROM_Read(void *buf, uint8_t bufLength)
{
    // Read buffer from EEPROM
    for (uint8_t i = 0; i < bufLength; i++) {
        ((uint8_t *)buf)[i] = EEPROM.read(i);
//        Serial.print(F("R "));
//        Serial.print(i);
//        Serial.print(F(": "));
//        Serial.println(((uint8_t *)buf)[i]);
    }
}

int16_t readTemperature()
{
    int16_t temperature;

    // Read temperature from sensor (blocking)
    temperature = dht22.readTemperature();

    // Check valid temperature value
    if (temperature == ~0) {
        // Increment temperature error counter
        sensorData.tempErrors++;

        // Temperature error (Check hardware connection)
        Serial.println(F("Temperature read error"));
    } else {
        // Store minimum temperature
        if (temperature < sensorData.tempMin) {
            sensorData.tempMin = temperature;
        }

        // Store maximum temperature
        if (temperature > sensorData.tempMax) {
            sensorData.tempMax = temperature;
        }
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
        // Increment humidity error counter
        sensorData.humidityErrors++;

        // Humidity error (Check hardware connection)
        Serial.println(F("Humidity read error"));
    } else {
        // Store minimum humidity
        if (humidity < sensorData.humidityMin) {
            sensorData.humidityMin = humidity;
        }

        // Store maximum humidity
        if (humidity > sensorData.humidityMax) {
            sensorData.humidityMax = humidity;
        }
    }

    return humidity;
}

void printTemperature(int16_t temperature)
{
    // Print temperature
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

void printHumidity(int16_t humidity)
{
    // Print humidity
    Serial.print(humidity / 10);
    Serial.print(F("."));
    Serial.print(humidity % 10);
    Serial.println(F(" %"));
}

void printStatus(int16_t temperature, int16_t humidity)
{
    // Print temperature
    Serial.print(F("Temperature: "));
    printTemperature(temperature);

    // Print humidity
    Serial.print(F("Humidity: "));
    printHumidity(humidity);

    // Print number of conversions and error counters
    Serial.print(F("Num reads: "));
    printDec32(sensorData.numReads);
    Serial.print(F("Temp errors: "));
    Serial.println(sensorData.tempErrors);
    Serial.print(F("Humidity errors: "));
    Serial.println(sensorData.humidityErrors);

    // Print min/max temperature
    Serial.print(F("Temp min: "));
    printTemperature(sensorData.tempMin);
    Serial.print(F("Temp max: "));
    printTemperature(sensorData.tempMax);

    // Print min/max humidity
    Serial.print(F("Humidity min: "));
    printHumidity(sensorData.humidityMin);
    Serial.print(F("Humidity max: "));
    printHumidity(sensorData.humidityMax);

    Serial.println();
}

void printDec32(uint32_t val)
{
    char buf[9];

    // Serial.print() on Arduino AVR can only print 16-bit variables, so use snprintf_P with a small
    // character buffer instead
    snprintf_P(buf, sizeof(buf), PSTR("%lu"), val);
    Serial.println(buf);
}
#endif // !defined(ARDUINO_SAM_DUE)
