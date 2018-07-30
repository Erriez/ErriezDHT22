/*
 * MIT License
 *
 * Copyright (c) 2018 Erriez
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
 * \file DHT22.h
 * \brief DHT22 (AM2303) Humidity and Temperature sensor library for Arduino
 * \details
 *      Source: https://github.com/Erriez/ErriezDHT22
 */

#ifndef DHT22_H__
#define DHT22_H__

#include <Arduino.h>

//! Enable debug prints to Serial
// #define DEBUG_PRINT

//! Minimum interval between sensor reads in milli seconds
#define DHT22_MIN_READ_INTERVAL     2000

//! Number of data bits is 5 Bytes * 8 bits:
//!   1 Byte: Humidity high
//!   1 Byte: Humidity low
//!   1 Byte: Temperature high
//!   1 Byte: Temperature low
//!   1 Byte: Parity
#define DHT22_NUM_DATA_BITS         (5 * 8)

//! Debug print configuration
#ifdef DEBUG_PRINT
  #define DEBUG_PRINTLN(...) { Serial.println(__VA_ARGS__); }
#else
  #define DEBUG_PRINTLN(...) {}
#endif

/*!
 * \brief DHT22 sensor class
 * \details
 *      According to the datasheet, the AM2302 is a low cost consumer temperature sensor. It may not
 *      be used in safety critical applications, emergency stop devices or any other occasion that
 *      failure of AM2303 may cause personal injury.
 *
 *      The AM2303 is factory calibrated. However, deviation may occur with aging.
 *
 *      The temperature/humidity read interval in this library is cached for 2 seconds to prevent
 *      heating-up the internal chip with continues reading.
 *
 *      Global interrupts are disabled during a synchronous sensor read transfer. This is required
 *      to sample the data bit lengths at maximum speed on low-end devices without any application
 *      interrupts. The read calls are protected with a timeout.
 *
 *      The application is responsible for checking ~0 values after a read which means that the
 *      read failed or a timeout occurred. Multiple reads by the application with an average
 *      calculation is recommended.
 *
 */
class DHT22
{
public:
    explicit DHT22(uint8_t pin);
    void begin();
    bool available();
    int16_t readTemperature();
    int16_t readHumidity();

private:
    //! Timestamp of the last completed measurement
    unsigned long _lastMeasurementTimestamp;
    //! Number of clock cycles per micro second for accurate pin measurement
    uint32_t _maxCycles;
    //! Buffer to store pin sample timing
    //! To prevent stack overflows at run-time, allocate this 320 Bytes buffer
    //! here instead of the function
    uint32_t cycles[DHT22_NUM_DATA_BITS * 2];
    //! 5 raw sensor data bytes
    //! Humidity high, humidity low, temperature high, temperature low, parity
    uint8_t _data[5];
    //! Last conversion status (Successful or not)
    bool _statusLastMeasurement;

    //! Sensor data pin
    uint8_t _pin;

#ifdef __AVR
    //! Bit number in IO pin register
    uint8_t _bit;
    //! Port in IO pin register
    uint8_t _port;
#endif

    bool readSensorData();
    bool generateStart();
    bool readBytes();
    uint32_t measurePulseWidth(uint8_t level);
};

#endif // DHT22_H__
