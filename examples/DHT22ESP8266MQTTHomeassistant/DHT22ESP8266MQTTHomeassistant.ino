/*
 * MIT License
 *
 * Copyright (c) 2020 Erriez
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

#include <ArduinoJson.h> 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ErriezDHT22.h>

/*
Config temperature:
homeassistant/sensor/dht22_01_t/config
{
    "device_class": "temperature", 
    "name": "DHT22 Temperature", 
    "unique_id": "dht22_t_01"
    "state_topic": "homeassistant/sensor/dht22_01/state",
    "unit_of_measurement": "°C",
    "value_template": "{{ value_json.temperature}}" 
}

Config humidity:
homeassistant/sensor/dht22_01_h/config
{
    "device_class": "humidity",
    "name": "DHT22 Humidity",
    "unique_id": "dht22_h_01",
    "state_topic": "homeassistant/sensor/dht22_01/state",
    "unit_of_measurement": "%", 
    "value_template": "{{ value_json.humidity}}"
}

Publish state:
homeassistant/sensor/dht22_01/state
{ "temperature": "23.2", "humidity": "43.7" }

*/

// Wifi: SSID and password
#define WIFI_SSID               "ssid"
#define WIFI_PASSWORD           "password"

// Enable SSL
// #define USE_SSL

// MQTT: ID, server IP, port, username and password
#define MQTT_CLIENT_ID          "home_dht22_1"
#define MQTT_SERVER_ADDR        "mqtt.server.nl"
#if defined(USE_SSL)
#define MQTT_SERVER_PORT        8883 // Mosquitto SSL port
#else
#define MQTT_SERVER_PORT        1883 // Mosquitto default port
#endif
#define MQTT_USER               "mqtt_user"
#define MQTT_PASSWORD           "mqtt_password"
#define MQTT_PUBLISH_DELAY_SEC  60

// MQTT topics for Home Assistant (discovery turned on)
#define MQTT_HA_DISCOVERY       "homeassistant"
#define MQTT_TOPIC_CONFIG_T     MQTT_HA_DISCOVERY"/sensor/dht22_01_t/config"
#define MQTT_TOPIC_CONFIG_H     MQTT_HA_DISCOVERY"/sensor/dht22_01_h/config"
#define MQTT_TOPIC_STATUS       MQTT_HA_DISCOVERY"/sensor/dht22_01/state"

// MQTT Payload config
#define CONFIG_DEVICE_NAME_T    "DHT22 Temperature"
#define CONFIG_DEVICE_NAME_H    "DHT22 Humidity"
#define CONFIG_UNIQUE_ID_T      "dht22_t_01"
#define CONFIG_UNIQUE_ID_H      "dht22_h_01"

// Pin defines
#define DHT22_PIN               4

#if defined(USE_SSL)

// First letsencrypt fullchain.pem
// For example in mosquitto.conf:
//    certfile /etc/letsencrypt/live/mqtt.server.nl/fullchain.pem
const char caCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
... < Paste certificate here >
-----END CERTIFICATE-----)EOF";

// X.509 parsed CA Cert
X509List caCertX509(caCert);

