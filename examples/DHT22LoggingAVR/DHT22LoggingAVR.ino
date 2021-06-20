/*
 * MIT License
 *
 * Copyright (c) 2018-2021 Erriez
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
 * \brief DHT22 low-power SD-card logging example for Arduino ATMega328 AVR targets (UNO/Nano/ProMini)
 * \details
 *      This example is optimized for low power usage.
 *      Write temperature and humidity every 10 minutes to SD-card.
 *      A low-power Arduino Pro Micro / Pro Mini 3.3V at 8MHz is recommended for low power.
 *      The DS3231 must wake-up the controller from sleep every minute.
 */

#if !defined(ARDUINO_ARCH_AVR)
#error "Example only supported on AVR targets"
#endif

#include <time.h>             // Built-in
#include <avr/power.h>        // Built-in
#include <SPI.h>              // Built-in
#include <SD.h>               // Built-in
#include <Wire.h>             // Built-in

#include <LowPower.h>         // https://github.com/rocketscream/Low-Power
#include <ErriezDHT22.h>      // https://github.com/Erriez/ErriezDHT22
#include <ErriezDS3231.h>     // https://github.com/Erriez/ErriezDS3231

// Enable to set new DS3231 RTC build date/time
//#define DS3231_SET_DATE_TIME

// Sample interval in minutes
#define LOG_INTERVAL_MIN      10

// Number of retries after DHT22 read error
#define DHT22_READ_RETRIES    3

// CSV files
#define FILE_VCC_CSV          "vcc.csv"
#define FILE_ERROR_CSV        "err.csv"

// Error LED (NOT PIN 13 which is shared with SPI CLK!)
#define LED_PIN               A0

// Connect DTH22 DATA pin to Arduino DIGITAL pin
#define DHT22_PIN             2

// Uno, Nano, Mini, other ATMega328 based: pin D2 (INT0) or D3 (INT1)
#define DS3231_INT_PIN        3

// SD-card pin
#define SD_CARD_CS_PIN        10
#define SD_CARD_POWER_PIN     4

// Num flashes error LED
#define LED_FLASH_OK            1
#define LED_FLASH_LOW_BATT      2
#define LED_FLASH_DHT22_ERROR   3
#define LED_FLASH_ERROR         5

// ADC calibration data (device specific)
#define VCC_3V3               3350

// Serial baudrate
#define SERIAL_BAUDRATE       9600

// Select serial port such as Serial, Serial0 or comment out to disable
#define SERIAL_PORT           Serial

#ifdef SERIAL_PORT
#define serialBegin()         { SERIAL_PORT.begin(SERIAL_BAUDRATE); }
#define serialPrint(x)        { SERIAL_PORT.print(x); }
#define serialPrintln(x)      { SERIAL_PORT.println(x); }
#define serialFlush()         { SERIAL_PORT.flush(); }
#else
#define serialBegin(x)        {}
#define serialPrint(x)        {}
#define serialPrintln(x)      {}
#define serialFlush()         {}
#endif

// Create DHT22 sensor object
DHT22 dht22 = DHT22(DHT22_PIN);

// Create DS3231 RTC object
ErriezDS3231 ds3231;

// Alarm interrupt flag must be volatile
static volatile bool ds3231AlarmInterrupt = false;

// SPCR register to enable/disable SD-card power
static byte keep_SPCR;

// Log data
static struct tm dt;
static int16_t temperature;
static int16_t humidity;
static uint16_t vcc;


void alarmHandler()
{
    // Set global DS3231 interrupt flag
    ds3231AlarmInterrupt = true;
}

void deepSleep(enum period_t sleepTime)
{
    // Flush serial
    serialFlush();

    // Enter low-power deepsleep
    LowPower.powerDown(sleepTime, ADC_OFF, BOD_ON);
}

void ledFlash(uint8_t flashCount)
{
    pinMode(LED_PIN, OUTPUT);

    serialFlush();
    for (uint8_t i = 0; i < flashCount; i++) {
        digitalWrite(LED_PIN, LOW);
        LowPower.powerDown(SLEEP_30MS, ADC_OFF, BOD_ON);
        digitalWrite(LED_PIN, HIGH);
        LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_ON);
    }
    
    pinMode(LED_PIN, INPUT);
}

void error(uint8_t flashCount, const __FlashStringHelper *msg, enum period_t sleepTime)
{
    // Print error message
    serialPrintln(msg);

    // Flash LED
    ledFlash(flashCount);

    // Deepsleep
    deepSleep(sleepTime);
}

