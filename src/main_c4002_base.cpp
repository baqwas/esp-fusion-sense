/**
 * @file main_c4002_base.cpp
 * @brief Advanced Embedded Firmware for ESP32-C6 C4002 Sensor Integration
 *
 * @section purpose Purpose
 * Provides a robust, polymorphic, and production-grade firmware architecture for managing
 * the DFRobot C4002 mmWave radar sensor or a fully synthetic software mock on the ESP32-C6
 * platform. Integrates a deterministic procedural rules engine for real-time threat evaluation
 * and command dispatch.
 *
 * @section version_history Version & Update History
 * - v1.0.0 (2026-05-12): Initial single-board evaluation baseline with procedural rules.
 * - v1.1.0 (2026-06-04): Integrated USB-CDC enumeration delay and watchdog safety checks.
 * - v1.2.0 (2026-07-27): Refactored into a polymorphic architecture supporting compile-time
 *                        mock/real sensor decoupling while fully preserving procedural rule execution
 *                        and correct library enum mapping.
 *
 * @section author Author
 * Reza
 *
 * @section copyright Copyright Notice
 * Copyright (c) 2026 Reza. All rights reserved.
 *
 * @section license MIT License
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sale copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @section prerequisites Prerequisites
 * - Hardware: ESP32-C6-DevKitC-1 development board, DFRobot C4002 sensor module (optional for mock).
 * - Toolchain: PlatformIO Core with `platform-espressif32` and `toolchain-riscv32-esp` (C++17 compliant).
 * - Libraries: DFRobot_C4002 @ 1.0.0
 *
 * @section ui_guide User Interface Guide
 * Headless operational model communicating over native USB-Serial/JTAG (`/dev/ttyACM0` or equivalent)
 * at 115200 baud, 8-N-1 configuration. Operational states, telemetry logs, and dispatched actions
 * are streamed directly to standard output. Build targets can be toggled via the `USE_MOCK_SENSOR` macro.
 *
 * @section error_responses Error Messages & Responses
 * - `C4002 initialization failed! Retrying in 1s...`: Indicates the hardware radar is not responding.
 *   Verify RX/TX line cross-connections (GPIO 4 / GPIO 3) and check power stability.
 * - `[MOCK] ...`: Confirms that the firmware is operating under software simulation mode without requiring
 *   physical hardware attached.
 *
 * @section workflow Processing Workflow and Algorithms
 * 1. **Initialization Phase:** Delays execution by 1000ms, initializes serial interfaces, and synchronizes
 *    with the sensor driver (or mock layer).
 * 2. **Polymorphic Polling:** Services the underlying buffer to extract macro state (`eTargetState_t`) and
 *    presence metrics (`sPresenceTarget_t`).
 * 3. **Procedural Rules Engine:** Sequentially evaluates parametric thresholds (`THRESHOLD_DISTANCE_M`,
 *    `THRESHOLD_ENERGY`) to determine zone safety, breach alerts, or active tracking states.
 * 4. **Action Dispatcher:** Formats structured JSON action payloads and streams them through the serial interface.
 *
 * @section references References and Notes
 * - DFRobot C4002 mmWave Radar Sensor Technical Datasheet.
 * - Espressif ESP32-C6 Technical Reference Manual (RISC-V Architecture).
 */

#include <Arduino.h>
#include <DFRobot_C4002.h>

// -------------------------------------------------------------------------
// Hardware Configurations
// -------------------------------------------------------------------------
constexpr uint8_t C4002_RX_PIN = 4;
constexpr uint8_t C4002_TX_PIN = 3;
constexpr uint32_t SERIAL_BAUD = 115200;

// -------------------------------------------------------------------------
// Rule Engine Thresholds
// -------------------------------------------------------------------------
constexpr float THRESHOLD_DISTANCE_M = 1.5f;
constexpr uint8_t THRESHOLD_ENERGY = 50;

// Configuration Toggle: Set to 1 for offline testing without hardware, 0 when the physical sensor is attached.
#ifndef USE_MOCK_SENSOR
#define USE_MOCK_SENSOR 1
#endif

/**
 * @class ISensor
 * @brief Abstract interface for sensor implementations to support mock testing and clean decoupling.
 */
class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool begin() = 0;
    virtual sRetResult_t getNoteInfo() = 0;
    virtual eTargetState_t getTargetState() = 0;
    virtual sPresenceTarget_t getPresenceTargetInfo() = 0;
};

/**
 * @class RealC4002Sensor
 * @brief Concrete implementation wrapping the DFRobot C4002 hardware driver.
 */
class RealC4002Sensor : public ISensor {
private:
    DFRobot_C4002 _sensor;

public:
    RealC4002Sensor() : _sensor(&Serial1, SERIAL_BAUD, C4002_RX_PIN, C4002_TX_PIN) {}

    bool begin() override {
        Serial1.begin(SERIAL_BAUD, SERIAL_8N1, C4002_RX_PIN, C4002_TX_PIN);
        return _sensor.begin();
    }

    sRetResult_t getNoteInfo() override {
        return _sensor.getNoteInfo();
    }

    eTargetState_t getTargetState() override {
        return _sensor.getTargetState();
    }

    sPresenceTarget_t getPresenceTargetInfo() override {
        return _sensor.getPresenceTargetInfo();
    }
};

