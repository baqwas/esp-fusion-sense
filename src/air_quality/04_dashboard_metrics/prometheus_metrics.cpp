/**
 * @file prometheus_metrics.cpp
 * @brief Module 4: Dashboard Metrics - Exercise 4.2: Prometheus Metrics Exporter
 *
 * @purpose Exposes environmental telemetry in standard Prometheus exposition text format
 *          via an embedded HTTP endpoint, allowing automated SOHO cluster scraping and monitoring.
 *
 * @version 1.0.1
 * @update_history
 *   - 1.0.1 (2026-08-01): Updated to leverage credentials from secrets.h.
 *   - 1.0.0 (2026-08-01): Initial release introducing Prometheus text-format telemetry exporter.
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
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, Fermion ENS160, DFRobot BME280.
 *   - Network: Active Wi-Fi AP configuration credentials via secrets.h.
 *
 * @user_interface_guide
 *   - Access `http://<node-ip>/metrics` via a Prometheus server or web browser to inspect scraped metrics.
 *
 * @error_messages_responses
 *   - "⚠️ [ERROR] WiFi connection failed": Verify network configuration parameters.
 *
 * @processing_workflow_and_algorithms
 *   1. Connect to local Wi-Fi and initialize sensor interfaces.
 *   2. Start HTTP server and expose the `/metrics` endpoint.
 *   3. Format live metrics using standard Prometheus type definitions and gauge naming conventions.
 *   4. Return plain-text exposition metrics upon external polling scrapers.
 *
 * @references_and_notes
 *   - Follows standard Prometheus exposition format specification for direct metric ingestion.
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

void handlePrometheusMetrics() {
    float temp = BME.getTemperature();
    float humidity = BME.getHumidity();
    ENS160.setTempAndHum(temp, humidity);

    uint8_t aqi = ENS160.getAQI();
    uint16_t tvoc = ENS160.getTVOC();
    uint16_t eco2 = ENS160.getECO2();

    String metrics = "";

    // Help and Type headers
    metrics += "# HELP node_temperature_celsius Ambient temperature in Celsius\n";
    metrics += "# TYPE node_temperature_celsius gauge\n";
    metrics += "node_temperature_celsius{node=\"" + String(DEVICE_ID) + "\"} " + String(temp, 2) + "\n";

    metrics += "# HELP node_humidity_percent Relative humidity percentage\n";
    metrics += "# TYPE node_humidity_percent gauge\n";
    metrics += "node_humidity_percent{node=\"" + String(DEVICE_ID) + "\"} " + String(humidity, 2) + "\n";

    metrics += "# HELP node_tvoc_ppb Total volatile organic compounds in ppb\n";
    metrics += "# TYPE node_tvoc_ppb gauge\n";
    metrics += "node_tvoc_ppb{node=\"" + String(DEVICE_ID) + "\"} " + String(tvoc) + "\n";

    metrics += "# HELP node_eco2_ppm Equivalent CO2 in ppm\n";
    metrics += "# TYPE node_eco2_ppm gauge\n";
    metrics += "node_eco2_ppm{node=\"" + String(DEVICE_ID) + "\"} " + String(eco2) + "\n";

    metrics += "# HELP node_air_quality_index Air Quality Index rating (1-5)\n";
    metrics += "# TYPE node_air_quality_index gauge\n";
    metrics += "node_air_quality_index{node=\"" + String(DEVICE_ID) + "\"} " + String(aqi) + "\n";

    server.send(200, "text/plain; version=0.0.4", metrics);
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌐 [INIT] Initializing Dashboard Metrics - Exercise 4.2: Prometheus Exporter...");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (BME.begin() != BME.eStatusOK) Serial.println("⚠️ [ERROR] BME280 failed.");
    if (ENS160.begin() != NO_ERR) Serial.println("⚠️ [ERROR] ENS160 failed.");
    ENS160.setPWRMode(ENS160_STANDARD_MODE);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("📡 [WIFI] Connecting to network");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n📡 [WIFI] Connected! IP Address: %s\n", WiFi.localIP().toString().c_str());

    server.on("/metrics", HTTP_GET, handlePrometheusMetrics);
    server.begin();
    Serial.println("🚀 [HTTP] Prometheus Metrics Exporter started on port 80.");
}

void loop() {
    server.handleClient();
}
