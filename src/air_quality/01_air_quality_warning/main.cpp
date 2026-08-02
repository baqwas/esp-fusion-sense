/**
 * @file main.cpp
 * @brief Project 1: Air Quality Warning - ENS160 + BME280 for FireBeetle 2 ESP32-C6
 *
 * @purpose Provides real-time environmental monitoring combining multi-gas TVOC/eCO2
 *          analytics with precise temperature and humidity compensation to calculate
 *          an accurate Air Quality Index (AQI).
 * @version 1.2.0
 * @update_history
 *   - 1.0.0 (2026-07-15): Initial prototype structure.
 *   - 1.1.0 (2026-07-22): Integrated BME280 compensation loop for ENS160.
 *   - 1.2.0 (2026-08-01): Added advanced professional documentation headers and structured states.
 *
 * @author Matha Goram
 * @copyright Copyright (c) 2026 ParkCircus Productions
 * @license The MIT License (MIT)
 *
 * @prerequisites
 *   - Hardware: DFRobot FireBeetle 2 ESP32-C6, Fermion ENS160, and BME280 sensors.
 *   - Dependencies: DFRobot_ENS160 and DFRobot_BME280 libraries via PlatformIO.
 *   - Wiring: I2C bus connected to SDA (GPIO 6) and SCL (GPIO 7).
 *
 * @user_interface_guide
 *   - Serial Monitor configured to 115200 baud.
 *   - Displays real-time qualitative status indicators (🟢 Excellent to 🔴 Unhealthy).
 *
 * @error_messages_responses
 *   - "BME280 sensor not detected. Retrying...": Check I2C bus wiring and device address (0x76).
 *   - "ENS160 sensor not detected. Retrying...": Verify power delivery and ENS160 address (0x53).
 *
 * @processing_workflow_and_algorithms
 *   1. Initialize I2C interface and sensor hardware states.
 *   2. Set ENS160 operating mode to standard gas measurement.
 *   3. Loop: Fetch ambient temperature and humidity from BME280.
 *   4. Feed environmental metrics into ENS160 for real-time compensation.
 *   5. Read eCO2, TVOC, and AQI tier values.
 *   6. Output formatted telemetry dashboard to Serial console every 2000ms.
 *
 * @references
 *   - DFRobot ENS160 & BME280 Datasheet and Integration Manuals.
 */

#include <Arduino.h>
#include <Wire.h>
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"

// System-level definitions for I2C and Addresses
#define I2C_SDA_PIN     19  // FireBeetle 2 ESP32-C6 default I2C SDA
#define I2C_SCL_PIN     20 // FireBeetle 2 ESP32-C6 default I2C SCL

// Instantiate sensors using DFRobot libraries
DFRobot_ENS160_I2C ENS160(&Wire, 0x53); // ENS160 default I2C address
DFRobot_BME280_IIC BME(&Wire, 0x76);    // BME280 default I2C address

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n🌿 [INIT] Initializing Air Quality Warning System...");

    // Initialize custom I2C pins for FireBeetle 2 ESP32-C6
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize BME280 environmental sensor
    while (BME.begin() != BME.eStatusOK) {
        Serial.println("⚠️ [ERROR] BME280 sensor not detected. Retrying...");
        delay(1000);
    }
    Serial.println("✅ [SUCCESS] BME280 Online.");

    // Initialize ENS160 air quality sensor
    while (ENS160.begin() != NO_ERR) {
        Serial.println("⚠️ [ERROR] ENS160 sensor not detected. Retrying...");
        delay(1000);
    }
    Serial.println("✅ [SUCCESS] ENS160 Online.");

    // Set ENS160 to standard operating mode (Standard Gas Measurement)
    ENS160.setPWRMode(ENS160_STANDARD_MODE);

    Serial.println("🚀 [READY] Air Quality Warning baseline routine active.\n");
}

void loop() {
    // 1. Fetch ambient compensation data from BME280
    float temp = BME.getTemperature();
    float humidity = BME.getHumidity();
    float pressure = BME.getPressure() / 100.0F; // Convert Pa to hPa

    // Pass temperature and humidity to ENS160 for real-time compensation
    ENS160.setTempAndHum(temp, humidity);

    // 2. Read ENS160 operational metrics
    uint8_t aqi = ENS160.getAQI();
    uint16_t tvoc = ENS160.getTVOC();
    uint16_t eco2 = ENS160.getECO2();

    // 3. Print Structured Metrics Dashboard
    Serial.println("==========================================");
    Serial.printf("🌡️ Temp:       %.2f °C\n", temp);
    Serial.printf("💧 Humidity:   %.2f %%RH\n", humidity);
    Serial.printf("🌪️ Pressure:   %.2f hPa\n", pressure);
    Serial.println("------------------------------------------");
    Serial.printf("🏭 eCO2:       %u ppm\n", eco2);
    Serial.printf("🧪 TVOC:       %u ppb\n", tvoc);
    Serial.printf("📊 AQI Index:  %u ", aqi);

    // Qualitative status check based on official AQI tiers (1-5 scale)
    switch(aqi) {
        case 1: Serial.println("🟢 [EXCELLENT]"); break;
        case 2: Serial.println("🔵 [GOOD]"); break;
        case 3: Serial.println("🟡 [MODERATE]"); break;
        case 4: Serial.println("🟠 [POOR]"); break;
        case 5: Serial.println("🔴 [UNHEALTHY]"); break;
        default: Serial.println("⚪ [CALIBRATING / WARMING UP]"); break;
    }
    Serial.println("==========================================\n");

    // Sample output every 2 seconds
    delay(2000);
}
