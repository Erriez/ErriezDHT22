/*
 * MIT License
 *
 * Copyright (c) 2020-2020 Erriez
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
 * \brief DHT22 logging example for Arduino
 * \details
 *      This example is optimized for low power usage.
 *      Write temperature and humidity every 10 minutes to SD-card.
 *      A low-power Arduino Pro Micro / Pro Mini 3.3V at 8MHz is recommended for low power.
 *      The DS3231 must wake-up the controller from sleep every minute or longer.
 *
 *      Library dependencies:
 *          - SPI.h             Build-in
 *          - SD.h              Build-in
 *          - Wire.h            Build-in
 *          - LowPower.h:       https://github.com/rocketscream/Low-Power
 *          - ErriezDHT22.h:    https://github.com/Erriez/ErriezDHT22
 *          - ErriezDS3231.h:   https://github.com/Erriez/ErriezDS3231
 */

#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_AVR)
#include <LowPower.h>
#endif

#include <ErriezDHT22.h>
#include <ErriezDS3231.h>

// Connect DTH22 DAT pin to Arduino DIGITAL pin
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_SAM_DUE)
#define DHT22_PIN      2
#elif defined(ESP8266) || defined(ESP32)
#define DHT22_PIN      4 // GPIO4 (Labeled as D2 on some ESP8266 boards)
#else
#error "May work, but not tested on this target"
#endif

// Number of temperature and humidity samples for average calculation
#define DHT22_NUM_SAMPLES         // 10

// Uno, Nano, Mini, other 328-based: pin D2 (INT0) or D3 (INT1)
// DUE: Any digital pin
// Leonardo: pin D7 (INT4)
// ESP8266 / NodeMCU / WeMos D1&R2: pin D3 (GPIO0)
#if defined(__AVR_ATmega328P__) || defined(ARDUINO_SAM_DUE)
#define RTC_INT_PIN     3
#elif defined(ARDUINO_AVR_LEONARDO)
#define RTC_INT_PIN     7
#else
#define RTC_INT_PIN     0 // GPIO0 pin for ESP8266 / ESP32 targets
#endif

// SD-card pin
#define SD_CARD_CS_PIN    10

//#define RTC_SET_DATE_TIME

// Create DHT22 sensor object
DHT22 dht22 = DHT22(DHT22_PIN);

// Create DS3231 RTC object
DS3231 rtc;

// Create date time object (automatically cleared at startup)
DS3231_DateTime dt = {
        .second = 0,
        .minute = 34,
        .hour = 21,
        .dayWeek = 4, // 1 = Monday
        .dayMonth = 15,
        .month = 11,
        .year = 2018
};

// Define days of the week in flash
const char day_1[] PROGMEM = "Monday";
const char day_2[] PROGMEM = "Tuesday";
const char day_3[] PROGMEM = "Wednesday";
const char day_4[] PROGMEM = "Thursday";
const char day_5[] PROGMEM = "Friday";
const char day_6[] PROGMEM = "Saturday";
const char day_7[] PROGMEM = "Sunday";

const char* const day_week_table[] PROGMEM = {
        day_1, day_2, day_3, day_4, day_5, day_6, day_7
};

// Define months in flash
const char month_1[] PROGMEM = "January";
const char month_2[] PROGMEM = "February";
const char month_3[] PROGMEM = "March";
const char month_4[] PROGMEM = "April";
const char month_5[] PROGMEM = "May";
const char month_6[] PROGMEM = "June";
const char month_7[] PROGMEM = "July";
const char month_8[] PROGMEM = "August";
const char month_9[] PROGMEM = "September";
const char month_10[] PROGMEM = "October";
const char month_11[] PROGMEM = "November";
const char month_12[] PROGMEM = "December";

const char* const month_table[] PROGMEM = {
        month_1, month_2, month_3, month_4, month_5, month_6,
        month_7, month_8, month_9, month_10, month_11, month_12
};

// Alarm interrupt flag must be volatile
volatile bool rtcAlarmInterrupt = false;

// Log data
typedef struct {
    uint16_t temperature;
    uint16_t humidity;
} LogData;

LogData logData;


void alarmHandler()
{
    // Set global RTC interrupt flag
    rtcAlarmInterrupt = true;
}

void readDateTime()
{
    // Read RTC date and time from RTC
    if (rtc.getDateTime(&dt)) {
        Serial.println(F("Error: Read date time failed"));
        return;
    }
}

void printDateTime()
{
    char buf[32];

    readDateTime();

    // Print day of the week, day month, month and year
    strncpy_P(buf, (char *)pgm_read_dword(&(day_week_table[dt.dayWeek - 1])), sizeof(buf));
    Serial.print(buf);
    Serial.print(F(" "));
    Serial.print(dt.dayMonth);
    Serial.print(F(" "));
    strncpy_P(buf, (char *)pgm_read_dword(&(month_table[dt.month - 1])), sizeof(buf));
    Serial.print(buf);
    Serial.print(F(" "));
    Serial.print(dt.year);
    Serial.print(F("  "));

    // Print time
    snprintf(buf, sizeof(buf), "%d:%02d:%02d", dt.hour, dt.minute, dt.second);
    Serial.println(buf);
}

void printLogData()
{
    char buf[10];

    // Read RTC date and time from RTC
    if (rtc.getDateTime(&dt)) {
        Serial.println(F("Error: Read date time failed"));
        return;
    }

    // Print log data
    snprintf(buf, sizeof(buf), "%d:%02d", dt.hour, dt.minute);
    Serial.print(buf);
    Serial.print(F(", "));
    printTemperature(logData.temperature);
    Serial.print(F(", "));
    printHumidity(logData.humidity);
}

