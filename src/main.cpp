/**
 * @file main.cpp
 * @brief Basic ESP32-C6 Firmware integration and multi-sensor telemetry processing suite for DFRobot C4002, ENS160, and BME280 peripherals.
 * @author Matha Goram
 * @version 1.2.0
 * @date 2026-07-27
 *
 * @copyright Copyright (c) 2026 Reza. All rights reserved.
 *
 * @license MIT License
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and inclusion permission notice shall be included in all
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
 * @update_history
 *   - v1.0.0 (2026-05-14): Initial architecture framework design for SOHO environmental nodes.
 *   - v1.1.0 (2026-06-20): Integrated localized static timing overrides and sensor warm-up loops.
 *   - v1.2.0 (2026-07-27): Refactored DFRobot C4002 constructor bindings, hardware serial pin assignments, and CI pipeline secrets integration.
 *
 * @prerequisites
 *   - Hardware: Espressif ESP32-C6-DevKitC-1 microcontroller, DFRobot C4002 mmWave Radar sensor, ENS160 Air Quality sensor, and BME280 Environmental sensor.
 *   - Software: PlatformIO Core, pioarduino platform framework (ESP32 Arduino Core v3.3.11), and compatible C++17 compiler toolchain.
 *   - Configuration: Local `secrets.h` header file providing `WIFI_SSID` and `WIFI_PASSWORD` macros (dynamically injected during CI/CD execution via GitHub Secrets).
 *
 * @user_interface_guide
 *   - Serial Console: Operational messages, telemetry data, and hardware diagnostic statuses are output via UART0 at 115200 baud (8 data bits, no parity, 1 stop bit).
 *   - Monitoring: Real-time target state and presence distance values stream continuously at a default interval of 500ms.
 *
 * @error_messages_responses
 *   - "C4002 initialization failed! Retrying...": Indicates UART communication timeout or handshake failure with the C4002 radar module. The system enters a blocking retry loop with a 1-second delay.
 *   - "ENS160 initialization failed!": Denotes an I2C bus communication error or missing hardware response from the air quality sensor. Execution proceeds in degraded mode.
 *   - "BME280 initialization failed!": Denotes failure to detect the BME280 environmental sensor at the designated I2C address (0x77). Execution proceeds in degraded mode.
 *
 * @processing_workflow_and_algorithms
 *   1. System Initialization: Configure global logging console, hardware serial interface (UART1) for radar communication, and the primary I2C bus (`Wire`).
 *   2. Peripheral Probing: Execute synchronous polling loops to verify hardware responsiveness for the C4002 radar, ENS160 air quality monitor (configured to standard power mode), and BME280 atmospheric sensor.
 *   3. Telemetry Acquisition Loop:
 *      - Query the C4002 radar module via `getRadarData()` to retrieve target presence metrics.
 *      - Extract target state and presence distance parameters from the structured return type.
 *      - Output formatted strings over the primary serial debugging interface.
 *      - Enforce a static 500ms delay between sampling cycles to maintain bus stability.
 *
 * @references
 *   - Espressif ESP32-C6 Technical Reference Manual & Datasheet
 *   - DFRobot C4002, ENS160, and BME280 Arduino Library Documentation
 *   - PlatformIO Build System Guidelines
 *
 * @notes
 *   - UART1 pins are explicitly mapped to GPIO 4 (RX) and GPIO 3 (TX) to match local SOHO hardware wiring layouts.
 *   - Designed for execution under automated CI pipelines using dummy credential injection to satisfy compilation requirements without leaking production secrets.
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <DFRobot_C4002.h>
#include <DFRobot_ENS160.h>
#include <DFRobot_BME280.h>
#include "secrets.h"

// Hardware Serial 1 definitions for C4002 Radar Sensor
constexpr uint8_t C4002_RX_PIN = 4;
constexpr uint8_t C4002_TX_PIN = 3;

/**
 * @brief Instantiate sensor drivers with proper parameters and pin bindings
 */
DFRobot_C4002 c4002(&Serial1, 115200, C4002_RX_PIN, C4002_TX_PIN);
DFRobot_ENS160_I2C ens160(&Wire, ENS160_I2C_ADDR_0);
DFRobot_BME280_I2C bme280(&Wire, 0x77);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial1.begin(115200, SERIAL_8N1, C4002_RX_PIN, C4002_TX_PIN);
    Wire.begin();

    while (c4002.begin() != 0) {
        Serial.println("C4002 initialization failed! Retrying...");
        delay(1000);
    }
    Serial.println("C4002 initialized successfully.");

    if (ens160.begin() != NO_ERR) {
        Serial.println("ENS160 initialization failed!");
    } else {
        ens160.setPWRMode(ENS160_STANDARD_MODE);
    }

    if (!bme280.begin()) {
        Serial.println("BME280 initialization failed!");
    }
}

void loop() {
    sRetResult_t radarData = c4002.getRadarData();

    Serial.print("Target State: ");
    Serial.print(radarData.targetState);
    Serial.print(" | Presence Distance: ");
    Serial.println(radarData.presenceDis);

    delay(500);
}
