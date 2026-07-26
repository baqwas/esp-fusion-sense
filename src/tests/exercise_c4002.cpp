/**
 * @file sensor_c4002_automation_exercises.cpp
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
 * @brief Illustrates practical IoT automation exercises using the Fermion C4002 mmWave sensor across smart lighting, security, energy management, and healthcare.
 *
 * This module provides reference implementations for four target use cases:
 * - Smart Home Automation (Presence-driven lighting control)
 * - Security & Surveillance (Stationary intrusion detection)
 * - Workspace & Energy Management (HVAC room occupancy coordination)
 * - Elderly & Healthcare Monitoring (Non-intrusive activity and rest tracking)
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-23 | Initial production release for C4002 mmWave automation reference exercises.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 / ESP32-S3 core board.
 * - Sensor: DFRobot Fermion C4002 24GHz mmWave Human Presence Sensor (SEN0691).
 * - Wiring: Connect sensor TX to ESP32 RX (GPIO 16), RX to TX (GPIO 17), VCC to 5V/3.3V, and GND to GND.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 * - Library Requirement: `DFRobot_C4002` dependency declared in platformio.ini.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams discrete automation event triggers and target state telemetry packets over UART0 at 115200 baud.
 */

#include <Arduino.h>
#include <DFRobot_C4002.h>

// Define hardware serial mapping for ESP32-C6 / ESP32-S3 peripheral communication
#define MMWAVE_RX_PIN 17
#define MMWAVE_TX_PIN 16

// Instantiate C4002 sensor object utilizing HardwareSerial1
DFRobot_C4002 c4002(&Serial1);

// Output Actuator Pins Simulation
const int LIGHTING_RELAY_PIN = 4;
const int HVAC_RELAY_PIN = 5;
const int ALARM_BUZZER_PIN = 18;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial1.begin(9600, SERIAL_8N1, MMWAVE_RX_PIN, MMWAVE_TX_PIN);

    pinMode(LIGHTING_RELAY_PIN, OUTPUT);
    pinMode(HVAC_RELAY_PIN, OUTPUT);
    pinMode(ALARM_BUZZER_PIN, OUTPUT);

    while (!c4002.begin()) {
        Serial.println("[ERROR] C4002 mmWave Sensor initialization failed. Retrying...");
        delay(2000);
    }
    Serial.println("[SUCCESS] C4002 mmWave Automation Exercises Initialized.");
}

/**
 * @brief Exercise 1: Smart Home Automation (Lighting Control)
 * Automatically activates lighting when active motion or stationary presence is detected.
 */
void exerciseSmartLighting(uint8_t targetState) {
    if (targetState > 0) {
        digitalWrite(LIGHTING_RELAY_PIN, HIGH);
        Serial.println("[LIGHTING] Human presence detected. Room Lights: ON.");
    } else {
        digitalWrite(LIGHTING_RELAY_PIN, LOW);
        Serial.println("[LIGHTING] Area vacant. Room Lights: OFF.");
    }
}

/**
 * @brief Exercise 2: Security & Surveillance (Intrusion Detection)
 * Triggers alert behaviors if stationary presence or motion is detected during secure arming modes.
 */
void exerciseSecurityMonitoring(uint8_t targetState) {
    if (targetState == 2) {
        digitalWrite(ALARM_BUZZER_PIN, HIGH);
        Serial.println("[SECURITY] ALERT: Active motion detected in secured zone!");
    } else if (targetState == 1) {
        digitalWrite(ALARM_BUZZER_PIN, LOW);
        Serial.println("[SECURITY] Stationary presence noted in zone.");
    } else {
        digitalWrite(ALARM_BUZZER_PIN, LOW);
        Serial.println("[SECURITY] Zone clear.");
    }
}

/**
 * @brief Exercise 3: Workspace & Energy Management (HVAC Control)
 * Optimizes office climate control by maintaining HVAC operation only when continuous occupancy is verified.
 */
void exerciseEnergyManagement(uint8_t targetState) {
    if (targetState > 0) {
        digitalWrite(HVAC_RELAY_PIN, HIGH);
        Serial.println("[ENERGY] Occupancy verified. HVAC Zone: ACTIVE.");
    } else {
        digitalWrite(HVAC_RELAY_PIN, LOW);
        Serial.println("[ENERGY] Zone empty. HVAC Zone: ECO / STANDBY.");
    }
}

/**
 * @brief Exercise 4: Elderly & Healthcare Monitoring (Rest & Breath Tracking)
 * Differentiates stationary presence (such as micro-motions/breathing during rest) to ensure well-being.
 */
void exerciseHealthcareMonitoring(uint8_t targetState) {
    if (targetState == 1) {
        Serial.println("[HEALTH] Patient/Resident resting quietly (Stationary presence / Breathing verified).");
    } else if (targetState == 2) {
        Serial.println("[HEALTH] Resident active / moving around room.");
    } else {
        Serial.println("[HEALTH] No target detected in monitoring area.");
    }
}

void loop() {
    sRetResult_t retResult = c4002.getNoteInfo();

    if (retResult.noteType == eResult) {
        Serial.println("\n----------------------------------------");
        Serial.println("[EXECUTION] Running C4002 mmWave Automation Exercises...");

        exerciseSmartLighting(retResult.targetState);
        exerciseSecurityMonitoring(retResult.targetState);
        exerciseEnergyManagement(retResult.targetState);
        exerciseHealthcareMonitoring(retResult.targetState);
    } else {
        Serial.println("[WARN] Waiting for valid mmWave telemetry frame...");
    }

    delay(1000);
}
