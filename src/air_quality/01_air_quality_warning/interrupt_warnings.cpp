/**
 * @file interrupt_warnings.cpp
 * @brief Project 1: Air Quality Warning - Hardware Interrupts & Thresholds
 *
 * @purpose Demonstrates the use of Interrupt Service Routines (ISRs) and GPIO hardware
 *          interrupts to process environmental data asynchronously. Implements a customized
 *          countdown window for sensor warm-up and threshold triggers to avoid blocking CPU
 *          cycles with continuous polling.
 *
 * @version 1.0.0
 * @update_history
 *   - 1.0.0 (2026-08-01): Initial release implementing ISR-based data acquisition and custom warm-up window.
 *
 * @author Matha Goram
 * @copyright Copyright (c) 2026 ParkCircus Productions. All Rights Reserved.
 * @license The MIT License (MIT)
 *
 * @prerequisites
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, Fermion ENS160, and BME280 sensors.
 *   - Dependencies: DFRobot_ENS160 and DFRobot_BME280 libraries via PlatformIO.
 *   - Wiring:
 *       - I2C SDA -> GPIO 19
 *       - I2C SCL -> GPIO 20
 *       - ENS160 INT -> GPIO 10 (Hardware Interrupt Pin)
 *
 * @user_interface_guide
 *   - Serial Monitor configured to 115200 baud.
 *   - Displays warm-up countdown timer initially.
 *   - Idles silently until an air quality threshold violation (AQI >= 4) is detected.
 *
 * @error_messages_responses
 *   - "⚠️ [ERROR] I2C Bus Failure": Check GPIO 19/20 wiring and pull-up resistors.
 *   - "⏳ [WARM-UP] Sensor stabilizing...": Normal behavior overriding baseline hardware reset loops.
 *
 * @processing_workflow_and_algorithms
 *   1. Initialize I2C interface, mapped to GPIO 19/20.
 *   2. Instantiate and verify BME280 and ENS160 ICs.
 *   3. Execute a customized, non-blocking 3-minute warm-up countdown to bypass baseline hardware reset timing loops.
 *   4. Attach `IRAM_ATTR` ISR to the ENS160 INT pin (GPIO 10) on a FALLING edge.
 *   5. Main Loop: Sleep or idle until the ISR flag is set by the hardware.
 *   6. Upon interrupt, fetch BME280 compensation data, feed to ENS160, and read AQI.
 *   7. If AQI crosses the threshold (>= 4), execute warning protocol.
 *
 * @references
 *   - ESP32-C6 Technical Reference Manual (Interrupt Matrix).
 *   - DFRobot ENS160 Datasheet (INT_CFG register behavior).
 *
 * @notes
 *   - The ISR function must be loaded into IRAM using the IRAM_ATTR attribute to ensure
 *     fast execution and prevent cache miss crashes during operation.
 */

#include <Arduino.h>
#include <Wire.h>
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"

// Hardware Pin Definitions
#define I2C_SDA_PIN     19
#define I2C_SCL_PIN     20
#define ENS160_INT_PIN  10

// Sensor Instantiation
DFRobot_ENS160_I2C ENS160(&Wire, 0x53);
DFRobot_BME280_IIC BME(&Wire, 0x76);

// System State Variables
volatile bool dataReadyFlag = false;
bool isWarmedUp = false;
const uint32_t WARMUP_DURATION_MS = 180000; // 3-minute custom warm-up window
uint32_t bootTime = 0;

/**
 * @brief Interrupt Service Routine (ISR) for ENS160 Data Ready.
 * Keeps execution minimal. Sets a volatile flag for the main loop.
 */
void IRAM_ATTR ens160_isr() {
    dataReadyFlag = true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n⚡ [INIT] Initializing Interrupt-Driven AQI Monitor...");

    // Initialize I2C Bus
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(100000);

    // Initialize Environmental Sensors
    if (BME.begin() != BME.eStatusOK) {
        Serial.println("⚠️ [ERROR] BME280 initialization failed.");
        while (1) { delay(1000); }
    }

    if (ENS160.begin() != NO_ERR) {
        Serial.println("⚠️ [ERROR] ENS160 initialization failed.");
        while (1) { delay(1000); }
    }

    // Configure ENS160 Operational Mode
    ENS160.setPWRMode(ENS160_STANDARD_MODE);

    // Enable Data Ready Interrupt on ENS160
    // Library triggers the INT pin low when new data populates the registers
    ENS160.setINTMode(ENS160.eINTModeEN | ENS160.eINTDataDrdyEN);

    // Attach Hardware Interrupt
    pinMode(ENS160_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENS160_INT_PIN), ens160_isr, FALLING);

    Serial.println("✅ [SUCCESS] Hardware interrupt attached to GPIO 10.");

    // Record boot time for the custom countdown window
    bootTime = millis();
}

void loop() {
    // 1. Customized Countdown Window (Overrides baseline hardware warm-up loop)
    if (!isWarmedUp) {
        uint32_t elapsedTime = millis() - bootTime;
        if (elapsedTime < WARMUP_DURATION_MS) {
            if (elapsedTime % 10000 == 0) { // Log every 10 seconds
                Serial.printf("⏳ [WARM-UP] Sensor stabilizing... %lu seconds remaining.\n",
                              (WARMUP_DURATION_MS - elapsedTime) / 1000);
                delay(1); // Yield to watchdog
            }
            return; // Skip evaluation until fully stabilized
        } else {
            isWarmedUp = true;
            Serial.println("🚀 [READY] Warm-up complete. Entering interrupt-driven idle state.");
        }
    }

    // 2. Asynchronous Event Processing
    if (dataReadyFlag) {
        // Immediately clear the interrupt flag
        dataReadyFlag = false;

        // Fetch ambient compensation data
        float temp = BME.getTemperature();
        float humidity = BME.getHumidity();
        ENS160.setTempAndHum(temp, humidity);

        // Fetch processed AQI
        uint8_t aqi = ENS160.getAQI();

        // 3. Threshold Trigger Logic
        // Only wake up the serial bus and alert if AQI is Poor (4) or Unhealthy (5)
        if (aqi >= 4) {
            uint16_t tvoc = ENS160.getTVOC();
            uint16_t eco2 = ENS160.getECO2();

            Serial.println("\n🚨 [WARNING] AIR QUALITY THRESHOLD VIOLATION DETECTED!");
            Serial.println("==========================================");
            Serial.printf("📊 AQI Index:  %u (DANGER)\n", aqi);
            Serial.printf("🏭 eCO2:       %u ppm\n", eco2);
            Serial.printf("🧪 TVOC:       %u ppb\n", tvoc);
            Serial.printf("🌡️ Temp/Hum:   %.2f °C | %.2f %%RH\n", temp, humidity);
            Serial.println("==========================================\n");

            // Note: In Module 02, this is where we will trigger the HVAC Relay!
        }
    }
}
