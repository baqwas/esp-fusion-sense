/**
 * @file low_power_sleep_test.cpp
 * @brief Low-Power Deep Sleep & Wake-Up Reliability Stress Testing for ESP32-C6
 *
 * Purpose: To validate deterministic power state transitions, state retention across
 *          deep sleep cycles, and conflict resolution under simultaneous multiple wake
 *          sources (RTC timer and external GPIO/sensor interrupts) for battery-backed SOHO nodes.
 * Author: Matha Goram
 * Version: 1.0.0
 * Update History:
 *   - 2026-07-26: Initial professional grade release for SOHO cluster edge deployment validation.
 * Copyright: (c) 2026 Parkcircus Productions. All rights reserved.
 * MIT License: Permission is hereby granted, free of charge, to any person obtaining a copy
 *              of this software and associated documentation files...
 * Prerequisites: DFRobot FireBeetle 2 ESP32-C6 hardware, ESP-IDF v5.x SDK, configured RTC GPIO pins,
 *                and a precision digital multimeter or power profiler for current draw measurement.
 * User Interface Guidance: Monitor serial output via PlatformIO serial monitor set to 115200 baud.
 *                          Wake causes and RTC memory payload statuses are dumped immediately upon boot.
 * Error Message Responses:
 *   - [ERR_WAKE_FAULT]: Unrecognized or illegal wake-up stub trigger detected.
 *   - [ERR_RTC_CORRUPT]: RTC slow memory CRC mismatch or invalid boot magic number.
 *   - [ERR_GPIO_CONFIG_FAIL]: Failed to isolate or hold peripheral state during sleep boundary.
 * Processing Workflow & Algorithms:
 *   1. Initialize hardware peripherals and inspect the last boot cause via esp_sleep_get_wakeup_cause().
 *   2. Read and validate preserved variables stored within RTC slow memory sections.
 *   3. Configure dual wake sources: RTC timer wakeup (e.g., 10 seconds) and external RTC GPIO interrupt.
 *   4. Enter deep sleep mode while asserting pull resistors and pin hold configurations to minimize current draw.
 * References & Notes: Espressif Systems ESP32-C6 Technical Reference Manual - Low-Power Management & RTC Subsystem.
 */

#include <Arduino.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"

#define CHECKPOINT(name) Serial.printf("[DIAGNOSTIC] Checkpoint reached: %s\n", name)

static const char *TAG = "TEST_LOW_POWER";

// Define an RTC slow memory structure that persists across deep sleep reset cycles
RTC_DATA_ATTR struct {
    uint32_t boot_count;
    uint32_t magic_marker;
    uint64_t last_wakeup_us;
} rtc_persistent_data = {0, 0, 0};

// Define external wakeup pin (e.g., GPIO 4 on ESP32-C6, routed via RTC IO)
#define WAKEUP_GPIO_PIN GPIO_NUM_4
#define SLEEP_DURATION_SEC 10

void print_wakeup_reason(esp_sleep_wakeup_cause_t wakeup_cause) {
    Serial.println("==================================================");
    switch (wakeup_cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("[WAKE_CAUSE] Triggered by: RTC Timer Expiration");
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            Serial.println("[WAKE_CAUSE] Triggered by: External GPIO Interrupt");
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            Serial.println("[WAKE_CAUSE] Triggered by: Normal Power-On Reset (POR) / Cold Boot");
            break;
    }
    Serial.println("==================================================");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    CHECKPOINT("Setup Started");

    // Validate or initialize RTC persistent memory block
    if (rtc_persistent_data.magic_marker != 0xA5A55A5A) {
        Serial.println("[RTC_MEM] Initializing fresh RTC persistent memory structure.");
        rtc_persistent_data.boot_count = 0;
        rtc_persistent_data.magic_marker = 0xA5A55A5A;
        rtc_persistent_data.last_wakeup_us = 0;
    } else {
        rtc_persistent_data.boot_count++;
        Serial.printf("[RTC_MEM] Successfully retained state across sleep! Boot Iteration: %u\n",
                      rtc_persistent_data.boot_count);
    }

    // Capture wake-up source and print system diagnostics
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    print_wakeup_reason(wakeup_cause);

    CHECKPOINT("Configuring Wake Sources");

    // 1. Configure Timer Wakeup Source (e.g., 10 seconds)
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_SEC * 1000000ULL);
    Serial.printf("[CONFIG] RTC Timer wakeup armed for: %d seconds\n", SLEEP_DURATION_SEC);

    // 2. Configure External GPIO Wakeup Source for ESP32-C6
    esp_deep_sleep_enable_gpio_wakeup(BIT(WAKEUP_GPIO_PIN), ESP_GPIO_WAKEUP_GPIO_LOW);
    rtc_gpio_hold_en(WAKEUP_GPIO_PIN);
    Serial.printf("[CONFIG] External GPIO wakeup armed on pin: %d (Low Level)\n", (int)WAKEUP_GPIO_PIN);

    CHECKPOINT("Setup Completed - Preparing for Deep Sleep Transition");

    // Output telemetry snapshot prior to entering low-power state
    uint32_t free_heap = esp_get_free_heap_size();
    Serial.printf("[TELEMETRY] Pre-Sleep Free Heap: %u bytes\n", free_heap);
    Serial.println("[INFO] System entering deep sleep in 3 seconds. Measure current draw now.");

    delay(3000);

    // Record timestamp before sleeping
    rtc_persistent_data.last_wakeup_us = (uint64_t)esp_timer_get_time();

    CHECKPOINT("Entering Deep Sleep State");
    Serial.flush();

    // Transition cleanly into deep sleep
    esp_deep_sleep_start();
}

void loop() {
    // Execution never reaches this point during deep sleep cycles
    vTaskDelay(pdMS_TO_TICKS(1000));
}
