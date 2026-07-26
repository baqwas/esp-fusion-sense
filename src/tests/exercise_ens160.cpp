/**
 * @file sensor_iot_automation_exercises.cpp
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
 * @brief Illustrates practical IoT automation exercises using the ENS160 sensor across HVAC, appliances, dashboards, and building management.
 *
 * This module provides reference implementations for four target use cases:
 * - Smart Ventilation & HVAC Systems (Automatic eCO2 air exchange triggering)
 * - Air Purifiers & Home Appliances (Dynamic TVOC cooking fume response)
 * - Smart Home Automation Dashboards (Structured JSON telemetry for microcontrollers)
 * - Building Automation & Smart Thermostats (Multi-zone AQI health compliance checks)
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-23 | Initial production release for IoT automation reference exercises.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 / ESP32-S3 core board.
 * - Sensor: DFRobot Fermion ENS160 Air Quality Sensor.
 * - Wiring: Connect sensor SDA to GPIO 6, SCL to GPIO 7, VCC to 3.3V, and GND to GND.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 * - Library Requirement: `DFRobot_ENS160` dependency declared in platformio.ini.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams discrete automation event triggers and structured JSON telemetry packets over UART0 at 115200 baud.
 */

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_ENS160.h>

// Instantiate sensor object
DFRobot_ENS160_I2C ens160(&Wire, 0x53);

// Output Actuator Pins Simulation
const int VENTILATION_RELAY_PIN = 4;
const int PURIFIER_PWM_PIN = 5;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(6, 7);
    pinMode(VENTILATION_RELAY_PIN, OUTPUT);
    pinMode(PURIFIER_PWM_PIN, OUTPUT);

    while (ens160.begin() != NO_ERR) {
        Serial.println("[ERROR] ENS160 initialization failed. Retrying...");
        delay(2000);
    }
    ens160.setPWRMode(ENS160_STANDARD_MODE);
    Serial.println("[SUCCESS] ENS160 IoT Automation Exercises Initialized.");
}

/**
 * @brief Exercise 1: Smart Ventilation & HVAC Systems
 * Automatically triggers air exchange relays when eCO2 exceeds healthy thresholds.
 */
void exerciseSmartVentilation(uint16_t eco2) {
    const uint16_t ECO2_THRESHOLD = 1000; // ppm
    if (eco2 > ECO2_THRESHOLD) {
        digitalWrite(VENTILATION_RELAY_PIN, HIGH);
        Serial.println("[HVAC] eCO2 elevated! Ventilation Fan: ACTIVATED.");
    } else {
        digitalWrite(VENTILATION_RELAY_PIN, LOW);
        Serial.println("[HVAC] eCO2 normal. Ventilation Fan: OFF.");
    }
}

/**
 * @brief Exercise 2: Air Purifiers & Home Appliances
 * Dynamically scales purifier fan PWM in response to TVOC cooking fumes or chemical vapors.
 */
void exerciseAirPurifier(uint16_t tvoc) {
    int pwmSpeed = 0;
    if (tvoc > 2000) {
        pwmSpeed = 255; // Max speed
        Serial.println("[APPLIANCE] High TVOC/Fumes detected! Purifier: MAX SPEED.");
    } else if (tvoc > 500) {
        pwmSpeed = 128; // Medium speed
        Serial.println("[APPLIANCE] Moderate TVOC detected. Purifier: MEDIUM SPEED.");
    } else {
        pwmSpeed = 0;   // Idle
        Serial.println("[APPLIANCE] Air clean. Purifier: IDLE.");
    }
    analogWrite(PURIFIER_PWM_PIN, pwmSpeed);
}

/**
 * @brief Exercise 3: Smart Home Automation Dashboards
 * Formats real-time environmental telemetry into structured JSON for MQTT / Dashboard transmission.
 */
void exerciseDashboardTelemetry(uint8_t aqi, uint16_t tvoc, uint16_t eco2) {
    Serial.println("{");
    Serial.print("  \"sensor\": \"ENS160\",\n");
    Serial.print("  \"aqi\": "); Serial.print(aqi); Serial.print(",\n");
    Serial.print("  \"tvoc_ppb\": "); Serial.print(tvoc); Serial.print(",\n");
    Serial.print("  \"eco2_ppm\": "); Serial.print(eco2); Serial.print("\n");
    Serial.println("}");
}

/**
 * @brief Exercise 4: Building Automation & Smart Thermostats
 * Maintains optimal indoor health parameters across commercial zones based on EPA AQI ratings.
 */
void exerciseBuildingAutomation(uint8_t aqi) {
    if (aqi >= 3) {
        Serial.println("[BUILDING] Health Warning: AQI degraded. Dispatching fresh air intake command to Smart Thermostat.");
    } else {
        Serial.println("[BUILDING] Indoor air health parameters optimal.");
    }
}

void loop() {
    if (ens160.getENS160Result() == NO_ERR) {
        uint8_t aqi = ens160.getAQI();
        uint16_t tvoc = ens160.getTVOC();
        uint16_t eco2 = ens160.getECO2();

        Serial.println("\n----------------------------------------");
        Serial.println("[EXECUTION] Running IoT Automation Exercises...");

        exerciseSmartVentilation(eco2);
        exerciseAirPurifier(tvoc);
        exerciseDashboardTelemetry(aqi, tvoc, eco2);
        exerciseBuildingAutomation(aqi);
    } else {
        Serial.println("[WARN] Sensor stabilizing or reading error.");
    }

    delay(5000);
}