void printTemperature(int16_t temperature)
{
    // Check valid temperature value
    if (temperature == ~0) {
        // Temperature error (Check hardware connection)
        Serial.print(F("ERR"));
    } else {
        // Print temperature
        Serial.print(temperature / 10);
        Serial.print(F("."));
        Serial.print(temperature % 10);
    }
}

void printHumidity(int16_t humidity)
{
    // Check valid humidity value
    if (humidity == ~0) {
        // Humidity error (Check hardware connection)
        Serial.print(F("ERR"));
    } else {
        // Print humidity
        Serial.print(humidity / 10);
        Serial.print(F("."));
        Serial.print(humidity % 10);
    }
}

void getCsvFilename(char *csvFilename, uint8_t csvFilenameLen)
{
    // Create CSV filename
    snprintf(csvFilename, csvFilenameLen,
             "%d%02d%02d.csv", dt.year, dt.month, dt.dayMonth);
}

void getCsvDate(char *csvDate, uint8_t csvDateLen)
{
    snprintf(csvDate, csvDateLen,
             "%d-%02d-%02d", dt.year, dt.month, dt.dayMonth);
}

void getCsvTime(char *csvTime, uint8_t csvTimeLen)
{
    snprintf(csvTime, csvTimeLen,
             "%d:%02d", dt.hour, dt.minute);
}

void saveLogDataSDCard()
{
    char csvFilename[14];
    char csvDate[11];
    char csvTime[6];
    char csvRow[40];
    bool writeHeader = false;
    File csvFile;

    // Get filename
    getCsvFilename(csvFilename, sizeof(csvFilename));

    // Get date
    getCsvDate(csvDate, sizeof(csvDate));

    // Get timestamp
    getCsvTime(csvTime, sizeof(csvTime));

    Serial.print(F("Saving "));
    Serial.print(csvFilename);
    Serial.print(F("..."));

    // Re-init SD-card (ignore return value which is broken in SD library)
    (void)SD.begin(SD_CARD_CS_PIN);

    // Check if file exists
    if (!SD.exists(csvFilename)) {
        writeHeader = true;
    }

    // Open file
    csvFile = SD.open(csvFilename, FILE_WRITE);
    if (!csvFile) {
        Serial.print(F("Error: Cannot open file"));
        return;
    }

    // Write CSV header if new file created
    if (writeHeader) {
        snprintf(csvRow, sizeof(csvRow), "%s,Temperature,Humidity\n", csvDate);
        if (csvFile.write(csvRow, strlen(csvRow)) != strlen(csvRow)) {
            Serial.print(F("Error: Write header failed"));
            return;
        }
    }

    snprintf(csvRow, sizeof(csvRow),
             "%s,%d.%d,%d.%d\n",
             csvTime,
             logData.temperature / 10, logData.temperature % 10,
             logData.humidity / 10, logData.humidity % 10);
    if (csvFile.write(csvRow, strlen(csvRow)) != strlen(csvRow)) {
        Serial.print(F("Error: Write data failed"));
        return;
    }

    // Close file
    csvFile.close();

    Serial.print(F("OK"));
}

void measure()
{
    // Read date and time from RTC
    readDateTime();

    // Read temperature and humidity
    dht22.readSensorData();
    logData.temperature = dht22.readTemperature();
    logData.humidity = dht22.readHumidity();
}

void setup()
{
    // Initialize serial port
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println(F("DHT22 logging example\n"));

    // Initialize SD-card
    while (!SD.begin(SPI_QUARTER_SPEED, SD_CARD_CS_PIN)) {
        Serial.println(F("Error: SD-card init failed"));
        delay(3000);
    }

    // Initialize TWI
    Wire.begin();
    Wire.setClock(400000);

    // Initialize RTC
    while (rtc.begin()) {
        Serial.println(F("Error: Could not detect DS3231 RTC"));
        delay(3000);
    }

#if defined(RTC_SET_DATE_TIME)
    // Set new RTC date/time
    rtc.setDateTime(&dt);
#endif

    // Check oscillator status
    if (rtc.isOscillatorStopped()) {
        Serial.println(F("Error: DS3231 RTC oscillator stopped. Program new date/time."));
        while (1) {
            ;
        }
    }

    // Attach to interrupt pin falling edge
    pinMode(RTC_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), alarmHandler, FALLING);

    // Program alarm 1
    rtc.setAlarm1(Alarm1MatchSeconds, 0, 0, 0, 0);
    rtc.alarmInterruptEnable(Alarm1, true);
    rtc.alarmInterruptEnable(Alarm2, false);

    // Print current date and time
    Serial.print(F("RTC date/time: "));
    printDateTime();

    // Initialize DHT22 sensor
    dht22.begin(DHT22_NUM_SAMPLES);
}

void loop()
{
    // Print date and time on every nINT/SQW pin falling edge
    if (rtcAlarmInterrupt) {
        if (rtc.getAlarmFlag(Alarm1)) {
            // Measure temperature and humidity
            measure();
            // Print temperature and humidity
            printLogData();
            Serial.print(F("  "));

            // Save to SD-card every 10 minutes
            if ((dt.minute % 10) == 0) {
                saveLogDataSDCard();
            }

            Serial.println();
        }

        // Clear alarm interrupts
        rtc.clearAlarmFlag(Alarm1);
        rtc.clearAlarmFlag(Alarm2);

        // Clear alarm interrupt when alarms are handled
        rtcAlarmInterrupt = false;
    }

    // Flush serial output
    Serial.flush();

#if defined(ARDUINO_ARCH_AVR)
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
#else
    delay(100);
#endif
}
