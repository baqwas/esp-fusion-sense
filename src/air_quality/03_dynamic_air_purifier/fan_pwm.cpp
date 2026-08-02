/**
 * @file fan_pwm.cpp
 * @brief Module 3: Dynamic Air Purifier - Exercise 3.1: PWM Fan Speed Control
 *
 * @purpose Implements hardware Pulse Width Modulation (PWM) on the ESP32-C6 to drive
 *          and control a brushless DC air purifier fan speed across a sliding scale,
 *          transitioning from discrete relay switching to proportional output control.
 *
 * @version 1.0.0
 * @update_history
 *   - 1.0.0 (2026-08-01): Initial release introducing LEDC hardware PWM channel configuration for fan speed control.
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
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, 4-wire PWM-controlled brushless DC fan or transistor-driven DC motor circuit.
 *   - Wiring: PWM control signal connected to GPIO 3; external power supply for fan motor; shared ground.
 *
 * @user_interface_guide
 *   - Serial Monitor (115200 baud) displays current PWM duty cycle percentage and calculated output signal values.
 *
 * @error_messages_responses
 *   - "⚠️ [ERROR] PWM channel assignment failed": Check LEDC channel allocation and pin mapping.
 *
 * @processing_workflow_and_algorithms
 *   1. Configure ESP32-C6 LEDC PWM channel with a standard frequency (25 kHz) and 8-bit resolution.
 *   2. Attach GPIO 3 to the configured PWM channel.
 *   3. Cycle the fan speed through a stepped progression (e.g., 0%, 25%, 50%, 75%, 100% duty cycle).
 *   4. Maintain each speed tier for a designated dwell interval while logging system status.
 *
 * @references_and_notes
 *   - Standard 4-wire computer and purifier fans expect a 25 kHz PWM frequency for optimal motor driver response without audible whining.
 */

#include <Arduino.h>

// Hardware Pin Definitions
#define FAN_PWM_PIN     3

// PWM Configuration Parameters
#define PWM_CHANNEL     0
#define PWM_FREQ        25000 // 25 kHz standard for computer/purifier fans
#define PWM_RESOLUTION  8     // 8-bit resolution (0 - 255)

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌐 [INIT] Initializing Dynamic Air Purifier - Exercise 3.1: PWM Fan Speed Control...");

    // Configure ESP32-C6 LEDC PWM functionality
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(FAN_PWM_PIN, PWM_CHANNEL);

    // Initialize fan to off state
    ledcWrite(PWM_CHANNEL, 0);
}

void loop() {
    // Step through progressive duty cycles: 0%, 25%, 50%, 75%, 100%
    uint8_t testSteps[] = {0, 64, 128, 192, 255};
    const char* speedLabels[] = {"OFF (0%)", "LOW (25%)", "MEDIUM (50%)", "HIGH (75%)", "MAX (100%)"};

    for (int i = 0; i < 5; i++) {
        uint8_t duty = testSteps[i];
        ledcWrite(PWM_CHANNEL, duty);

        Serial.printf("🌀 [PWM] Setting fan speed to: %s (Duty Value: %u/255)\n", speedLabels[i], duty);
        delay(5000); // Hold speed tier for 5 seconds
    }
}
