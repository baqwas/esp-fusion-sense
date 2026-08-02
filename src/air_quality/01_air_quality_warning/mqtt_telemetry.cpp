/**
 * @file mqtt_telemetry.cpp
 * @brief Project 1: Air Quality Warning - Networked Telemetry Broadcast
 *
 * @purpose Upgrades the interrupt-driven environmental node to format and broadcast
 *          Air Quality warnings to a Mosquitto MQTT broker. Implements robust Wi-Fi provisioning
 *          with retry-and-disconnect routines, Publish/Subscribe architecture, and JSON payload
 *          serialization, enabling asynchronous monitoring by external systems.
 *
 * @version 1.2.0
 * @update_history
 *   - 1.2.0 (2026-08-01): Implemented robust Wi-Fi connection retry limit and full session disconnect logic.
 *   - 1.1.0 (2026-08-01): Integrated MQTT authentication and broker configuration via secrets.h.
 *   - 1.0.0 (2026-08-01): Initial release adding Wi-Fi, PubSubClient, and JSON serialization.
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
 *   - Serial Monitor (115200 baud) logs Wi-Fi connection status, retry sequences, MQTT broker session states,
 *     and published JSON payloads.
 *   - External MQTT clients can subscribe to `telemetry/air_quality/+/warnings` to monitor the fleet.
 *
 * @error_messages_responses
 *   - "⚠️ [WIFI] Attempt failed. Retrying...": Indicates an interim connection timeout; will disconnect and re-attempt.
 *   - "⚠️ [WIFI] Connection failed. Restarting provisioning attempt...": Full session teardown executed before retrying.
 *   - "⚠️ [MQTT] Broker unreachable (rc=-2)": Check the broker IP address and ensure the Mosquitto service is running.
 *   - "⚠️ [MQTT] Broker unreachable (rc=5)": Verify `MQTT_USER` and `MQTT_PASS` credentials are correct.
 *
 * @processing_workflow_and_algorithms
 *   1. Hardware Initialization: Map I2C and hardware interrupt pins; initialize ENS160 and BME280.
 *   2. Network Provisioning: Establish a resilient 802.11 b/g/n Wi-Fi connection with bounded retries and session teardown recovery.
 *   3. MQTT Session Management: Connect and authenticate to the broker using the `DEVICE_ID` compiler build flag as the unique client identifier.
 *   4. Asynchronous Polling: Enter an idle state until the ENS160 hardware interrupt fires.
 *   5. Data Evaluation: Fetch sensor telemetry and evaluate against the AQI threshold (>= 4).
 *   6. Payload Serialization: If the threshold is crossed, serialize the data into a JSON string.
 *   7. Telemetry Broadcast: Publish the JSON payload to the broker and return to idle.
 *
 * @references_and_notes
 *   - The unique MQTT client identifier and topic structure dynamically inherit the `DEVICE_ID`
 *     predefined compiler build flag from `platformio.ini` to guarantee collision-free deployment
 *     across the cluster without relying on external configuration files.
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"
#include "secrets.h" // Contains WIFI_SSID, WIFI_PASS, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS

// Fallback for unique identifier if compiler build flag is missing
#ifndef DEVICE_ID
#define DEVICE_ID "esp32c6-unknown"
#endif

// Hardware Pin Definitions
#define I2C_SDA_PIN     19
#define I2C_SCL_PIN     20
#define ENS160_INT_PIN  10

// Topic generation buffers
char pub_topic_warning[128];

// Object Instantiations
WiFiClient espClient;
PubSubClient mqtt(espClient);
DFRobot_ENS160_I2C ENS160(&Wire, 0x53);
DFRobot_BME280_IIC BME(&Wire, 0x76);

// System State Variables
volatile bool dataReadyFlag = false;
bool isWarmedUp = false;
const uint32_t WARMUP_DURATION_MS = 180000; // 3-minute warm-up window
uint32_t bootTime = 0;

/**
 * @brief Interrupt Service Routine (ISR) for ENS160 Data Ready.
 */
void IRAM_ATTR ens160_isr() {
    dataReadyFlag = true;
}

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
        while (WiFi.status() != WL_CONNECTED && retries < 20) { // 10 seconds timeout per attempt
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
            WiFi.disconnect(true, true); // Fully disconnect and clear credentials state
            delay(1000);
        }
    }
}

