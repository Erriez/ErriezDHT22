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
 * \brief DHT22 - AM2302/AM2303 temperature and relative humidity sensor calibration for Arduino
 * \details
 *      Source:         https://github.com/Erriez/ErriezDHT22
 *      Documentation:  https://erriez.github.io/ErriezDHT22
 *
 *      This calculation is made with an analog calibrated hair humidity device. Points from
 *      100% to 20% are drawn on a piece of paper and compared with an ideal line from 0% to 100%.
 *      The compensation is a fixed offset and corner calculation: 100%=>100%, 0%=>11.78%
 */

// Corner tan(41.42)
#define DHT22_COMPENSATION_CORNER   0.88223916111525597236547154433081

// Fixed offset
#define DHT22_COMPENSATION_OFFSET   11.78


void printFloat(float humidity)
{
    char buf[16];

    dtostrf(humidity, 5, 1, buf);
    Serial.print(buf);
}

void humidityCompensationFloat()
{
    float humidityIn;
    float humidityOut;

    Serial.println(F("\nHumidity float compensation"));
    Serial.println(F("  IN  | OUT"));
    Serial.println(F("------+------"));

    for (humidityIn = 0.0; humidityIn <= 100.0; humidityIn += 1.0) {
        printFloat(humidityIn);
        Serial.print(F(" |"));

        // Calculate compensation
        humidityOut = DHT22_COMPENSATION_OFFSET + (DHT22_COMPENSATION_CORNER * humidityIn);

        printFloat(humidityOut);
        Serial.println();
    }
}

void humidityCompensationInt()
{
    int16_t humidityIn;
    int16_t humidityOut;
    char buf[16];

    Serial.println(F("\nHumidity int compensation"));
    Serial.println(F("  IN  | OUT"));
    Serial.println(F("------+------"));

    for (humidityIn = 0; humidityIn <= 1000; humidityIn += 10) {
        printFloat((float)humidityIn / 10);
        Serial.print(F(" | "));

        // Calculate compensation
        humidityOut = ((DHT22_COMPENSATION_OFFSET * 1000) +
                       (DHT22_COMPENSATION_CORNER * 10000 * (uint32_t)humidityIn) / 100) / 100;

        printFloat((float)humidityOut / 10);
        Serial.println();
    }
}

void setup()
{
    // Initialize serial
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    Serial.println(F("\nHumidity compensation test\n"));

    humidityCompensationFloat();
    humidityCompensationInt();
}

void loop()
{

}

