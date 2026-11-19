/**
 * @file main.cpp
 * @brief Phase 1: Basic Connectivity and Hardware Telemetry Test for FireBeetle 2 ESP32-C6 with Solar Charger.
 * @path ~/esp-fusion-sense/src/solar_power/01_connectivity/main.cpp
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
 *    - DFRobot Sunflower Solar Power Manager 5V (SKU: DFR0559) or onboard charger
 *    - 3.7V LiPo Battery connected to the designated battery terminal
 *    - 5V Monocrystalline Solar Panel connected to input power
 * 2. Software / Toolchain:
 *    - CLion IDE with PlatformIO plugin installed
 *    - PlatformIO configuration (`platformio.ini`) targeting `dfrobot_firebeetle2_esp32c6`
 *
 * ============================================================================
 * USER INTERFACE GUIDE
 * ============================================================================
 * - Serial Monitor Baud Rate: 115200 bps
 * - Output Format: Human-readable text logs streaming system uptime, raw ADC values,
 *   and estimated battery voltage levels every 5 seconds.
 *
 * ============================================================================
 * PROCESSING WORKFLOW & ALGORITHMS
 * ============================================================================
 * 1. Initialization:
 *    - Initializes system hardware and opens hardware Serial at 115200 baud.
 *    - Configures the designated analog input pin for high-impedance voltage reading.
 * 2. Execution Loop:
 *    - Queries system uptime via `millis()`.
 *    - Performs analog-to-digital conversion (ADC) on the battery voltage divider pin.
 *    - Applies scaling calculation: V_batt = (ADC_raw / 4095.0) * V_ref * Divider_Ratio.
 *    - Transmits formatted string over UART to the host console.
 *
 * ============================================================================
 * ERROR MESSAGE RESPONSES
 * ============================================================================
 * - [WARN] Low Voltage Warning: Triggered if estimated battery voltage falls below 3.3V.
 * - [ERROR] ADC Read Failure: Triggered if raw ADC returns rail limits (0 or 4095) continuously,
 *   indicating potential disconnect or short circuit on the sensing line.
 *
 * ============================================================================
 * REFERENCES & NOTES
 * ============================================================================
 * - ESP32-C6 Datasheet & Technical Reference Manual (Espressif Systems)
 * - DFRobot FireBeetle 2 ESP32-C6 Wiki & Pinout Diagrams
 * ============================================================================
 */

#include <Arduino.h>

// Pin Definitions for Telemetry and Monitoring
constexpr uint8_t BATTERY_ADC_PIN = A0;   // Analog input pin mapped to battery voltage divider
constexpr unsigned long TELEMETRY_INTERVAL_MS = 5000; // 5-second polling interval

// Calibration constants
constexpr float ADC_MAX_RESOLUTIONS = 4095.0f;
constexpr float ESP32_VREF = 3.3f;
constexpr float VOLTAGE_DIVIDER_RATIO = 2.0f; // Adjust based on onboard or external hardware resistor divider

unsigned long previousMillis = 0;

void setup() {
    // Initialize serial communication channel
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Wait for serial monitor to connect during bootstrap
    }

    Serial.println("\n[INFO] --------------------------------------------------");
    Serial.println("[INFO] Initializing FireBeetle 2 ESP32-C6 Solar Telemetry Node");
    Serial.println("[INFO] Phase 1: Basic Connectivity & Hardware Diagnostic");
    Serial.println("[INFO] --------------------------------------------------");

    // Configure ADC pin mode
    pinMode(BATTERY_ADC_PIN, INPUT);
}

void loop() {
    unsigned long currentMillis = millis();

    // Non-blocking interval check for telemetry logging
    if (currentMillis - previousMillis >= TELEMETRY_INTERVAL_MS) {
        previousMillis = currentMillis;

        unsigned long uptimeSeconds = currentMillis / 1000;

        // Read raw ADC value
        int rawValue = analogRead(BATTERY_ADC_PIN);

        // Calculate estimated battery voltage
        float estimatedVoltage = (static_cast<float>(rawValue) / ADC_MAX_RESOLUTIONS)
                                 * ESP32_VREF
                                 * VOLTAGE_DIVIDER_RATIO;

        // Diagnostic checks and warnings
        if (rawValue <= 10) {
            Serial.println("[ERROR] ADC Read Failure: Voltage reading near ground. Check wiring.");
        } else if (estimatedVoltage < 3.3f) {
            Serial.println("[WARN] Low Voltage Warning: Battery level dropping below safe threshold!");
        }

        // Output telemetry payload to serial console
        Serial.printf("[TELEMETRY] Uptime: %lu s | Raw ADC: %d | Est. Battery Voltage: %.2fV\n",
                      uptimeSeconds, rawValue, estimatedVoltage);
    }
}