/**
 * @class MockC4002Sensor
 * @brief Simulated sensor implementation for offline development and pipeline testing.
 */
class MockC4002Sensor : public ISensor {
private:
    unsigned long _lastToggle = 0;
    bool _stateFlag = false;

public:
    bool begin() override {
        Serial.println("[MOCK] Initialized simulated C4002 sensor layer.");
        return 0; // Matches DFRobot begin() convention where 0 is success
    }

    sRetResult_t getNoteInfo() override {
        if (millis() - _lastToggle > 3000) {
            _stateFlag = !_stateFlag;
            _lastToggle = millis();
        }
        sRetResult_t dummy = {eNoNote, 0};
        return dummy;
    }

    eTargetState_t getTargetState() override {
        if (_stateFlag) {
            return ePresence;
        }
        return eNoTarget;
    }

    sPresenceTarget_t getPresenceTargetInfo() override {
        sPresenceTarget_t presence;
        if (_stateFlag) {
            presence.distance = 1.0f; // Within THRESHOLD_DISTANCE_M (1.5f)
            presence.energy = 75;     // Above THRESHOLD_ENERGY (50)
        } else {
            presence.distance = 0.0f;
            presence.energy = 0;
        }
        return presence;
    }
};

// Global sensor instance selector based on build configuration
#if USE_MOCK_SENSOR
MockC4002Sensor c4002;
#else
RealC4002Sensor c4002;
#endif

// -------------------------------------------------------------------------
// Function Prototypes
// -------------------------------------------------------------------------
void evaluateEnvironmentAndAct(eTargetState_t state, sPresenceTarget_t presence);
void transmitCommand(const char* actionId, const char* payload);

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup() {
    // Allow native USB-CDC time to enumerate on host before writing logs
    delay(3000);

    Serial.begin(SERIAL_BAUD);

    // Localized static timing override for serial bus stability
    delay(1000);

    Serial.println("\n--- C4002 Radar Evaluation Engine Started ---");

    // Sensor Warm-up and Synchronization Loop
    Serial.print("Synchronizing with C4002...");
    while (c4002.begin() != 0) {
        Serial.println("C4002 initialization failed! Retrying in 1s...");
        delay(1000);
    }
    Serial.println(" OK. Sensor is online.");
    Serial.println("---------------------------------------------");
}

// -------------------------------------------------------------------------
// Main Execution Loop
// -------------------------------------------------------------------------
void loop() {
    // 1. Service the incoming data buffer from the sensor
    sRetResult_t retResult = c4002.getNoteInfo();
    (void)retResult; // Cast to void to suppress unused variable warnings

    // 2. Extract telemetry
    eTargetState_t targetState = c4002.getTargetState();
    sPresenceTarget_t presenceInfo = c4002.getPresenceTargetInfo();

    // 3. Pass telemetry to the Rules Engine
    evaluateEnvironmentAndAct(targetState, presenceInfo);

    // 4. Stagger evaluation cycles
    delay(500);
}

// -------------------------------------------------------------------------
// Rules-Based Decision Engine
// -------------------------------------------------------------------------
/**
 * @brief Evaluates radar telemetry against defined environmental rules and dispatches commands.
 *
 * @param state The current macro state of the target (Absent, Present, Moving).
 * @param presence Struct containing distance (m) and energy (0-100) metrics.
 */
void evaluateEnvironmentAndAct(eTargetState_t state, sPresenceTarget_t presence) {

    // Log the raw telemetry for baseline monitoring
    Serial.printf("[TELEMETRY] State: %d | Distance: %.2fm | Energy: %d \n",
                  static_cast<int>(state), presence.distance, presence.energy);

    // --- RULE 1: Environment is clear ---
    if (state == eNoTarget) {
        transmitCommand("CMD_CLEAR_STATE", "{\"zone_status\":\"empty\",\"alert_level\":0}");
        return;
    }

    // --- RULE 2: Target is present/moving and breaches proximity & energy thresholds ---
    if ((state == ePresence || state == eMotion) &&
        (presence.distance < THRESHOLD_DISTANCE_M) &&
        (presence.energy > THRESHOLD_ENERGY)) {

        char payload[64];
        snprintf(payload, sizeof(payload), "{\"zone_status\":\"breach\",\"dist\":%.2f,\"energy\":%d}",
                 presence.distance, presence.energy);

        transmitCommand("CMD_PROXIMITY_ALERT", payload);
        return;
    }

    // --- RULE 3: Target is present/moving but outside critical proximity ---
    if (state == ePresence || state == eMotion) {
        char payload[64];
        snprintf(payload, sizeof(payload), "{\"zone_status\":\"tracking\",\"dist\":%.2f}",
                 presence.distance);

        transmitCommand("CMD_TRACKING_ACTIVE", payload);
        return;
    }
}

// -------------------------------------------------------------------------
// Action Dispatcher
// -------------------------------------------------------------------------
/**
 * @brief Simulates the transmission of an action command (e.g., via MQTT or serial bridge).
 *
 * @param actionId The routing identifier for the command.
 * @param payload The formatted JSON data payload.
 */
void transmitCommand(const char* actionId, const char* payload) {
    Serial.print("  -> [ACTION DISPATCHED] ID: ");
    Serial.print(actionId);
    Serial.print(" | Payload: ");
    Serial.println(payload);
    Serial.println(); // Formatting break
}
