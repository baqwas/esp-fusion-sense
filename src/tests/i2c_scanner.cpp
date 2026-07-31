/**
 * @file i2c_scanner.cpp
 * @author Matha Goram
 * @version 1.0.0
 * @date 2026-07-30
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
 * @brief Scans the I2C bus for connected peripheral hardware addresses on the ESP32-C6.
 *
 * The `i2c_scanner.cpp` utility systematically pings all valid 7-bit I2C addresses
 * (0x01 to 0x7F) over the designated SDA and SCL pins, reporting any active
 * responding nodes to the debugging host via serial output.
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-30 | Initial standalone implementation for hardware bus discovery.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board.
 * - Peripherals: Target I2C sensors or expansion modules under test.
 * - Wiring: Connect module SDA to GPIO 6 and SCL to GPIO 7.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams scan results over UART0 at 115200 baud, 8-N-1.
 * - Diagnostic Metrics: Hexadecimal address reporting for any acknowledged devices.
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Opens serial telemetry at 115,200 baud.
 *    - Initializes the `Wire` library explicitly assigning GPIO 6 (SDA) and GPIO 7 (SCL).
 *    - Sets standard I2C clock frequency to 100kHz.
 * 2. Execution Loop (`loop()`):
 *    - Iterates through addresses 1 through 126.
 *    - Transmits empty payload packets using `Wire.beginTransmission()` and `Wire.endTransmission()`.
 *    - Evaluates error codes to flag active nodes and prints a summary block.
 *    - Delays for 5000 milliseconds prior to subsequent scanning passes.
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: Serial monitor displays "No I2C devices found".
 *   - Cause: Incomplete wiring, lack of 3.3V power supply to the module, or swapped SDA/SCL lines.
 *   - Resolution: Verify physical breadboard wiring against hardware specifications and confirm power rails.
/**
 * @file i2c_scanner.cpp
 * @author ParkCircus Products Engineering Team
 * @version 1.0.1
 * @date 2026-07-31
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
 * @brief Scans the I2C bus for connected peripheral hardware addresses on the ESP32-C6.
 *
 * The `i2c_scanner.cpp` utility systematically pings all valid 7-bit I2C addresses
 * (0x01 to 0x7F) over the designated SDA and SCL pins, reporting any active
 * responding nodes to the debugging host via serial output.
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.1 | 2026-07-31 | Corrected I2C pin mapping from JTAG pins (6/7) to correct hardware pins (19/20).
 * @version 1.0.0 | 2026-07-30 | Initial standalone implementation for hardware bus discovery.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board.
 * - Peripherals: Target I2C sensors or expansion modules under test.
 * - Wiring: Connect module SDA to GPIO 19 and SCL to GPIO 20. Do NOT use GPIO 6/7 (JTAG).
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams scan results over UART0 at 115200 baud, 8-N-1.
 * - Diagnostic Metrics: Hexadecimal address reporting for any acknowledged devices.
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Opens serial telemetry at 115,200 baud.
 *    - Initializes the `Wire` library explicitly assigning GPIO 19 (SDA) and GPIO 20 (SCL).
 *    - Sets standard I2C clock frequency to 100kHz.
 * 2. Execution Loop (`loop()`):
 *    - Iterates through addresses 1 through 126.
 *    - Transmits empty payload packets using `Wire.beginTransmission()` and `Wire.endTransmission()`.
 *    - Evaluates error codes to flag active nodes and prints a summary block.
 *    - Delays for 5000 milliseconds prior to subsequent scanning passes.
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: Serial monitor displays "No I2C devices found".
 *   - Cause: Incomplete wiring, lack of 3.3V power supply to the module, or swapped SDA/SCL lines.
 *   - Resolution: Verify physical breadboard wiring against hardware specifications and confirm power rails. Ensure GPIO 19 and 20 are used.
 */

/**
 * @page doc_notes 7. References & Notes
 * @note Implements standard Arduino Wire-based bus probing logic adapted for the ESP32-C6.
 */

#include <Arduino.h>
#include <Wire.h>

void setup() {
    // Initialize host debugging serial monitor
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n[INIT] Starting I2C Bus Scanner utility for ESP32-C6...");

    // Initialize Wire on explicit pins for FireBeetle 2 ESP32-C6 (GPIO 19 = SDA, GPIO 20 = SCL)
    Wire.begin(19, 20);
    Wire.setClock(100000); // Set standard 100kHz I2C clock
    delay(200);
}

void loop() {
    byte error, address;
    int nDevices = 0;

    Serial.println("\nScanning I2C addresses (0x01 - 0x7E)...");

    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("  [FOUND] I2C device detected at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println(" (!)");
            nDevices++;
        } else if (error == 4) {
            Serial.print("  [ERROR] Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }

    if (nDevices == 0) {
        Serial.println("No I2C devices found on the bus.");
    } else {
        Serial.print("Scan complete. Total devices found: ");
        Serial.println(nDevices);
    }

    Serial.println("----------------------------------------");
    delay(5000); // Wait 5 seconds before next scan cycle
}
