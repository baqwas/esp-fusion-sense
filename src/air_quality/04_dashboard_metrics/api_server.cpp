/**
 * @file api_server.cpp
 * @brief Module 4: Dashboard Metrics - Exercise 4.1: Local Web Server & REST API
 *
 * @purpose Implements an embedded HTTP server and REST API endpoint on the ESP32-C6
 *          to expose real-time environmental telemetry (temperature, humidity, TVOC, eCO2, AQI)
 *          as structured JSON payloads for local web dashboards and client polling.
 *
 * @version 1.0.1
 * @update_history
 *   - 1.0.1 (2026-08-01): Updated to leverage credentials from secrets.h.[cite: 3]
 *   - 1.0.0 (2026-08-01): Initial release introducing WebServer endpoint routing for JSON telemetry.[cite: 3]
 *
 * @author Matha Goram[cite: 3]
 * @copyright Copyright (c) 2026 ParkCircus Productions. All Rights Reserved.[cite: 3]
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
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, Fermion ENS160, DFRobot BME280.[cite: 3]
 *   - Network: Active Wi-Fi AP configuration credentials via secrets.h.
 *
 * @user_interface_guide
 *   - Access `http://<node-ip>/api/telemetry` via a local web browser or curl to retrieve JSON sensor metrics.[cite: 3]
 *
 * @error_messages_responses
 *   - "⚠️ [ERROR] WiFi connection failed": Verify SSID and password configuration.[cite: 3]
 *   - "⚠️ [ERROR] Sensor initialization failed": Check I2C wiring and power rails.[cite: 3]
 *
 * @processing_workflow_and_algorithms
 *   1. Connect to local Wi-Fi network and initialize sensor buses (I2C).[cite: 3]
 *   2. Start HTTP server on port 80 and register route handler for `/api/telemetry`.[cite: 3]
 *   3. Continuously sample environmental parameters from BME280 and ENS160.[cite: 3]
 *   4. Serve formatted JSON data dynamically upon client HTTP GET requests.[cite: 3]
 *
 * @references_and_notes
 *   - Standard lightweight JSON formatting is used to avoid external heavy serialization library overhead on constrained microcontrollers.[cite: 3]
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"
#include "secrets.h"

// Hardware Pin Definitions
#define I2C_SDA_PIN     19
#define I2C_SCL_PIN     20

// Sensor Objects
DFRobot_ENS160_I2C ENS160(&Wire, 0x53);
DFRobot_BME280_IIC BME(&Wire, 0x76);

// HTTP Server Instance on Port 80
WebServer server(80);

void handleTelemetry() {
    float temp = BME.getTemperature();
    float humidity = BME.getHumidity();
    ENS160.setTempAndHum(temp, humidity);

    uint8_t aqi = ENS160.getAQI();
    uint16_t tvoc = ENS160.getTVOC();
    uint16_t eco2 = ENS160.getECO2();

    // Construct JSON payload
    String json = "{";
    json += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
    json += "\"temperature_c\":" + String(temp, 2) + ",";
    json += "\"humidity_pct\":" + String(humidity, 2) + ",";
    json += "\"tvoc_ppb\":" + String(tvoc) + ",";
    json += "\"eco2_ppm\":" + String(eco2) + ",";
    json += "\"aqi\":" + String(aqi);
    json += "}";

    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌐 [INIT] Initializing Dashboard Metrics - Exercise 4.1: Local REST API...");

    // Initialize I2C and Sensors
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (BME.begin() != BME.eStatusOK) Serial.println("⚠️ [ERROR] BME280 failed.");
    if (ENS160.begin() != NO_ERR) Serial.println("⚠️ [ERROR] ENS160 failed.");
    ENS160.setPWRMode(ENS160_STANDARD_MODE);

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("📡 [WIFI] Connecting to network");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n📡 [WIFI] Connected! IP Address: %s\n", WiFi.localIP().toString().c_str());

    // Setup HTTP Endpoints
    server.on("/api/telemetry", HTTP_GET, handleTelemetry);
    server.begin();
    Serial.println("🚀 [HTTP] REST API Server started on port 80.");
}

void loop() {
    server.handleClient();
}
