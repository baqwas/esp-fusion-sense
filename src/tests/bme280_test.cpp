/**
 * @file bme280_test.cpp
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
 * @brief Validates I2C connectivity and environmental data acquisition for the DFRobot BME280 sensor module.
 *
 * The `bme280_test.cpp` module initializes the meteorological sensor
 * connected to the FireBeetle 2 ESP32-C6. It verifies I2C
 * bus communication and reads temperature, relative humidity, and barometric pressure.
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-23 | Initial production release for BME280 environmental validation.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board.
 * - Sensor: DFRobot BME280 Meteorological Sensor.
 * - Wiring: Connect sensor SDA to ESP32 I2C SDA (GPIO 6), SCL to SCL (GPIO 7), VCC to 3.3V, and GND to GND.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 * - Library Requirement: `DFRobot_BME280` dependency declared in platformio.ini.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams initialization logs and metrics packets over UART0 at 115200 baud, 8-N-1.
 * - Diagnostic Metrics: Prints real-time ambient temperature, humidity, and barometric pressure.
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Starts serial communication channel at 115,200 baud.
 *    - Initializes Wire (I2C) interface mapped to ESP32-C6 default pins.
 *    - Verifies BME280 sensor presence and configures baseline parameters.
 * 2. Execution Loop (`loop()`):
 *    - Measures ambient temperature, humidity, and barometric pressure from the BME280.
 *    - Outputs consolidated telemetry packet to the serial monitor.
 *    - Pauses execution for 5000 milliseconds before subsequent read cycle.
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: Serial monitor displays initialization failure warnings for BME280.
 *   - Cause: I2C address conflict, loose wiring, or missing pull-up resistors on SDA/SCL lines.
 *   - Resolution: Check that GPIO 6 (SDA) and GPIO 7 (SCL) connections are secure and verify power input is stable.
 * - Symptom: Build fails due to missing library headers (`DFRobot_BME280.h`).
 *   - Cause: Required library dependencies are not declared in `lib_deps` within `platformio.ini`.
 *   - Resolution: Add the appropriate DFRobot library reference to your project build configuration.
 */

/**
 * @page doc_notes 7. References & Notes
 * @note Designed for isolated module verification within the SOHO sensor node framework.
 * @reference DFRobot BME280 Meteorological Sensor Product Wiki.
 */

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_BME280.h>

// Instantiate sensor object
DFRobot_BME280_I2C bme280(&Wire, 0x77); // Default BME280 I2C address is usually 0x77 (or 0x76)

void setup() {
    // Initialize host debugging serial monitor
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n[INIT] Initializing DFRobot BME280 Meteorological Sensor...");

    // Initialize I2C bus for ESP32-C6 (GPIO 6 = SDA, GPIO 7 = SCL)
    Wire.begin(6, 7);

    // Initialize BME280 Meteorological Sensor
    while (bme280.begin() != 0) {
        Serial.println("[ERROR] BME280 sensor not found! Check I2C wiring. Retrying in 2 seconds...");
        delay(2000);
    }
    Serial.println("[SUCCESS] BME280 sensor initialized.");
}

void loop() {
    Serial.println("----------------------------------------");

    // Read Meteorological Data from BME280
    float temp = bme280.getTemperature();
    float pressure = bme280.getPressure() / 100.0F; // Convert Pa to hPa
    float humidity = bme280.getHumidity();

    Serial.println("[BME280] Meteorological Metrics:");
    Serial.print("  - Temperature: "); Serial.print(temp); Serial.println(" °C");
    Serial.print("  - Pressure:    "); Serial.print(pressure); Serial.println(" hPa");
    Serial.print("  - Humidity:    "); Serial.print(humidity); Serial.println(" %");

    delay(5000); // Poll every 5 seconds
}