/* Example output:

    Humidity compensation test
    
    
    Humidity float compensation
      IN  | OUT
    ------+------
      0.0 | 11.8
      1.0 | 12.7
      2.0 | 13.5
      3.0 | 14.4
      4.0 | 15.3
      5.0 | 16.2
      6.0 | 17.1
      7.0 | 18.0
      8.0 | 18.8
      9.0 | 19.7
     10.0 | 20.6
     11.0 | 21.5
     12.0 | 22.4
     13.0 | 23.2
     14.0 | 24.1
     15.0 | 25.0
     16.0 | 25.9
     17.0 | 26.8
     18.0 | 27.7
     19.0 | 28.5
     20.0 | 29.4
     21.0 | 30.3
     22.0 | 31.2
     23.0 | 32.1
     24.0 | 33.0
     25.0 | 33.8
     26.0 | 34.7
     27.0 | 35.6
     28.0 | 36.5
     29.0 | 37.4
     30.0 | 38.2
     31.0 | 39.1
     32.0 | 40.0
     33.0 | 40.9
     34.0 | 41.8
     35.0 | 42.7
     36.0 | 43.5
     37.0 | 44.4
     38.0 | 45.3
     39.0 | 46.2
     40.0 | 47.1
     41.0 | 48.0
     42.0 | 48.8
     43.0 | 49.7
     44.0 | 50.6
     45.0 | 51.5
     46.0 | 52.4
     47.0 | 53.2
     48.0 | 54.1
     49.0 | 55.0
     50.0 | 55.9
     51.0 | 56.8
     52.0 | 57.7
     53.0 | 58.5
     54.0 | 59.4
     55.0 | 60.3
     56.0 | 61.2
     57.0 | 62.1
     58.0 | 62.9
     59.0 | 63.8
     60.0 | 64.7
     61.0 | 65.6
     62.0 | 66.5
     63.0 | 67.4
     64.0 | 68.2
     65.0 | 69.1
     66.0 | 70.0
     67.0 | 70.9
     68.0 | 71.8
     69.0 | 72.7
     70.0 | 73.5
     71.0 | 74.4
     72.0 | 75.3
     73.0 | 76.2
     74.0 | 77.1
     75.0 | 77.9
     76.0 | 78.8
     77.0 | 79.7
     78.0 | 80.6
     79.0 | 81.5
     80.0 | 82.4
     81.0 | 83.2
     82.0 | 84.1
     83.0 | 85.0
     84.0 | 85.9
     85.0 | 86.8
     86.0 | 87.7
     87.0 | 88.5
     88.0 | 89.4
     89.0 | 90.3
     90.0 | 91.2
     91.0 | 92.1
     92.0 | 92.9
     93.0 | 93.8
     94.0 | 94.7
     95.0 | 95.6
     96.0 | 96.5
     97.0 | 97.4
     98.0 | 98.2
     99.0 | 99.1
    100.0 |100.0
    
    Humidity int compensation
      IN  | OUT
    ------+------
      0.0 |  11.7
      1.0 |  12.6
      2.0 |  13.5
      3.0 |  14.4
      4.0 |  15.3
      5.0 |  16.1
      6.0 |  17.0
      7.0 |  17.9
      8.0 |  18.8
      9.0 |  19.7
     10.0 |  20.6
     11.0 |  21.4
     12.0 |  22.3
     13.0 |  23.2
     14.0 |  24.1
     15.0 |  25.0
     16.0 |  25.8
     17.0 |  26.7
     18.0 |  27.6
     19.0 |  28.5
     20.0 |  29.4
     21.0 |  30.3
     22.0 |  31.1
     23.0 |  32.0
     24.0 |  32.9
     25.0 |  33.8
     26.0 |  34.7
     27.0 |  35.5
     28.0 |  36.4
     29.0 |  37.3
     30.0 |  38.2
     31.0 |  39.1
     32.0 |  40.0
     33.0 |  40.8
     34.0 |  41.7
     35.0 |  42.6
     36.0 |  43.5
     37.0 |  44.4
     38.0 |  45.3
     39.0 |  46.1
     40.0 |  47.0
     41.0 |  47.9
     42.0 |  48.8
     43.0 |  49.7
     44.0 |  50.5
     45.0 |  51.4
     46.0 |  52.3
     47.0 |  53.2
     48.0 |  54.1
     49.0 |  55.0
     50.0 |  55.8
     51.0 |  56.7
     52.0 |  57.6
     53.0 |  58.5
     54.0 |  59.4
     55.0 |  60.3
     56.0 |  61.1
     57.0 |  62.0
     58.0 |  62.9
     59.0 |  63.8
     60.0 |  64.7
     61.0 |  65.5
     62.0 |  66.4
     63.0 |  67.3
     64.0 |  68.2
     65.0 |  69.1
     66.0 |  70.0
     67.0 |  70.8
     68.0 |  71.7
     69.0 |  72.6
     70.0 |  73.5
     71.0 |  74.4
     72.0 |  75.2
     73.0 |  76.1
     74.0 |  77.0
     75.0 |  77.9
     76.0 |  78.8
     77.0 |  79.7
     78.0 |  80.5
     79.0 |  81.4
     80.0 |  82.3
     81.0 |  83.2
     82.0 |  84.1
     83.0 |  85.0
     84.0 |  85.8
     85.0 |  86.7
     86.0 |  87.6
     87.0 |  88.5
     88.0 |  89.4
     89.0 |  90.2
     90.0 |  91.1
     91.0 |  92.0
     92.0 |  92.9
     93.0 |  93.8
     94.0 |  94.7
     95.0 |  95.5
     96.0 |  96.4
     97.0 |  97.3
     98.0 |  98.2
     99.0 |  99.1
    100.0 | 100.0
*/
