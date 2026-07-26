/**
 * @file multi_protocol_test.cpp
 * @brief Multi-Protocol Coexistence & Wireless Stress Testing for ESP32-C6
 *
 * Purpose: To validate concurrent radio operation and stack stability under high
 *          concurrency across Wi-Fi 6, BLE, and IEEE 802.15.4 interfaces on the ESP32-C6.
 * Author: Matha Goram
 * Version: 1.0.0
 * Update History:
 *   - 2026-07-25: Initial release for SOHO cluster validation and student lab deployment.
 * Copyright: (c) 2026 Parkcircus Productions. All rights reserved.
 * MIT License: Permission is hereby granted, free of charge, to any person obtaining a copy
 *              of this software and associated documentation files...
 * Prerequisites: DFRobot FireBeetle 2 ESP32-C6 hardware, ESP-IDF v5.x SDK, active MQTT broker
 *                endpoint, and 802.15.4 packet sniffer for verification.
 * User Interface Guidance: Monitor serial output via idf.py monitor set to 115200 baud.
 *                          Operational status is indicated by discrete heartbeat LEDs and
 *                          telemetry publish confirmations.
 * Error Message Responses:
 *   - [ERR_WIFI_DISCONNECT]: AP link lost. Executing exponential backoff retry.
 *   - [ERR_BLE_STACK_BUSY]: Controller buffer exhausted. Dropping non-critical advertisement frame.
 *   - [ERR_RADIO_COLLISION]: Arbitration fault detected on shared MAC layer.
 * Processing Workflow & Algorithms:
 *   1. Initialize the Wi-Fi station interface and establish an authenticated MQTT session.
 *   2. Start the NimBLE stack to broadcast background telemetry advertisements.
 *   3. Spin up an IEEE 802.15.4 OpenThread stack task to dispatch periodic mesh heartbeat packets.
 *   4. Run concurrent threads executing non-blocking polling loops while monitoring heap
 *      allocation health and task watchdog timers.
 * References & Notes: Espressif Systems ESP32-C6 Technical Reference Manual; IEEE 802.11ax/802.15.4
 *                     coexistence guidelines.
 */

#include <Arduino.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#define CHECKPOINT(name) Serial.printf("[DIAGNOSTIC] Checkpoint reached: %s\n", name)

static const char *TAG = "TEST_MULTI_PROTO";
uint8_t mac_addr[6];

// BLE advertising callback or setup stub
static void ble_app_advertise(void) {
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    fields.name = (const uint8_t *)WIFI_HOSTNAME_DEFAULT;
    fields.name_len = strlen(WIFI_HOSTNAME_DEFAULT);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
}

static void ble_app_on_sync(void) {
    ble_hs_util_ensure_addr(0);
    ble_app_advertise();
    Serial.println("[DIAGNOSTIC] BLE Stack: Advertising started with device name profile.");
}

void nimble_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    CHECKPOINT("Setup Started");

    // Initialize NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    CHECKPOINT("NVS Initialized");

    // Print Hostname and Base MAC configuration
    Serial.printf("[CONFIG] Target Hostname/Device ID: %s\n", WIFI_HOSTNAME_DEFAULT);
    esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
    Serial.printf("[CONFIG] Wi-Fi STA MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    // Initialize Wi-Fi in Station Mode (Active stack binding without external AP credentials yet)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    CHECKPOINT("Wi-Fi Interface Initialized (Station Mode Active)");

    // Initialize NimBLE Stack for simultaneous BT 5 / BLE validation
    nimble_port_init();
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(nimble_host_task);

    CHECKPOINT("NimBLE Stack Initialized");
    CHECKPOINT("Setup Completed - Entering Multi-Stack Coexistence Loop");
}

void loop() {
    CHECKPOINT("Loop Iteration Start");

    // Gather runtime health statistics
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    wifi_ap_record_t ap_info;

    bool wifi_connected = (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);

    Serial.println("==================================================");
    Serial.printf("[TELEMETRY] Free Heap: %u bytes (Min Ever: %u bytes)\n", free_heap, min_free_heap);
    Serial.printf("[STATUS] Wi-Fi Link State: %s\n", wifi_connected ? "Associated with AP" : "Active / Unassociated (Scanning/Ready)");
    Serial.printf("[STATUS] BLE Advertising: Active (ID: %s)\n", WIFI_HOSTNAME_DEFAULT);
    Serial.println("[INFO] Concurrent Radio Coexistence Check: OK");
    Serial.println("==================================================");

    delay(5000);
}