#if defined(DS3231_SET_DATE_TIME)
bool getBuildDateTime(struct tm *dt)
{
    const char *monthName[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    int hour;
    int minute;
    int second;
    uint8_t monthIndex;
    char month[12];
    int dayMonth;
    int year;
    time_t t;

    // Convert build macro to time
    if (sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second) != 3) {
        return false;
    }

    // Convert build macro to date
    if (sscanf(__DATE__, "%s %d %d", month, &dayMonth, &year) != 3) {
        return false;
    }

    // Convert month string to month number
    for (monthIndex = 0; monthIndex < 12; monthIndex++) {
        if (strcmp(month, monthName[monthIndex]) == 0) {
            break;
        }
    }
    if (monthIndex >= 12) {
        // Not found
        return false;
    }

    // Set date/time
    dt->tm_hour = hour;
    dt->tm_min = minute;
    dt->tm_sec = second;
    dt->tm_mday = dayMonth;
    dt->tm_mon = monthIndex;
    dt->tm_year = year - 1900;

    // Calculate day of the week
    t = mktime(dt);
    dt->tm_wday = localtime(&t)->tm_wday;

    return true;
}

bool ds3231SetDateTime()
{
    struct tm dt;

    // Convert compile date/time to date/time string
    if (!getBuildDateTime(&dt)) {
        serialPrint(F("Error: Get build date/time failed"));
        return false;
    }

    // Print build date/time
    serialPrint(F("Build date time: "));
    serialPrintln(asctime(&dt));

    // Set new date time
    serialPrint(F("Set RTC date time..."));
    if (!ds3231.write(&dt)) {
        serialPrintln(F("FAILED"));
    } else {
        serialPrintln(F("OK"));
    }

    return true;
}
#endif

bool ds3231ReadDateTime()
{
    // Read date and time from DS3231 RTC
    return ds3231.read(&dt);
}

void dht22Read()
{  
    // Read temperature and humidity
    dht22.readSensorData();
    temperature = dht22.readTemperature();
    humidity = dht22.readHumidity();
}

bool dht22IsReadSuccess()
{
    if ((temperature == ~0) || (humidity == ~0)) {
        return false;
    }
    return true;
}

void readVcc() 
{
    uint16_t result;
    uint8_t low;
    uint8_t high;
    
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the internal 1.1V reference
    #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
      ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
      ADMUX = _BV(MUX5) | _BV(MUX0);
    #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
      ADMUX = _BV(MUX3) | _BV(MUX2);
    #else
      ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    #endif  
  
    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA, ADSC)) {
        ;
    }
  
    low  = ADCL; // must read ADCL first - it then locks ADCH  
    high = ADCH; // unlocks both
  
    result = (high << 8) | low;

    // Calculate Vcc (in mV); 1125300 = 1.1 * 1023* 1000
    result = 1125300L / result;

    // Calibrate
    vcc = map(result, 0, 3300, 0, VCC_3V3);
}

bool isLowBatt()
{
    // Check battery voltage
    if (vcc < 2980) {
        return true;
    }
    return false;
}

void turnOnSDcard()
{
    // https://thecavepearlproject.org/2017/05/21/switching-off-sd-cards-for-low-power-data-logging

    // Turn SD-card power on via 1K resistor to basis NPN transistor
    // Collector connected to GND SD-card.
    pinMode(SD_CARD_POWER_PIN, OUTPUT);
    digitalWrite(SD_CARD_POWER_PIN, HIGH);
    delay(6);

    // Some cards will fail on power-up unless SS is pulled up  ( &  D0/MISO as well? )
    DDRB |= (1<<DDB5) | (1<<DDB3) | (1<<DDB2); // set SCLK(D13), MOSI(D11) & SS(D10) as OUTPUT
    // Note: | is an OR operation so  the other pins stay as they were.                (MISO stays as INPUT)
    PORTB &= ~(1<<DDB5);  // disable pin 13 SCLK pull-up – leave pull-up in place on the other 3 lines

    // Enable the SPI clock
    power_spi_enable();

    // Enable SPI peripheral
    SPCR = keep_SPCR;

    delay(10);
}

void turnOffSDcard()
{
    delay(6);

    // Disable SPI
    SPCR = 0;

    // Disable SPI clock
    power_spi_disable();

    // Set all SPI pins to INPUT
    DDRB &= ~((1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2));
    // Set all SPI pins HIGH
    PORTB |= ((1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2));

    // Note: LED_BUILTIN on pin 13 must be removed, otherwise it bleeds current

    // Wait 1 second for internal SD-card handling
    deepSleep(SLEEP_1S);

    // Turn SD-card power off via basis NPN transistor
    pinMode(SD_CARD_POWER_PIN, OUTPUT);
    digitalWrite(SD_CARD_POWER_PIN, LOW);
}

