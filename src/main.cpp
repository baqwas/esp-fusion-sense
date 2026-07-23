//
// Created by reza on 7/23/26.
//
#include <Arduino.h>
#include <Wire.h>
#include "DFRobot_C4002.h"
#include "DFRobot_ENS160.h"
#include "DFRobot_BME280.h"

// If secrets.h is missing, compilation will fail, forcing students to read the README
#include "secrets.h"

// Sensor Instances
DFRobot_ENS160_I2C ENS160(&Wire, /*I2CAddr*/ 0x53);
typedef DFRobot_BME280_IIC BME;
BME BME280(&Wire, /*I2CAddr*/ 0x76);
DFRobot_C4002 c4002;

void setup() {
    Serial.begin(115200);
    // Initialize Hardware Serial 1 for the C4002 mmWave sensor (TX=D3, RX=D2 on FireBeetle C6)
    Serial1.begin(115200, SERIAL_8N1, D2, D3);

    Serial.println("Initializing Multi-Sensor Fusion Engine...");

    // 1. Initialize BME280
    if (BME280.begin() != BME::eStatusOK) {
        Serial.println("BME280 init failed! Check wiring.");
        while(1);
    }

    // 2. Initialize ENS160
    if (ENS160.begin() != 0) {
        Serial.println("ENS160 init failed! Check wiring.");
        while(1);
    }
    ENS160.setPWRMode(ENS160_STANDARD_MODE);

    // 3. Initialize C4002 mmWave
    if (!c4002.begin(Serial1)) {
        Serial.println("C4002 init failed! Check wiring.");
        while(1);
    }

    Serial.println("All sensors online. Entering fusion loop.");
}

void loop() {
    // --- STEP 1: Gather Environmental Data ---
    float temp = BME280.getTemperature();
    float humidity = BME280.getHumidity();

    // Feed temp & humidity to ENS160 for accurate baseline compensation
    ENS160.setTempAndHum(temp, humidity);
    uint16_t tvoc = ENS160.getTVOC();
    uint16_t eco2 = ENS160.getECO2();

    // --- STEP 2: Gather Spatial Data ---
    // Poll the C4002 for presence and distance metrics
    sRetResult_t radarData = c4002.getNoteInfo();

    // --- STEP 3: Print Raw Telemetry (Student Phase 1 & 2) ---
    Serial.printf("Temp: %.2fC | Hum: %.2f%% | TVOC: %d ppb | eCO2: %d ppm\n", temp, humidity, tvoc, eco2);

    if (radarData.noteType == eResult) {
         Serial.printf("Radar Target State: %d | Distance: %.2f m\n",
                       radarData.result.targetState,
                       radarData.result.presenceDis);
    }

    // --- STEP 4: Fusion Logic Placeholder (Student Phase 4) ---
    // Example: if (radarData.result.targetState != 0 && eco2 > 1000) { triggerFan(); }

    Serial.println("----------------------------------------");
    delay(2000);
}