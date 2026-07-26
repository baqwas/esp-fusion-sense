/**
 * @file fermionC4002_test.cpp
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
 * @brief Validates physical presence and UART connectivity of the DFRobot SEN0691 C4002 mmWave sensor.
 *
 * The `sensor_presence_test.cpp` module establishes baseline handshaking between the
 * FireBeetle 2 ESP32-C6 microcontroller and the Fermion C4002 24GHz mmWave radar sensor.
 * It verifies serial communication integrity and checks whether target occupancy state data
 * can be successfully read from the module.
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-23 | Initial production release for C4002 mmWave sensor validation.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board.
 * - Sensor: DFRobot Fermion C4002 24GHz mmWave Human Presence Sensor (SEN0691).
 * - Wiring: Connect sensor TX to ESP32 RX, sensor RX to ESP32 TX, VIN to 5V/3.3V, and GND to GND.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 * - Library Requirement: `DFRobot_C4002` dependency declared in platformio.ini.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams initialization status and target detection state updates
 *   over the UART0 interface configured at 115200 baud, 8-N-1.
 * - Diagnostic Codes: Emits explicit connection validation notices and parsed target states
 *   (No Target, Static Presence, or Motion) directly to the host terminal monitor.
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Activates system serial monitoring channels at 115,200 baud.
 *    - Establishes hardware serial communication link mapped to the mmWave module.
 *    - Invokes `c4002.begin()` to test command handshaking and module response.
 *    - Retries connection sequence in a controlled loop if initial verification fails.
 * 2. Execution Loop (`loop()`):
 *    - Polls raw telemetry buffers via `c4002.getNoteInfo()`.
 *    - Parses target classification results for human presence or motion states.
 *    - Prints formatted diagnostic packets to the host serial console at regular intervals.
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: Serial monitor outputs "C4002 Sensor initialization failed!" continuously.
 *   - Cause: Incorrect wiring harness mapping (TX/RX lines crossed or power unsupplied).
 *   - Resolution: Verify that sensor TX connects to RX pin and sensor RX connects to TX pin.
 * - Symptom: Build fails due to missing `DFRobot_C4002.h` header file.
 *   - Cause: Library dependency omitted from project configuration specifications.
 *   - Resolution: Add `DFRobot/DFRobot_C4002` to the `lib_deps` array in `platformio.ini`.
 */

/**
 * @page doc_notes 7. References & Notes
 * @note Designed for isolated test execution using PlatformIO build filters.
 * @reference DFRobot Fermion C4002 mmWave Human Presence Sensor (SEN0691) Product Wiki & Library Manual.
 */

#include <Arduino.h>
#include <DFRobot_C4002.h>

// Define hardware serial mapping for ESP32-C6 (using Serial1 for peripheral communication)
// If using hardware Serial1, define custom RX/TX pins if necessary
#define MMWAVE_RX_PIN 17
#define MMWAVE_TX_PIN 16

// Instantiate the C4002 sensor object utilizing HardwareSerial
DFRobot_C4002 c4002(&Serial1);

void setup() {
    // Initialize host debugging serial monitor
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n[INIT] Establishing connection with DFRobot C4002 mmWave Sensor...");

    // Initialize the peripheral serial communication port for the radar module (default 9600 baud for C4002)
    Serial1.begin(9600, SERIAL_8N1, MMWAVE_RX_PIN, MMWAVE_TX_PIN);

    // Verify sensor presence and handshaking response
    while (!c4002.begin()) {
        Serial.println("[ERROR] C4002 Sensor not detected or wiring check failed! Retrying in 2 seconds...");
        delay(2000);
    }

    Serial.println("[SUCCESS] DFRobot C4002 mmWave Sensor successfully connected and verified!");
}

void loop() {
    // Poll the sensor data frame to check presence and target status
    sRetResult_t retResult = c4002.getNoteInfo();

    if (retResult.noteType == eResult) {
        Serial.println("----------------------------------------");
        Serial.println("[STATUS] Sensor active and communicating.");

        // Output basic presence metrics
        if (retResult.targetState == 0) {
            Serial.println("[TARGET] State: No Target Detected");
        } else if (retResult.targetState == 1) {
            Serial.println("[TARGET] State: Stationary Presence (Micro-motion / Breathing)");
        } else if (retResult.targetState == 2) {
            Serial.println("[TARGET] State: Active Motion Detected");
        }
    } else {
        Serial.println("[WARN] Waiting for valid sensor data packet...");
    }

    delay(1000);
}
