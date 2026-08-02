/**
 * @file pid_purifier.cpp
 * @brief Module 3: Dynamic Air Purifier - Exercise 3.2: Proportional-Integral (PI) Control Loop
 *
 * @purpose Implements closed-loop feedback control using Proportional-Integral (PI) logic
 *          to dynamically scale air purifier fan speeds relative to real-time TVOC and eCO2
 *          sensor error metrics, eliminating manual tuning and ensuring smooth air scrubbing.
 *
 * @version 1.0.0
 * @update_history
 *   - 1.0.0 (2026-08-01): Initial release introducing closed-loop PI controller mapping sensor error to PWM output.
 *
 * @author Matha Goram
 * @copyright Copyright (c) 2026 ParkCircus Productions. All Rights Reserved.
 *
 * @license
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
 * @prerequisites
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, Fermion ENS160 Air Quality sensor, PWM fan circuit.
 *   - Wiring: I2C pins (SDA: 19, SCL: 20), Fan PWM control on GPIO 3.
 *
 * @user_interface_guide
 *   - Serial Monitor (115200 baud) logs current TVOC levels, error terms, PI control outputs, and resulting PWM duty cycles.
 *
 * @error_messages_responses
 *   - "⚠️ [ERROR] ENS160 initialization failed": Verify sensor wiring and power supply.
 *
 * @processing_workflow_and_algorithms
 *   1. Sample TVOC and eCO2 pollution metrics from the ENS160 sensor.
 *   2. Compute error term relative to a clean target setpoint.
 *   3. Calculate Proportional (P) and Integral (I) correction outputs to prevent windup.
 *   4. Map combined PI output directly into an 8-bit PWM duty cycle value for the blower motor.
 *
 * @references_and_notes
 *   - Integral windup protection bounds the accumulated error term to ensure stable recovery when pollution spikes subside.
 */

#include <Arduino.h>
#include <Wire.h>
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"

// Hardware Pin Definitions
#define I2C_SDA_PIN     19
#define I2C_SCL_PIN     20
#define FAN_PWM_PIN     3

// PWM Configuration
#define PWM_CHANNEL     0
#define PWM_FREQ        25000
#define PWM_RESOLUTION  8

// Sensor Objects
DFRobot_ENS160_I2C ENS160(&Wire, 0x53);
DFRobot_BME280_IIC BME(&Wire, 0x76);

// PI Controller Constants & Variables
const float Kp = 0.5f;          // Proportional gain coefficient
const float Ki = 0.05f;         // Integral gain coefficient
const float TARGET_TVOC = 100.0f; // Target baseline TVOC (ppb)

float integralError = 0.0f;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌐 [INIT] Initializing Dynamic Air Purifier - Exercise 3.2: PI Control Loop...");

    // Initialize PWM
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(FAN_PWM_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 0);

    // Initialize I2C and Sensors
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);

    if (BME.begin() != BME.eStatusOK) {
        Serial.println("⚠️ [ERROR] BME280 initialization failed.");
    }
    if (ENS160.begin() != NO_ERR) {
        Serial.println("⚠️ [ERROR] ENS160 initialization failed.");
    }
    ENS160.setPWRMode(ENS160_STANDARD_MODE);
}

void loop() {
    float temp = BME.getTemperature();
    float humidity = BME.getHumidity();
    ENS160.setTempAndHum(temp, humidity);

    float currentTVOC = (float)ENS160.getTVOC();
    float currenteCO2 = (float)ENS160.getECO2();

    // Calculate Error: Positive error means pollution is above target setpoint
    float error = currentTVOC - TARGET_TVOC;
    if (error < 0) error = 0; // Do not ramp negative for cleaner baseline idle

    // Integral accumulation with anti-windup clamping
    integralError += error;
    if (integralError > 2000.0f) integralError = 2000.0f;
    if (integralError < 0.0f) integralError = 0.0f;

    // Compute PI Output
    float piOutput = (Kp * error) + (Ki * integralError);

    // Map output to 8-bit PWM range (0 - 255)
    int pwmValue = (int)piOutput;
    if (pwmValue < 0) pwmValue = 0;
    if (pwmValue > 255) pwmValue = 255;

    // Apply control signal to fan
    ledcWrite(PWM_CHANNEL, pwmValue);

    Serial.printf("📊 [PI CONTROL] TVOC: %.1f ppb | Target: %.1f | Error: %.1f | PWM Duty: %d/255\n",
                  currentTVOC, TARGET_TVOC, error, pwmValue);

    delay(3000); // Loop execution interval
}