void csvWrite(const char *filename, const char *lines)
{
    File f;

    // Print write
    serialPrint(filename);
    serialPrint(F(": "));
    serialPrint(lines);

    // Open file
    f = SD.open(filename, FILE_WRITE);
    if (!f) {
        serialPrintln(F("Error: Cannot open file"));
        return;
    }

    // Write lines
    if (f.write(lines, strlen(lines)) != strlen(lines)) {
        serialPrintln(F("Error: File write failed"));
    }

    // Close file
    f.close();
}

void writeLogToSD(bool writeVcc)
{
    char csvFilename[14];
    char dateStr[11];
    char timeStr[6];
    char row[40];

    // Create date/time strings
    snprintf_P(dateStr, sizeof(dateStr), PSTR("%d-%02d-%02d"), 1900 + dt.tm_year, dt.tm_mon + 1, dt.tm_mday);
    snprintf_P(timeStr, sizeof(timeStr), PSTR("%d:%02d"), dt.tm_hour, dt.tm_min);

    // SD-card power-on
    turnOnSDcard();

    // Re-init SD-card (ignore return value which is broken in SD library)
    (void)SD.begin(SD_CARD_CS_PIN);

    if (!dht22IsReadSuccess()) {
        // Write to error CSV file
        snprintf_P(row, sizeof(row), PSTR("%s %s\n"), dateStr, timeStr);
        csvWrite(FILE_ERROR_CSV, row);
    } else {
        // Write to CSV file
        snprintf(csvFilename, sizeof(csvFilename), "%d%02d.csv", 1900 + dt.tm_year, dt.tm_mon + 1);
        if (!SD.exists(csvFilename)) {
            csvWrite(csvFilename, "Timestamp,Temperature,Humidity\n");
        }
        snprintf_P(row, sizeof(row), PSTR("%s %s,%d.%d,%d.%d\n"),
            dateStr, timeStr,
            temperature / 10, temperature % 10, humidity / 10, humidity % 10);
        csvWrite(csvFilename, row);
    }

    if (writeVcc) {
        // Write VCC to CSV file
        snprintf(row, sizeof(row), "%s %s,%d.%d\n", dateStr, timeStr, vcc / 1000, vcc % 1000);
        csvWrite(FILE_VCC_CSV, row);
    }

    // SD-card power-off
    turnOffSDcard();
}

void printLog()
{
    char dateStr[11];
    char timeStr[9];
    char line[40];

    // Create date/time strings
    snprintf_P(dateStr, sizeof(dateStr), PSTR("%d-%02d-%02d"), 1900 + dt.tm_year, dt.tm_mon + 1, dt.tm_mday);
    snprintf_P(timeStr, sizeof(timeStr), PSTR("%d:%02d:%02d"), dt.tm_hour, dt.tm_min, dt.tm_sec);

    // Print logged data
    snprintf_P(line, sizeof(line), PSTR("%s %s, "), dateStr, timeStr);
    serialPrint(line);
    serialPrint(F("T: "));
    if (!dht22IsReadSuccess()) {
        serialPrint(F("ERR"));
    } else {
        // Print temperature
        serialPrint(temperature / 10);
        serialPrint(F("."));
        serialPrint(temperature % 10);
    }
    serialPrint(F(", H: "));
    if (!dht22IsReadSuccess()) {
        serialPrint(F("ERR"));
    } else {
        // Print humidity
        serialPrint(humidity / 10);
        serialPrint(F("."));
        serialPrint(humidity % 10);
    }
    serialPrint(F(", VCC: "));
    serialPrint(vcc);
    serialPrint(F("mV, Interval: "));
    serialPrint(LOG_INTERVAL_MIN);
    serialPrintln(F(" min"));
}

bool measure(bool bReadVcc)
{
    bool retval = true;

    // Read temperature/humidity from DHT22
    for (uint8_t i = 0; i < DHT22_READ_RETRIES; i++) {
        dht22Read();

        if (!dht22IsReadSuccess()) {
            error(LED_FLASH_DHT22_ERROR, F("DHT22 error"), SLEEP_4S);
            retval = false;
        } else {
            break;
        }
    }

    // Read date/time from RTC
    if (!ds3231ReadDateTime()) {
        error(LED_FLASH_ERROR, F("DS3231 error"), SLEEP_250MS);
        retval = false;
    }

    // Read VCC
    if (bReadVcc) {
        readVcc();
    }

    // Check low battery
    if (isLowBatt()) {
        error(LED_FLASH_LOW_BATT, F("LOW BATT!"), SLEEP_8S);
        retval = false;
    }

    return retval;
}

