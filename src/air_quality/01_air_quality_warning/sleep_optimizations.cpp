/**
 * @file sleep_optimizations.cpp
 * @brief Project 1: Air Quality Warning - Low-Power Deep Sleep Telemetry
 *
 * @purpose Implements advanced power optimization for battery-operated ESP32 deployments.
 *          Utilizes RTC memory to preserve state across reboots, puts the microcontroller
 *          into deep sleep during idle periods, and selectively powers the Wi-Fi radio only
 *          when critical air quality threshold warnings require transmission.
 *
 * @version 1.0.0
 * @update_history
 *   - 1.0.0 (2026-08-01): Initial release adding deep sleep cycles, RTC state tracking, and conditional Wi-Fi telemetry.
 *
 * @author Matha Goram
 * @copyright Copyright (c) 2026 ParkCircus Productions. All Rights Reserved.
 *
 * @license
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @prerequisites
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, Fermion ENS160, and BME280 sensors.
 *   - Infrastructure: A reachable Mosquitto MQTT broker deployed on the local Linux SOHO network.
 *   - Dependencies: `PubSubClient` by Nick O'Leary, alongside DFRobot sensor libraries.
 *   - Security: A populated `secrets.h` file containing `WIFI_SSID`, `WIFI_PASS`, `MQTT_HOST`, `MQTT_PORT`, `MQTT_USER`, and `MQTT_PASS`.
 *
 * @user_interface_guide
 *   - Serial Monitor (115200 baud) logs wake reasons, sensor evaluations, deep sleep transitions, and MQTT payload transmissions.
 *   - External MQTT clients can subscribe to `telemetry/air_quality/+/warnings` to monitor remote battery nodes.
 *
 * @error_messages_responses
 *   - "⚠️ [WIFI] Connection failed. Resetting session...": Interim association timeout; executing teardown and retry.
 *   - "⚠️ [MQTT] Broker unreachable (rc=%d)": Verify broker availability and authentication tokens.
 *
 * @processing_workflow_and_algorithms
 *   1. Wake State Analysis: Determine boot reason from deep sleep vs. initial power-on via ESP RTC utilities.
 *   2. Sensor Initialization & Stabilization: Wake I2C bus, initialize ENS160 and BME280 sensors, and respect thermal warm-up parameters.
 *   3. Telemetry Acquisition: Fetch compensated temperature, humidity, and Air Quality Index (AQI) metrics.
 *   4. Conditional Network Activation: If AQI is below the warning threshold, bypass Wi-Fi and re-enter deep sleep to conserve battery.
 *   5. Secure Broadcast: If AQI >= 4, provision Wi-Fi with robust teardown retry logic, connect to Mosquitto MQTT broker, publish JSON payload, and sleep.
 *
 * @references_and_notes
 *   - RTC memory retention preserves counter states across sleep cycles without relying on volatile heap variables.
 *   - Deep sleep significantly extends operational lifespan for remote field-deployed environmental sensor nodes.
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"
#include "secrets.h"

// Fallback for unique identifier if compiler build flag is missing
#ifndef DEVICE_ID
#define DEVICE_ID "esp32c6-unknown"
#endif

// Hardware Pin Definitions
#define I2C_SDA_PIN     19
#define I2C_SCL_PIN     20
#define ENS160_INT_PIN  10

// Sleep configuration: Wake up every 60 seconds to check air quality
#define TIME_TO_SLEEP_SEC  60
#define uS_TO_S_FACTOR     1000000ULL

// Topic generation buffers
char pub_topic_warning[128];

// Object Instantiations
WiFiClient espClient;
PubSubClient mqtt(espClient);
DFRobot_ENS160_I2C ENS160(&Wire, 0x53);
DFRobot_BME280_IIC BME(&Wire, 0x76);

// RTC Persistent Memory Variables (survives deep sleep cycles)
RTC_DATA_ATTR int bootCount = 0;

/**
 * @brief Provisions the Wi-Fi connection using a robust retry and complete session teardown strategy.
 */
