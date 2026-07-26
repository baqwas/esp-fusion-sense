/**
 * @file sen0691_test.cpp
 * @author Reza
 * @version 1.1.0
 * @date 2026-07-24
 *
 * @copyright Copyright (c) 2026 Reza. All rights reserved.
 *
 * @license MIT License
 *
 * Permission is hereby granted, free of charge, to obtain a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * =============================================================================
 * PURPOSE
 * =============================================================================
 * Provides a standalone test and parameterization target for the DFRobot Fermion
 * C4002 mmWave Human Presence Sensor (SEN0691) integrated with the DFRobot FireBeetle 2
 * ESP32-C6 platform. It handles device initialization, distance threshold configuration,
 * environment noise calibration, and asynchronous target state polling over UART.
 *
 * =============================================================================
 * UPDATE HISTORY
 * =============================================================================
 * - 1.0.0 (2026-05-12): Initial release for baseline hardware integration.
 * - 1.1.0 (2026-07-24): Refactored for PlatformIO isolated source filters, integrated
 *                        compile-time WIFI_HOSTNAME_DEFAULT build flags, and added
 *                        robust error handling loops.
 *
 * =============================================================================
 * PREREQUISITES
 * =============================================================================
 * - Hardware:
 *   1. DFRobot FireBeetle 2 ESP32-C6 development board.
 *   2. DFRobot Fermion C4002 mmWave Human Presence Sensor (SEN0691).
 *   3. Half-size (400-point) breadboard and wiring harness.
 * - Software & Toolchain:
 *   1. PlatformIO Core / CLion with PlatformIO Plugin (Linux/Debian environment).
 *   2. Arduino framework for Espressif 32.
 *   3. DFRobot_C4002 library dependency.
 *
 * =============================================================================
 * USER INTERFACE GUIDE (SERIAL MONITOR)
 * =============================================================================
 * - Baud Rate: 115200 bps
 * - Data Format: 8N1
 * - Output Metrics: Real-time status packets displaying target presence state and
 *                   measured distance in meters.
 *
 * =============================================================================
 * ERROR MESSAGE RESPONSES
 * =============================================================================
 * - "Sensor initialization failed. Check wiring!":
 *   Indicates a handshake failure over UART. Verify that VIN, GND, TX, and RX
 *   lines are properly seated and that the baud rate matches.
 *
 * =============================================================================
 * PROCESSING WORKFLOW & ALGORITHMS
 * =============================================================================
 * 1. Initialization Phase:
 *    - Boot hardware serial interface at 115200 baud.
 *    - Execute protocol handshake (`sens.begin()`). Loop with 1000ms delay on failure.
 * 2. Configuration Phase:
 *    - Push distance constraints via `setDistanceThreshold(0, 3)` to restrict field
 *      of view bounds to 3 meters.
 *    - Set telemetry reporting period to 100ms.
 *    - Initialize environment noise collection routine to filter background elements.
 * 3. Execution Phase (Main Loop):
 *    - Fetch asynchronous frames utilizing `getAllVendorResult()`.
 *    - Validate frame headers against the expected 0xAA sync byte.
 *    - Evaluate target state flags and stream formatted string metrics to host.
 *
 * =============================================================================
 * REFERENCES & NOTES
 * =============================================================================
 * - DFRobot SEN0691 Wiki and C4002 Protocol Specification.
 * - Espressif ESP32-C6 Technical Reference Manual.
 * - Local SOHO network configuration standards utilizing compile-time device IDs.
 */

#include "Arduino.h"
#include "DFRobot_C4002.h"

// Define hardware serial mapping for ESP32-C6
#define SENSOR_SERIAL Serial1

// Instantiate the C4002 driver object referencing the serial port
DFRobot_C4002 sens(&SENSOR_SERIAL);

void setup() {
  // Initialize debugging terminal
  Serial.begin(115200);
  while(!Serial) {;}

  // Initialize sensor serial interface (RX: GPIO 4, TX: GPIO 5)
  SENSOR_SERIAL.begin(115200, SERIAL_8N1, 4, 5);

  Serial.println("Initializing DFRobot SEN0691 mmWave Sensor...");

  // Execute protocol handshake loop
  while (!sens.begin()) {
    Serial.println("Sensor initialization failed. Check wiring!");
    delay(1000);
  }
  Serial.println("Sensor initialized successfully.");

  // Configure detection distance threshold boundaries (0m to 3m limit)
  sens.setDistanceThreshold(0, 3);
  Serial.println("Detection range limited to 3 meters.");

  // Configure telemetry report frequency period (ms)
  sens.setReportPeriod(100);

  // Calibrate baseline environmental noise
  sens.startEnvNoiseCollection();
  Serial.println("Environmental noise calibration routine started.");
}

void loop() {
  // Fetch telemetry results package from the sensor pipeline
  sRetResult_t result = sens.getAllVendorResult();

  // Validate packet header signature
  if (result.header == 0xAA) {
    if (result.targetState == 1) {
      Serial.print("Target Detected! Distance: ");
      Serial.print(result.distance);
      Serial.println(" m");
    } else {
      Serial.println("No target detected.");
    }
  } else {
    Serial.println("Warning: Invalid packet header or sync error.");
  }

  delay(500);
}