void serialInit()
{
    // Initialize serial port
    serialBegin();
    serialPrintln(F("\nDHT22 low-power SD Card logging"));
}

void twiInit()
{
    // Initialize TWI
    Wire.begin();
    Wire.setClock(400000);
}

void sdInit()
{
    // Turn SD-card power on
    turnOnSDcard();

    // Initialize SD-card
    while (!SD.begin(SPI_QUARTER_SPEED, SD_CARD_CS_PIN)) {
        error(LED_FLASH_ERROR, F("Error: SD-card init failed"), SLEEP_4S);
    }
    
    // Create error CSV if not exist
    if (!SD.exists(FILE_ERROR_CSV)) {
        csvWrite(FILE_ERROR_CSV, "Timestamp\n");
    }
    
    // Create VCC CSV if not exist
    if (!SD.exists(FILE_VCC_CSV)) {
        csvWrite(FILE_VCC_CSV, "Timestamp,VCC\n");
    }

    // Turn SD-card power off
    turnOffSDcard();
}

void ds3231Init()
{
    // Initialize DS3231 RTC
    while (!ds3231.begin()) {
        error(LED_FLASH_ERROR, F("Error: Could not detect DS3231 RTC"), SLEEP_4S);
    }

    // Set date/time RTC
#if defined(DS3231_SET_DATE_TIME)
    if (!ds3231SetDateTime()) {
        while (1)
            ;
    }
#endif

    // Check RTC oscillator status
    while (!ds3231.isRunning()) {
        error(LED_FLASH_ERROR, F("Error: DS3231 RTC oscillator stopped. Program new date/time."), SLEEP_4S);
    }

    // Disable DS3231 square wave out pin
    ds3231.outputClockPinEnable(false);
    ds3231.setSquareWave(SquareWaveDisable);

    // Program alarm 1 every minute and alarm 2 disabled
    ds3231.setAlarm1(Alarm1MatchSeconds, 0, 0, 0, 0);
    ds3231.alarmInterruptEnable(Alarm1, true);
    ds3231.alarmInterruptEnable(Alarm2, false);
}

void ds3231Start()
{
    // Clear RTC alarm interrupts
    ds3231.clearAlarmFlag(Alarm1);
    ds3231.clearAlarmFlag(Alarm2);
    ds3231AlarmInterrupt = false;

    // Attach to interrupt pin falling edge
    pinMode(DS3231_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INT_PIN), alarmHandler, FALLING);

    // Read date/time
    (void)ds3231ReadDateTime();
}

void dht22Init()
{
    // Initialize DHT22
    dht22.begin();
}

void setup()
{
    bool measureSuccess;
    
    // Begin initialization
    ledFlash(LED_FLASH_OK);

    // Initialize serial
    serialInit();

    // Initialize SD-card
    sdInit();
    
    // Initialize TWI
    twiInit();

    // Initialize DS3231 RTC
    ds3231Init();

    // Initialize DHT22 sensor
    dht22Init();

    do {
        // Start measurement
        measureSuccess = measure(/*bReadVcc=*/true);

        // Print measurement
        printLog();
    } while (!measureSuccess);

    // Reset DS3231 RTC
    ds3231Start();

    // End initialization
    ledFlash(LED_FLASH_OK);
}

void loop()
{
    // Check wakeup from DS3231 nINT/SQW interrupt
    if (ds3231AlarmInterrupt) {
        // Check logging interval
        if (((dt.tm_min % LOG_INTERVAL_MIN) == 0) && (dt.tm_sec == 0)) {
            bool logVcc = dt.tm_min == 0;

            // Start measurement
            (void)measure(logVcc);

            // Write logs to SD-card
            writeLogToSD(logVcc);
        }

        // Increment time for next logging interval (ignore overflow)
        dt.tm_min++;
        dt.tm_sec = 0;

        // Clear alarm 1 interrupt
        ds3231.clearAlarmFlag(Alarm1);

        // Clear alarm interrupt when alarms are handled
        ds3231AlarmInterrupt = false;
    }

    // Flush serial output
    deepSleep(SLEEP_FOREVER);
}