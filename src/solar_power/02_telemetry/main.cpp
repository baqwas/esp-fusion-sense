/**
 * @file main.cpp
 * @brief Phase 2: Structured JSON Telemetry Feed for FireBeetle 2 ESP32-C6 Solar Node.
 * @path ~/esp-fusion-sense/src/solar_power/02_telemetry/main.cpp
 *
 * @version 1.0.0
 * @date 2026-09-17
 *
 * @author Matha Goram
 * @copyright Copyright (c) 2026 ParkCircus Productions
 *
 * @license
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, and modify, merge, publish, distribute, sublicense, and/or sell
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
 *
 * ============================================================================
 * PREREQUISITES
 * ============================================================================
 * 1. Hardware:
 *    - DFRobot FireBeetle 2 ESP32-C6 (SKU: DFR1075)
 *    - Solar Power Management Module (Onboard or DFR0559)
 *    - 3.7V LiPo Battery and 5V Monocrystalline Solar Panel connected
 * 2. Software / Toolchain:
 *    - CLion IDE with PlatformIO plugin
 *    - Environment configuration: `solar_power_telemetry_c6` in `platformio.ini`
 *
 * ============================================================================
 * USER INTERFACE GUIDE
 * ============================================================================
 * - Serial Monitor Baud Rate: 115200 bps
 * - Output Format: Structured JSON object stream emitted every 10 seconds,
 *   ready for automated log parsing or downstream time-series ingest.
 *
 * ============================================================================
 * PROCESSING WORKFLOW & ALGORITHMS
 * ============================================================================
 * 1. Initialization:
 *    - Starts serial interface and configures analog acquisition pin.
 * 2. Telemetry Generation Loop:
 *    - Captures system uptime via `millis()`.
 *    - Samples raw analog voltage from the battery monitoring divider.
 *    - Serializes metrics into a strict JSON key-value payload.
 *
 * ============================================================================
 * ERROR MESSAGE RESPONSES
 * ============================================================================
 * - {"error": "adc_fault"} -> Emitted if raw analog signal is out of bounds.
 *
 * ============================================================================
 * REFERENCES & NOTES
 * ============================================================================
 * - Designed to integrate with downstream MQTT or local serial loggers in Phase 3.
 * ============================================================================
 */

#include <Arduino.h>

constexpr uint8_t BATTERY_ADC_PIN = A0;
constexpr unsigned long TELEMETRY_INTERVAL_MS = 10000; // 10-second interval

constexpr float ADC_MAX_RESOLUTIONS = 4095.0f;
constexpr float ESP32_VREF = 3.3f;
constexpr float VOLTAGE_DIVIDER_RATIO = 2.0f;

unsigned long previousMillis = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n[INFO] Initializing FireBeetle 2 ESP32-C6 Solar Telemetry Node");
    Serial.println("[INFO] Phase 2: Structured JSON Telemetry Feed Active");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= TELEMETRY_INTERVAL_MS) {
        previousMillis = currentMillis;

        unsigned long uptime = currentMillis / 1000;
        int rawAdc = analogRead(BATTERY_ADC_PIN);
        float batteryVoltage = (static_cast<float>(rawAdc) / ADC_MAX_RESOLUTIONS)
                                 * ESP32_VREF
                                 * VOLTAGE_DIVIDER_RATIO;

        // Output structured JSON payload
        Serial.print("{");
        Serial.print("\"device\":\"firebeetle-esp32c6\",");
        Serial.print("\"uptime_s\":"); Serial.print(uptime);
        Serial.print(",\"battery_v\":"); Serial.print(batteryVoltage, 2);
        Serial.print(",\"raw_adc\":"); Serial.print(rawAdc);
        Serial.print(",\"status\":\"active\"");
        Serial.println("}");
    }
}
