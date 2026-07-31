/**
 * @file fermion_ens160_test.cpp
 * @author Matha Goram
 * @version 1.0.0
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
 * @brief Validates I2C connectivity, air quality data acquisition, and IoT automation exercises for the DFRobot Fermion ENS160 sensor module.
 *
 * The `fermion_ens160_test.cpp` module initializes the air quality sensor
 * connected to the FireBeetle 2 ESP32-C6. It verifies I2C bus communication,
 * reads raw metrics (TVOC, eCO2, AQI), and executes integrated automation routines
 * for HVAC ventilation, air purifiers, JSON telemetry dashboards, and building automation.
 */

/**
 * @page doc_history 2. Update History
 * @version 1.0.0 | 2026-07-31 | Consolidated standalone ENS160 verification and IoT automation exercises into a single module.
 */

/**
 * @page doc_prereq 3. Prerequisites & Hardware Requirements
 * @hardware
 * - Controller: FireBeetle 2 ESP32-C6 core board.
 * - Sensor: DFRobot Fermion ENS160 Air Quality Sensor.
 * - Wiring: Connect sensor SDA to ESP32 I2C SDA (GPIO 19), SCL to SCL (GPIO 20), VCC to 3.3V, and GND to GND.
 *
 * @software
 * - Development Environment: JetBrains CLion with PlatformIO plugin.
 * - Library Requirement: `DFRobot_ENS160` dependency declared in platformio.ini.
 */

/**
 * @page doc_ui 4. User Interface & Telemetry Guide
 * @ui
 * - Serial Telemetry: Streams initialization logs, discrete automation triggers, and structured JSON telemetry packets over UART0 at 115200 baud, 8-N-1.
 * - Diagnostic Metrics: Prints real-time AQI, TVOC concentration, and eCO2 concentration.
 */

/**
 * @page doc_workflow 5. Processing Workflow & Algorithms
 * @workflow
 * 1. Initialization Phase (`setup()`):
 *    - Starts serial communication channel at 115,200 baud.
 *    - Initializes Wire (I2C) interface explicitly mapped to ESP32-C6 pins 19 (SDA) and 20 (SCL).
 *    - Configures simulation actuator GPIO pins.
 *    - Verifies ENS160 air quality sensor handshaking at address 0x53 and sets operating mode to standard run.
 * 2. Execution Loop (`loop()`):
 *    - Validates data readiness status from the ENS160 using standard library polling functions.
 *    - Reads air quality parameters (AQI, TVOC, eCO2).
 *    - Executes integrated automation routines for HVAC, air purifiers, JSON dashboards, and building thermostats.
 *    - Pauses execution for 5000 milliseconds before subsequent read cycle.
 */

/**
 * @page doc_errors 6. Error Message Responses & Troubleshooting
 * @errors
 * - Symptom: Serial monitor displays initialization failure warnings for ENS160.
 *   - Cause: I2C address mismatch, loose wiring, or missing pull-up resistors on SDA/SCL lines.
 *   - Resolution: Check that GPIO 19 (SDA) and GPIO 20 (SCL) connections are secure and verify address 0x53.
 * - Symptom: Build fails due to missing library headers (`DFRobot_ENS160.h`).
 *   - Cause: Required library dependencies are not declared in `lib_deps` within `platformio.ini`.
 *   - Resolution: Add the appropriate DFRobot library reference to your project build configuration.
 */

/**
 * @page doc_notes 7. References & Notes
 * @note Designed for consolidated standalone testing and practical automation verification within the SOHO sensor node framework.
 * @reference DFRobot Fermion: ENS160 Air Quality Sensor Product Wiki.
 */

#include <Arduino.h>
#include <Wire.h>
#include <DFRobot_ENS160.h>

// Instantiate sensor object using address 0x53 for the Fermion combo board
DFRobot_ENS160_I2C ens160(&Wire, 0x53);

// Output Actuator Pins Simulation
const int VENTILATION_RELAY_PIN = 4;
const int PURIFIER_PWM_PIN = 5;

void setup() {
    // Initialize host debugging serial monitor
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n[INIT] Initializing DFRobot Fermion ENS160 Air Quality Sensor...");

    // Initialize I2C bus explicitly for ESP32-C6 Fermion wiring (GPIO 19 = SDA, GPIO 20 = SCL)
    Wire.begin(19, 20);
    Wire.setClock(100000); // Set standard 100kHz I2C clock for stability
    delay(200);            // Allow I2C bus state machine to stabilize

    pinMode(VENTILATION_RELAY_PIN, OUTPUT);
    pinMode(PURIFIER_PWM_PIN, OUTPUT);

    // Initialize ENS160 Air Quality Sensor
    while (ens160.begin() != NO_ERR) {
        Serial.println("[ERROR] ENS160 sensor not found! Check I2C wiring. Retrying in 2 seconds...");
        delay(2000);
    }

    // Set ENS160 to standard operating mode (IDLE = 0, STANDARD = 1)
    ens160.setPWRMode(ENS160_STANDARD_MODE);
    Serial.println("[SUCCESS] ENS160 sensor initialized and set to standard mode.");
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
    Serial.println("\n----------------------------------------");

    // Read Air Quality Data from ENS160 using standard library polling functions
    uint8_t aqi = ens160.getAQI();
    uint16_t tvoc = ens160.getTVOC();
    uint16_t eco2 = ens160.getECO2();

    Serial.println("[ENS160] Air Quality Metrics:");
    Serial.print("  - Air Quality Index (AQI 1-5): "); Serial.println(aqi);
    Serial.print("  - TVOC Concentration:          "); Serial.print(tvoc); Serial.println(" ppb");
    Serial.print("  - eCO2 Concentration:          "); Serial.print(eco2); Serial.println(" ppm");

    // Execute Automation Routines
    exerciseSmartVentilation(eco2);
    exerciseAirPurifier(tvoc);
    exerciseDashboardTelemetry(aqi, tvoc, eco2);
    exerciseBuildingAutomation(aqi);

    delay(5000); // Poll every 5 seconds
}
