/**
 * @file hvac_controller.cpp
 * @brief Module 2: HVAC Relay Control - Exercise 2.2: State Machine Debouncing
 *
 * @purpose Implements a robust non-blocking finite state machine (FSM) for HVAC compressor
 *          staging and thermal protection. Enforces mandatory minimum off-time intervals
 *          to prevent short-cycling damage to compressor motors.
 *
 * @version 1.0.0
 * @update_history
 *   - 1.0.0 (2026-08-01): Initial release introducing non-blocking HVAC state machine and compressor protection timers.
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
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6 and relay actuator on GPIO 4.
 *   - Logic: Simulated ambient sensor input driving state transitions.
 *
 * @user_interface_guide
 *   - Serial Monitor (115200 baud) outputs current FSM state, temperature readings, and anti-short-cycle countdowns.
 *
 * @error_messages_responses
 *   - "⚠️ [PROTECTION] Compressor locked out. Minimum off-time remaining: %lu sec": Normal safeguard operation preventing immediate re-engagement.
 *
 * @processing_workflow_and_algorithms
 *   1. Monitor environmental temperature inputs asynchronously.
 *   2. Evaluate state transitions between IDLE, CALL_FOR_COOLING, and COMPRESSOR_LOCKOUT.
 *   3. Enforce a strict 30-second anti-short-cycle lockout timer whenever the compressor shuts down.
 *   4. Actuate relay pins according to validated state outcomes.
 *
 * @references_and_notes
 *   - Industrial HVAC units typically require 3 to 5 minutes of compressor off-time. The 30-second window here serves as an accelerated lab demonstration.
 */

#include <Arduino.h>

#define HVAC_RELAY_PIN  4

// FSM States
enum HvacState {
    STATE_IDLE,
    STATE_CALL_FOR_COOLING,
    STATE_LOCKOUT_DELAY
};

HvacState currentState = STATE_IDLE;

// Timing & Thresholds
const uint32_t LOCKOUT_DURATION_MS = 30000; // 30-second anti-short-cycle protection
uint32_t lockoutTimerStart = 0;
float simulatedTemperature = 24.0f; // Initial ambient degrees Celsius

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌐 [INIT] Initializing HVAC Controller State Machine (Exercise 2.2)...");
    pinMode(HVAC_RELAY_PIN, OUTPUT);
    digitalWrite(HVAC_RELAY_PIN, LOW);
}

void loop() {
    uint32_t currentMillis = millis();

    // Simulate temperature fluctuation for demonstration
    if (currentMillis % 20000 < 10000) {
        simulatedTemperature = 26.5f; // Above threshold, triggers cooling
    } else {
        simulatedTemperature = 22.0f; // Below threshold, satisfies cooling
    }

    switch (currentState) {
        case STATE_IDLE:
            digitalWrite(HVAC_RELAY_PIN, LOW);
            Serial.printf("💤 [FSM: IDLE] Temp: %.2f°C. Monitoring for cooling demand...\n", simulatedTemperature);

            if (simulatedTemperature >= 25.0f) {
                Serial.println("🌡️ [FSM] Cooling threshold crossed! Transitioning to CALL_FOR_COOLING.");
                currentState = STATE_CALL_FOR_COOLING;
            }
            break;

        case STATE_CALL_FOR_COOLING:
            digitalWrite(HVAC_RELAY_PIN, HIGH);
            Serial.printf("⚡ [FSM: COOLING] Relay ENGAGED. Temp: %.2f°C\n", simulatedTemperature);

            if (simulatedTemperature < 23.0f) {
                Serial.println("❄️ [FSM] Target temperature reached. Disengaging compressor and entering lockout.");
                digitalWrite(HVAC_RELAY_PIN, LOW);
                lockoutTimerStart = currentMillis;
                currentState = STATE_LOCKOUT_DELAY;
            }
            break;

        case STATE_LOCKOUT_DELAY:
            {
                uint32_t elapsedLockout = currentMillis - lockoutTimerStart;
                if (elapsedLockout >= LOCKOUT_DURATION_MS) {
                    Serial.println("✅ [FSM] Lockout timer expired. Returning to IDLE state.");
                    currentState = STATE_IDLE;
                } else {
                    uint32_t remainingSec = (LOCKOUT_DURATION_MS - elapsedLockout) / 1000;
                    Serial.printf("⏳ [FSM: LOCKOUT] Compressor resting. %lu seconds remaining.\n", remainingSec);
                }
            }
            break;
    }

    delay(5000); // Evaluation tick interval
}