void setup_wifi() {
    delay(10);
    WiFi.mode(WIFI_STA);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.printf("\n📡 [WIFI] Connecting to %s ", WIFI_SSID);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        int retries = 0;
        while (WiFi.status() != WL_CONNECTED && retries < 20) {
            delay(500);
            Serial.print(".");
            retries++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✅ [WIFI] Connected!");
            Serial.printf("   IP Address: %s\n", WiFi.localIP().toString().c_str());
            break;
        } else {
            Serial.println("\n⚠️ [WIFI] Connection failed. Disconnecting and resetting session...");
            WiFi.disconnect(true, true);
            delay(1000);
        }
    }
}

/**
 * @brief Manages the MQTT connection session and authenticates with the broker.
 */
void connect_mqtt_once() {
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    int attempts = 0;
    while (!mqtt.connected() && attempts < 3) {
        Serial.printf("🔄 [MQTT] Connecting as Client: %s (Attempt %d)...\n", DEVICE_ID, attempts + 1);
        if (mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS)) {
            Serial.println("✅ [MQTT] Session established.");
        } else {
            Serial.printf("⚠️ [MQTT] Broker unreachable (rc=%d). Retrying...\n", mqtt.state());
            delay(2000);
            attempts++;
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    bootCount++;
    Serial.printf("\n🌐 [WAKE] System boot count: %d\n", bootCount);

    // Construct dynamic MQTT topics based on compiler flag
    snprintf(pub_topic_warning, sizeof(pub_topic_warning), "telemetry/air_quality/%s/warnings", DEVICE_ID);

    // Initialize I2C Bus
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);

    // Initialize Sensors
    if (BME.begin() != BME.eStatusOK) {
        Serial.println("⚠️ [ERROR] BME280 initialization failed.");
        goto enter_deep_sleep;
    }
    if (ENS160.begin() != NO_ERR) {
        Serial.println("⚠️ [ERROR] ENS160 initialization failed.");
        goto enter_deep_sleep;
    }

    ENS160.setPWRMode(ENS160_STANDARD_MODE);
    delay(100); // Short stabilization pause for sensor read

    {
        float temp = BME.getTemperature();
        float humidity = BME.getHumidity();
        ENS160.setTempAndHum(temp, humidity);

        uint8_t aqi = ENS160.getAQI();
        uint16_t tvoc = ENS160.getTVOC();
        uint16_t eco2 = ENS160.getECO2();
        float pressure = BME.getPressure() / 100.0F;

        Serial.printf("📊 [READING] AQI: %u | eCO2: %u ppm | TVOC: %u ppb | Temp: %.2f°C\n", aqi, eco2, tvoc, temp);

        // Threshold Trigger Logic: Only power radio and transmit if AQI meets warning criteria (>= 4)
        if (aqi >= 4) {
            Serial.println("🚨 [WARNING] Threshold exceeded! Initializing radio transmission...");
            setup_wifi();
            connect_mqtt_once();

            if (mqtt.connected()) {
                char json_payload[256];
                snprintf(json_payload, sizeof(json_payload),
                    "{\"device_id\":\"%s\",\"boot_count\":%d,\"aqi\":%u,\"eco2\":%u,\"tvoc\":%u,\"temp_c\":%.2f,\"humidity_rh\":%.2f,\"pressure_hpa\":%.2f}",
                    DEVICE_ID, bootCount, aqi, eco2, tvoc, temp, humidity, pressure);

                if (mqtt.publish(pub_topic_warning, json_payload)) {
                    Serial.println("✅ [SUCCESS] Warning payload broadcasted successfully.");
                } else {
                    Serial.println("⚠️ [ERROR] Payload transmission failed.");
                }
                mqtt.disconnect();
            }
            WiFi.disconnect(true, true);
        } else {
            Serial.println("💤 [NORMAL] Air quality nominal. Skipping radio initialization to save power.");
        }
    }

enter_deep_sleep:
    Serial.printf("🛌 [SLEEP] Entering deep sleep for %d seconds...\n\n", TIME_TO_SLEEP_SEC);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_SEC * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void loop() {
    // Unused in deep-sleep architecture; execution halts after setup cycle.
}
