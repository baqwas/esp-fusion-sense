/**
 * @file fermionens160_test.cpp
 * @author ParkCircus Products Engineering Team
 * @version 1.0.0
 * @date 2026-07-23
 *
 * @copyright Copyright (c) 2026 ParkCircus Products. All Rights Reserved.
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
 * @brief Validates I2C connectivity and air quality data acquisition for the DFRobot ENS160 sensor module.
 *
 * The `fermionens160_test.cpp` module initializes the air quality sensor
 * connected to the FireBeetle 2 ESP32-C6[cite: 1]. It verifies I2C
 * bus communication and reads TVOC, eCO2, and AQI indices.
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-23 | Initial production release for ENS160 air quality validation.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board[cite: 1].
 * - Sensor: DFRobot Fermion ENS160 Air Quality Sensor[cite: 1].
 * - Wiring: Connect sensor SDA to ESP32 I2C SDA (GPIO 6), SCL to SCL (GPIO 7), VCC to 3.3V, and GND to GND[cite: 1].
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin[cite: 1].
 * - Library Requirement: `DFRobot_ENS160` dependency declared in platformio.ini[cite: 1].
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams initialization logs and metrics packets over UART0 at 115200 baud, 8-N-1[cite: 1].
 * - Diagnostic Metrics: Prints real-time TVOC, eCO2, and Air Quality Index (AQI)[cite: 1].
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Starts serial communication channel at 115,200 baud[cite: 1].
 *    - Initializes Wire (I2C) interface mapped to ESP32-C6 default pins[cite: 1].
 *    - Verifies ENS160 air quality sensor handshaking and sets operating mode to standard run[cite: 1].
 * 2. Execution Loop (`loop()`):
 *    - Reads air quality parameters (AQI, TVOC, eCO2) from the ENS160[cite: 1].
 *    - Outputs consolidated telemetry packet to the serial monitor[cite: 1].
 *    - Pauses execution for 5000 milliseconds before subsequent read cycle[cite: 1].
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: Serial monitor displays initialization failure warnings for ENS160[cite: 1].
 *   - Cause: I2C address conflict, loose wiring, or missing pull-up resistors on SDA/SCL lines[cite: 1].
 *   - Resolution: Check that GPIO 6 (SDA) and GPIO 7 (SCL) connections are secure and verify power input is stable[cite: 1].
 * - Symptom: Build fails due to missing library headers (`DFRobot_ENS160.h`)[cite: 1].
 *   - Cause: Required library dependencies are not declared in `lib_deps` within `platformio.ini`[cite: 1].
 *   - Resolution: Add the appropriate DFRobot library reference to your project build configuration[cite: 1].
 */

/**
 * @page doc_notes 7. References & Notes
 * @note Designed for isolated module verification within the SOHO sensor node framework[cite: 1].
 * @reference DFRobot Fermion: ENS160 Air Quality Sensor Product Wiki[cite: 1].
 */

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_ENS160.h>

// Instantiate sensor object
DFRobot_ENS160_I2C ens160(&Wire, 0x53); // Default ENS160 I2C address is usually 0x53 (or 0x52 depending on ADDR pin)

void setup() {
    // Initialize host debugging serial monitor
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n[INIT] Initializing DFRobot ENS160 Air Quality Sensor...");

    // Initialize I2C bus for ESP32-C6 (GPIO 6 = SDA, GPIO 7 = SCL)
    Wire.begin(6, 7);

    // Initialize ENS160 Air Quality Sensor
    while (ens160.begin() != NO_ERR) {
        Serial.println("[ERROR] ENS160 sensor not found! Check I2C wiring. Retrying in 2 seconds...");
        delay(2000);
    }

    // Set ENS160 to standard operating mode (IDLE = 0, STANDARD = 1)
    ens160.setPWRMode(ENS160_STANDARD_MODE);
    Serial.println("[SUCCESS] ENS160 sensor initialized and set to standard mode.");
}

void loop() {
    Serial.println("----------------------------------------");

    // Read Air Quality Data from ENS160
    uint8_t ensStatus = ens160.getENS160Result();
    if (ensStatus == NO_ERR) {
        uint8_t aqi = ens160.getAQI();
        uint16_t tvoc = ens160.getTVOC();
        uint16_t eco2 = ens160.getECO2();

        Serial.println("[ENS160] Air Quality Metrics:");
        Serial.print("  - Air Quality Index (AQI 1-5): "); Serial.println(aqi);
        Serial.print("  - TVOC Concentration:          "); Serial.print(tvoc); Serial.println(" ppb");
        Serial.print("  - eCO2 Concentration:          "); Serial.print(eco2); Serial.println(" ppm");
    } else {
        Serial.println("[WARN] ENS160 data read error or sensor warming up.");
    }

    delay(5000); // Poll every 5 seconds
}