// MQTT broker cert SHA1 fingerprint, used to validate connection to right server
// $ sudo openssl x509 -noout -fingerprint -sha1 -in <path>/fullchain.pem
const uint8_t mqttCertFingerprint[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

WiFiClientSecure wifiClient;

#else

// No SSL
WiFiClient wifiClient;

#endif

PubSubClient client(wifiClient);
DHT22 dht22 = DHT22(DHT22_PIN);


void mqttPublish(const char *topic, const char *payload, boolean retained=false)
{
    // Debug print
    Serial.print ("INFO: PUB: ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(payload);

    // MQTT publish
    client.publish(topic, payload, retained);
}

void mqttPublishConfig()
{
    StaticJsonDocument<200> doc;
    String payload;

    // Register device
    doc["device_class"] = "temperature";
    doc["name"] = CONFIG_DEVICE_NAME_T;
    doc["unique_id"] = CONFIG_UNIQUE_ID_T;
    doc["state_topic"] = MQTT_TOPIC_STATUS;
    doc["unit_of_measurement"] = "°C";
    doc["value_template"] = "{{ value_json.temperature}}";
    serializeJson(doc, payload);

    // Publish Home Assistant Sensor Config DHT22 temperature
    mqttPublish(MQTT_TOPIC_CONFIG_T, payload.c_str());

    payload = "";
    doc["device_class"] = "humidity";
    doc["name"] = CONFIG_DEVICE_NAME_H;
    doc["unique_id"] = CONFIG_UNIQUE_ID_H;
    doc["state_topic"] = MQTT_TOPIC_STATUS;
    doc["unit_of_measurement"] = "%";
    doc["value_template"] = "{{ value_json.humidity}}";
    serializeJson(doc, payload);

    // Publish Home Assistant Sensor Config DHT22 humidity
    mqttPublish(MQTT_TOPIC_CONFIG_H, payload.c_str());
}

void mqttPublishState(int16_t temperature, int16_t humidity)
{
    StaticJsonDocument<200> doc;
    char temperatureStr[10];
    char humidityStr[10];
    String payload;

    // Convert temperature and humidity to char array
    snprintf(temperatureStr, sizeof(temperatureStr), "%.01f", temperature / 10.0);
    snprintf(humidityStr, sizeof(humidityStr), "%.01f", humidity / 10.0);

    // Create JSON dictionary
    doc["temperature"] = temperatureStr;
    doc["humidity"] = humidityStr;
    serializeJson(doc, payload);

    // Publish DHT22 status
    mqttPublish(MQTT_TOPIC_STATUS, payload.c_str(), true);
}

void dht22Measure()
{
    int16_t temperature;
    int16_t humidity;

    // Check minimum interval of 2000 ms between sensor reads
    if (dht22.available()) {
        // Read temperature from sensor (blocking)
        temperature = dht22.readTemperature();

        // Read humidity from sensor (blocking)
        humidity = dht22.readHumidity();

        // Publish temperature and humidity
        mqttPublishState(temperature, humidity);
    }
}

bool isFirstPowerOn()
{
    rst_info *info;

    info = ESP.getResetInfoPtr();
    if (info->reason == REASON_DEFAULT_RST) {
        // Reset cause: Power-on
        return true;
    } else {
        // Reset cause: Not power-on
        return false;
    }
}

void mqttReconnect()
{
    while (!client.connected()) {
        Serial.print("INFO: Connecting to MQTT broker ");
        Serial.print(MQTT_SERVER_ADDR);
        Serial.print(":");
        Serial.print(MQTT_SERVER_PORT);
        Serial.print("...");
        
        if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("Connected");

            if (isFirstPowerOn()) {
                // Publish Home Assistant config on first power-up
                mqttPublishConfig();
            }
        } else {
            Serial.print("ERROR: Failed, rc=");
            Serial.println(client.state());
            Serial.println("DEBUG: Retry in 5 seconds...");
            
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() 
{ 
    // Init serial
    Serial.begin(115200);
  
    // Init LED
    pinMode(LED_BUILTIN, OUTPUT);
    // LED on
    digitalWrite(LED_BUILTIN, LOW);
  
    // Init WiFi connection
    Serial.println();
    Serial.println();
    Serial.print("INFO: Connecting to WiFi ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
  
    Serial.print("\nINFO: IP address: ");
    Serial.println(WiFi.localIP());

#if defined(USE_SSL)
    // Set certificate
    wifiClient.setTrustAnchors(&caCertX509);         // Load CA cert into trust store
    //wifiClient.allowSelfSignedCerts();             // Enable self-signed cert support
    wifiClient.setFingerprint(mqttCertFingerprint);

    /* Optionally do none of the above and allow insecure connections.                                             
     * This will accept any certificates from the server, without validation and is not recommended.
     */
    //wifiClient.setInsecure();
#endif

    // Init the MQTT connection
    client.setServer(MQTT_SERVER_ADDR, MQTT_SERVER_PORT);

    // Initialize sensor
    dht22.begin();

    // Connect to MQTT broker
    mqttReconnect();

    // Measure DHT22 temperature and humidity
    dht22Measure();

    // Disconnect
    client.disconnect();

    // Sleep 30 seconds (Make sure RST and D0 are connected)
    Serial.flush();
    ESP.deepSleep(MQTT_PUBLISH_DELAY_SEC * 1000000UL);
}

void loop() 
{
  
}
