/**
 * @file i2c_advanced_diag.cpp
 * @author Matha Goram
 * @version 1.1.1
 * @date 2026-07-31
 *
 * @copyright Copyright (c) 2026 ParkCircus Productions. All Rights Reserved.
 *
 * @license MIT License
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ============================================================================
 * DOCUMENTATION INDEX
 * ============================================================================
 * 1. Purpose & System Scope
 * 2. Update History
 * 3. Prerequisites & Hardware Requirements
 * 4. User Interface & Telemetry Guide
 * 5. Processing Workflow & Algorithms
 * 6. Error Message Responses & Troubleshooting
 * 7. References & Notes
 * ============================================================================
 */

/**
 * @page doc_overview 1. Purpose & System Scope
 * @brief Advanced I2C diagnostic utility for ESP32-C6 featuring bus recovery,
 *        multi-frequency scanning, and hardware verification for specific sensors (e.g., BME280).
 */

/**
 * @page doc_history 2. Update History
 * @version 1.1.1 | 2026-07-31 | Updated docstrings to match professional project standards.
 * @version 1.1.0 | 2026-07-31 | Initial advanced implementation with clock-pulsing recovery logic.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board.
 * - Peripherals: Target I2C sensors (BME280, ENS160, etc.).
 * - Wiring: Connect module SDA to GPIO 19 and SCL to GPIO 20. INT pins should float unless explicitly handled.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams multi-frequency scan results and register verifications over UART0 at 115200 baud, 8-N-1.
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Opens serial telemetry at 115,200 baud.
 *    - Executes a pre-initialization bus recovery algorithm (`recoverI2CBus`) to clear stuck SDA lines.
 *    - Initializes the `Wire` library explicitly on pins 19 (SDA) and 20 (SCL).
 * 2. Execution Phase (`loop()`):
 *    - Iterates through multiple frequency tiers (10kHz, 50kHz, 100kHz, 400kHz).
 *    - Pings addresses 0x01 through 0x7E at each frequency.
 *    - If address 0x76 or 0x77 is discovered, attempts an explicit query of register 0xD0 to verify the BME280 chip ID (0x60).
 *    - Halts execution for 10 seconds between cycles.
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: "[FATAL] SDA line is stuck LOW."
 *   - Cause: SDA wire is physically shorted to Ground, or a sensor has locked the bus mid-transmission.
 *   - Resolution: Check breadboard routing for shorts. Reset power to the external sensor modules.
 */

/**
 * @page doc_notes 7. References & Notes
 * @note Utilizing multiple frequency checks is crucial for diagnosing parasitic breadboard capacitance.
 */

#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 19
#define SCL_PIN 20

// BME280 Specifics
#define BME280_ADDR_1 0x76
#define BME280_ADDR_2 0x77
#define BME280_CHIP_ID_REG 0xD0
#define BME280_EXPECTED_ID 0x60

/**
 * Manually clocks the SCL line to free a stuck I2C bus.
 * Returns 0 if bus is clear, 1 if SDA remains stuck low.
 */
int recoverI2CBus(uint8_t sda, uint8_t scl) {
    pinMode(sda, INPUT_PULLUP);
    pinMode(scl, INPUT_PULLUP);
    delay(50);

    // If both lines are high, the bus is already clear
    if (digitalRead(sda) == HIGH && digitalRead(scl) == HIGH) {
        return 0;
    }

    Serial.println("  [WARN] Bus appears stuck. Attempting 9-clock recovery...");
    pinMode(scl, OUTPUT);

    for (int i = 0; i < 9; i++) {
        digitalWrite(scl, LOW);
        delayMicroseconds(10);
        digitalWrite(scl, HIGH);
        delayMicroseconds(10);

        pinMode(sda, INPUT_PULLUP);
        if (digitalRead(sda) == HIGH) {
            Serial.println("  [SUCCESS] Bus recovered during clock pulsing.");
            return 0;
        }
    }

    return (digitalRead(sda) == LOW) ? 1 : 0;
}

void scanBusAtFrequency(uint32_t frequency) {
    Wire.setClock(frequency);
    byte error, address;
    int nDevices = 0;

    Serial.printf("\nScanning at %lu Hz...\n", frequency);

    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf("  [FOUND] Device at 0x%02X\n", address);
            nDevices++;

            // If it's a known BME280 address, probe its Chip ID
            if (address == BME280_ADDR_1 || address == BME280_ADDR_2) {
                Wire.beginTransmission(address);
                Wire.write(BME280_CHIP_ID_REG);
                Wire.endTransmission(false);
                Wire.requestFrom((uint8_t)address, (uint8_t)1);

                if (Wire.available()) {
                    byte chipID = Wire.read();
                    Serial.printf("    -> Queried Register 0xD0. Returned ID: 0x%02X ", chipID);
                    if (chipID == BME280_EXPECTED_ID) {
                        Serial.println("(Matches BME280!)");
                    } else {
                        Serial.println("(Unknown Device)");
                    }
                }
            }
        }
    }

    if (nDevices == 0) {
        Serial.println("  No devices found.");
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Give terminal time to connect

    Serial.println("\n========================================");
    Serial.println("[INIT] Advanced I2C Diagnostics Suite");
    Serial.println("========================================");

    Serial.println("\nPhase 1: Hardware Bus Recovery Check");
    if (recoverI2CBus(SDA_PIN, SCL_PIN) != 0) {
        Serial.println("  [FATAL] SDA line is stuck LOW. Check wiring for shorts to Ground.");
    } else {
        Serial.println("  [OK] I2C lines are idle and ready.");
    }

    // Initialize Wire normally after recovery check
    Wire.begin(SDA_PIN, SCL_PIN);
}

void loop() {
    Serial.println("\nPhase 2: Multi-Frequency Sweep");

    // Sweep through standard frequencies to test for breadboard capacitance issues
    scanBusAtFrequency(10000);  // 10 kHz (Ultra low, highly resilient to capacitance)
    scanBusAtFrequency(50000);  // 50 kHz
    scanBusAtFrequency(100000); // 100 kHz (Standard Arduino default)
    scanBusAtFrequency(400000); // 400 kHz (Fast mode)

    Serial.println("\nDiagnostic cycle complete. Waiting 10 seconds...");
    Serial.println("========================================");
    delay(10000);
}