/**
 * @brief Manages the MQTT connection session, authentication, and dynamically builds topics.
 */
void reconnect_mqtt() {
    while (!mqtt.connected()) {
        Serial.printf("🔄 [MQTT] Attempting connection as Client: %s...\n", DEVICE_ID);

        // Attempt to connect to Mosquitto with authentication credentials
        if (mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS)) {
            Serial.println("✅ [MQTT] Session established with broker.");
        } else {
            Serial.printf("⚠️ [MQTT] Broker unreachable (rc=%d). Retrying in 5 seconds...\n", mqtt.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌐 [INIT] Initializing Networked AQI Telemetry Node...");

    // Construct dynamic MQTT topics based on compiler flag
    snprintf(pub_topic_warning, sizeof(pub_topic_warning), "ha/telemetry/air_quality/%s/warnings", DEVICE_ID);

    // Initialize Network & Broker
    setup_wifi();
    mqtt.setServer(MQTT_HOST, MQTT_PORT);

    // Initialize I2C Bus
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);

    // Initialize Sensors
    if (BME.begin() != BME.eStatusOK) {
        Serial.println("⚠️ [ERROR] BME280 initialization failed.");
        while (1) { delay(1000); }
    }
    if (ENS160.begin() != NO_ERR) {
        Serial.println("⚠️ [ERROR] ENS160 initialization failed.");
        while (1) { delay(1000); }
    }

    ENS160.setPWRMode(ENS160_STANDARD_MODE);
    ENS160.setINTMode(ENS160.eINTModeEN | ENS160.eINTDataDrdyEN);

    pinMode(ENS160_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENS160_INT_PIN), ens160_isr, FALLING);

    bootTime = millis();
}

void loop() {
    // Ensure network persistence
    if (!mqtt.connected()) {
        reconnect_mqtt();
    }
    mqtt.loop(); // Maintain MQTT keep-alive

    // 1. Customized Countdown Window
    if (!isWarmedUp) {
        uint32_t elapsedTime = millis() - bootTime;
        if (elapsedTime < WARMUP_DURATION_MS) {
            if (elapsedTime % 10000 == 0) {
                Serial.printf("⏳ [WARM-UP] Sensor stabilizing... %lu seconds remaining.\n",
                              (WARMUP_DURATION_MS - elapsedTime) / 1000);
                delay(1);
            }
            return;
        } else {
            isWarmedUp = true;
            Serial.println("🚀 [READY] Warm-up complete. Entering networked idle state.");
        }
    }

    // 2. Asynchronous Event Processing
    if (dataReadyFlag) {
        dataReadyFlag = false;

        float temp = BME.getTemperature();
        float humidity = BME.getHumidity();
        ENS160.setTempAndHum(temp, humidity);

        uint8_t aqi = ENS160.getAQI();

        // 3. Threshold Trigger Logic
        if (aqi >= 4) {
            uint16_t tvoc = ENS160.getTVOC();
            uint16_t eco2 = ENS160.getECO2();
            float pressure = BME.getPressure() / 100.0F;

            // 4. Payload Serialization (JSON format)
            char json_payload[256];
            snprintf(json_payload, sizeof(json_payload),
                "{\"device_id\":\"%s\",\"aqi\":%u,\"eco2\":%u,\"tvoc\":%u,\"temp_c\":%.2f,\"humidity_rh\":%.2f,\"pressure_hpa\":%.2f}",
                DEVICE_ID, aqi, eco2, tvoc, temp, humidity, pressure);

            // 5. Broadcast to MQTT Broker
            Serial.println("\n🚨 [WARNING] THRESHOLD VIOLATION! Broadcasting telemetry...");
            Serial.printf("📤 Topic: %s\n", pub_topic_warning);
            Serial.printf("📦 Payload: %s\n", json_payload);

            if (mqtt.publish(pub_topic_warning, json_payload)) {
                Serial.println("✅ [SUCCESS] Payload transmitted.");
            } else {
                Serial.println("⚠️ [ERROR] Payload transmission failed.");
            }
        }
    }
}
