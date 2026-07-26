/**
 * @file freertos_event_notification_test.cpp
 * @brief Advanced Real-Time Operating System Event Groups and Direct Task Notifications Test Suite.
 *
 * @details
 * Orchestrates lightweight inter-task synchronization using FreeRTOS Event Groups (bitmask flag coordination)
 * and direct task notifications. Evaluates multi-event bit-flag waiting conditions, atomic signaling,
 * and low-overhead task awakening under deterministic timing conditions without external peripheral dependencies.
 *
 * @copyright Copyright (c) 2026 Parkcircus Productions. All rights reserved.
 * @license MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @author Matha Goram
 * @version 1.0.0
 * @date 2026-07-25
 *
 * @section update_history Update History
 * - v1.0.0 (2026-07-25): Initial event group bit-masking and direct task notification verification routine.
 *
 * @section prerequisites Prerequisites
 * - PlatformIO Core & Espressif 32 Development Platform.
 * - FireBeetle 2 ESP32-C6 target hardware configuration.
 * - USB-CDC serial bridge driver interface (/dev/ttyACM5 or equivalent).
 *
 * @section user_interface_guide User Interface Guide
 * - Launch the monitor utility using `pio run -e freertos_event_notification_test_c6 -t upload -t monitor`.
 * - Observe real-time serial logs reporting event bit flags, notification receipts, and system heap status.
 *
 * @section error_message_responses Error Message Responses
 * - `[Init] Failed to create event group!`: Indicates system heap exhaustion preventing event group allocation.
 * - `[HandlerTask] Event wait timeout!`: Indicates that expected bit flags were not set within the specified window.
 *
 * @section processing_workflow_and_algorithms Processing Workflow and Algorithms
 * 1. Initialize system serial communication interface at 115200 baud.
 * 2. Create a FreeRTOS Event Group handle to manage synchronized status bit flags.
 * 3. Spawn a signaling producer task and a synchronized handler worker task.
 * 4. The signaling task sequentially sets specific bit flags in the event group and issues direct task notifications.
 * 5. The handler task blocks safely, waiting for multiple event bits to clear or for direct task notifications to arrive.
 * 6. Stream telemetry status and verification counters over the serial interface.
 *
 * @section references_and_notes References and Notes
 * - FreeRTOS Real-Time Kernel Documentation & Event Groups / Task Notifications Architecture.
 * - Espressif ESP-IDF Programming Guide for Advanced Synchronization Primitives.
 */

#include <Arduino.h>

// Define event group bit masks
#define EVENT_BIT_SENSOR_READY (1 << 0)
#define EVENT_BIT_NETWORK_OK   (1 << 1)
#define EVENT_BIT_CONFIG_LOAD  (1 << 2)
#define EVENT_MASK_ALL         (EVENT_BIT_SENSOR_READY | EVENT_BIT_NETWORK_OK | EVENT_BIT_CONFIG_LOAD)

// Event group handle
EventGroupHandle_t systemEventGroup;

// Task handle for direct task notifications
TaskHandle_t handlerTaskHandle = NULL;

void signalingTask(void *pvParameters) {
  uint32_t cycleCount = 0;
  while (1) {
    cycleCount++;
    vTaskDelay(pdMS_TO_TICKS(2000));

    Serial.println("\n[Signaler] --- Starting new synchronization cycle ---");

    // 1. Set individual event bits sequentially
    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupSetBits(systemEventGroup, EVENT_BIT_SENSOR_READY);
    Serial.println("[Signaler] Set BIT_SENSOR_READY");

    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupSetBits(systemEventGroup, EVENT_BIT_NETWORK_OK);
    Serial.println("[Signaler] Set BIT_NETWORK_OK");

    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupSetBits(systemEventGroup, EVENT_BIT_CONFIG_LOAD);
    Serial.println("[Signaler] Set BIT_CONFIG_LOAD");

    // 2. Send a direct task notification to the handler task
    if (handlerTaskHandle != NULL) {
      xTaskNotify(handlerTaskHandle, cycleCount, eSetValueWithOverwrite);
      Serial.printf("[Signaler] Sent direct notification with value %lu\n", cycleCount);
    }
  }
}

void handlerTask(void *pvParameters) {
  uint32_t notificationValue = 0;

  while (1) {
    // Block until all target event bits are set, clearing them upon exit
    EventBits_t uxBits = xEventGroupWaitBits(
      systemEventGroup,
      EVENT_MASK_ALL,
      pdTRUE,        // Clear bits on exit
      pdTRUE,        // Wait for ALL bits (and)
      portMAX_DELAY  // Block indefinitely until conditions met
    );

    Serial.printf("[HandlerTask] Event group triggered! Bits received: 0x%02X | Free Heap: %d bytes\n",
                  (unsigned int)uxBits, ESP.getFreeHeap());

    // Wait for the direct task notification sent by the signaler
    if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &notificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
      Serial.printf("[HandlerTask] Direct notification received! Payload value: %lu\n", notificationValue);
    } else {
      Serial.println("[HandlerTask] Direct notification timeout!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Create the event group
  systemEventGroup = xEventGroupCreate();
  if (systemEventGroup == NULL) {
    Serial.println("[Init] Failed to create event group!");
    return;
  }

  // Create tasks and store handler reference for direct notifications
  xTaskCreate(handlerTask, "HandlerTask", 2048, NULL, 2, &handlerTaskHandle);
  xTaskCreate(signalingTask, "SignalingTask", 2048, NULL, 1, NULL);
}

void loop() {
  // Main loop remains idle while background tasks handle synchronization
  vTaskDelay(pdMS_TO_TICKS(5000));
}
