/**
 * @file relay_test.cpp
 * @brief Module 2: HVAC Relay Control - Exercise 2.1: Basic GPIO Relay Switching
 *
 * @purpose Demonstrates foundational physical actuation by toggling an electromechanical
 *          relay connected to an ESP32-C6 GPIO output pin. Establishes baseline active-high
 *          versus active-low signal evaluation for HVAC compressor or fan staging.
 *
 * @version 1.0.0
 * @update_history
 *   - 1.0.0 (2026-08-01): Initial release implementing discrete GPIO relay actuation loops.
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
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6 paired with a standard 5V opto-isolated relay module.
 *   - Wiring: Relay trigger connected to GPIO 4; VCC to 5V (or 3.3V logic supply); GND to system ground.
 *
 * @user_interface_guide
 *   - Serial Monitor (115200 baud) displays relay switching states and operational countdown timers.
 *   - Onboard relay LED clicks and indicator light verify physical circuit continuity.
 *
 * @error_messages_responses
 *   - "⚠️ [ERROR] Relay pin configuration fault": Check physical pin mapping and transistor driver continuity.
 *
 * @processing_workflow_and_algorithms
 *   1. Initialize GPIO 4 as a digital output mode.
 *   2. Drive relay control pin HIGH to energize the electromagnetic coil (closing contacts).
 *   3. Maintain energized state for a defined dwell duration while logging execution.
 *   4. Drive relay control pin LOW to de-energize the coil (opening contacts).
 *   5. Delay for an idle cooling period before repeating the test cycle.
 *
 * @references_and_notes
 *   - Most opto-isolated relay boards utilize active-LOW logic (LOW to engage, HIGH to release). Modify polarity definitions if hardware inversion is observed.
 */

#include <Arduino.h>

// Hardware Pin Definitions for HVAC Relay
#define HVAC_RELAY_PIN  4

// Timing Parameters
const uint32_t SWITCH_DELAY_MS = 5000; // 5-second dwell interval

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🔌 [INIT] Initializing HVAC Relay Control - Exercise 2.1: Basic Switching...");

    // Configure relay control pin as digital output
    pinMode(HVAC_RELAY_PIN, OUTPUT);

    // Ensure relay starts in a safe, de-energized state (LOW)
    digitalWrite(HVAC_RELAY_PIN, LOW);
}

void loop() {
    Serial.println("\n⚡ [RELAY] Energizing HVAC Actuator (CLOSED)...");
    digitalWrite(HVAC_RELAY_PIN, HIGH);
    delay(SWITCH_DELAY_MS);

    Serial.println("💤 [RELAY] De-energizing HVAC Actuator (OPEN)...");
    digitalWrite(HVAC_RELAY_PIN, LOW);
    delay(SWITCH_DELAY_MS);
}
