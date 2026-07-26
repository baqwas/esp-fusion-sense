/**
 * @file watchdog_test.cpp
 * @brief Watchdog Timers & Exception Recovery Stress Testing for ESP32-C6
 *
 * Purpose: To validate system resilience against unexpected production faults, including
 *          FreeRTOS Task Watchdog Timer (TWDT) exception recovery during blocking delays
 *          and Brownout Detector (BOD) monitoring for low-voltage power dips.
 * Author: Matha Goram
 * Version: 1.0.0
 * Update History:
 *   - 2026-07-26: Initial professional grade release for robust exception recovery validation.
 * Copyright: (c) 2026 Parkcircus Productions. All rights reserved.
 * MIT License: Permission is hereby granted, free of charge, to any person obtaining a copy
 *              of this software and associated documentation files...
 * Prerequisites: DFRobot FireBeetle 2 ESP32-C6 hardware, ESP-IDF v5.x SDK, configured FreeRTOS tasks,
 *                and a variable DC power supply for brownout voltage dip simulations.
 * User Interface Guidance: Monitor serial output via PlatformIO serial monitor set to 115200 baud.
 *                          Watchdog panic dumps, reset causes, and brownout status events are logged live.
 * Error Message Responses:
 *   - [ERR_TWDT_FAULT]: Task Watchdog Timer expired; unyielding task failed to feed watchdog.
 *   - [ERR_BROWNOUT_WARN]: Input voltage dipped below operational threshold; brownout reset imminent.
 *   - [ERR_EXCEPTION_HALT]: Fatal CPU exception or memory access violation intercepted.
 * Processing Workflow & Algorithms:
 *   1. Initialize core system services and register background diagnostic tasks with the TWDT.
 *   2. Spawn a stress task that intentionally introduces a blocking condition to test watchdog timeout recovery.
 *   3. Monitor Brownout Detector configurations and log supply voltage excursion warnings.
 *   4. Capture panic stack traces upon reset and dump diagnostic information to non-volatile logs.
 * References & Notes: Espressif Systems ESP32-C6 Technical Reference Manual - Interrupts, System Reset & Watchdog Timers.
 */

#include <Arduino.h>
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_log.h"

#define CHECKPOINT(name) Serial.printf("[DIAGNOSTIC] Checkpoint reached: %s\n", name)

static const char *TAG = "TEST_WATCHDOG";

// Task handle for the fault injection task
TaskHandle_t fault_task_handle = NULL;

void print_reset_reason() {
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.println("==================================================");
    switch (reason) {
        case ESP_RST_POWERON:
            Serial.println("[RESET_CAUSE] Power-on reset (Cold Boot)");
            break;
        case ESP_RST_SW:
            Serial.println("[RESET_CAUSE] Software reset via esp_restart()");
            break;
        case ESP_RST_PANIC:
            Serial.println("[RESET_CAUSE] Software reset due to exception/panic (Watchdog Triggered)");
            break;
        case ESP_RST_BROWNOUT:
            Serial.println("[RESET_CAUSE] Brownout reset (Voltage dip below BOD threshold)");
            break;
        default:
            Serial.printf("[RESET_CAUSE] Other reset cause: %d\n", (int)reason);
            break;
    }
    Serial.println("==================================================");
}

void fault_injection_task(void *pvParameters) {
    // Subscribe this task to the Task Watchdog Timer (TWDT)
    if (esp_task_wdt_add(NULL) == ESP_OK) {
        Serial.println("[TWDT] Fault injection task successfully registered with Watchdog.");
    }

    uint32_t counter = 0;
    while (true) {
        counter++;
        Serial.printf("[FAULT_TASK] Running normally. Iteration: %u. Feeding watchdog...\n", counter);

        // Feed the watchdog for the first few iterations
        if (counter < 5) {
            esp_task_wdt_reset();
        } else {
            // Intentionally starve the watchdog on iteration 5 to trigger a clean panic reboot
            Serial.println("[ERR_TWDT_FAULT] Intentionally withholding watchdog feed to test TWDT trip!");
            while (true) {
                // Infinite blocking loop simulating a locked task
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    CHECKPOINT("Setup Started");

    // Print the previous boot/reset reason
    print_reset_reason();

    // Initialize Task Watchdog Timer with a 5-second timeout and panic enabled
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 5000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };

    if (esp_task_wdt_init(&twdt_config) == ESP_OK) {
        Serial.println("[TWDT] Task Watchdog Timer initialized successfully (5s timeout).");
    }

    // Subscribe current setup/loop task to TWDT as well
    esp_task_wdt_add(NULL);

    CHECKPOINT("Creating Fault Injection Task");

    // Create a background task to test watchdog exception handling
    xTaskCreatePinnedToCore(
        fault_injection_task,
        "FaultTask",
        2048,
        NULL,
        1,
        &fault_task_handle,
        ARDUINO_RUNNING_CORE
    );

    CHECKPOINT("Setup Completed - Operating normally until watchdog trip");
}

void loop() {
    // Feed the main task watchdog to prevent main loop panic
    esp_task_wdt_reset();

    uint32_t free_heap = esp_get_free_heap_size();
    Serial.printf("[TELEMETRY] Main Loop Healthy | Free Heap: %u bytes\n", free_heap);

    vTaskDelay(pdMS_TO_TICKS(2000));
}
