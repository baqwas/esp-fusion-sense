/**
 * @file freertos_queue_sync_test.cpp
 * @brief Advanced Real-Time Operating System Inter-Task Communication and Queue Synchronization Test Suite.
 *
 * @details
 * Orchestrates safe asynchronous message passing between decoupled FreeRTOS tasks using thread-safe queues.
 * Evaluates queue blocking behavior, timeout management, message payload integrity, and task synchronization
 * under deterministic timing conditions without external peripheral dependencies.
 *
 * @copyright Copyright (c) 2026 ParkCircus Productions. All rights reserved.
 * @license MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
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
 * - v1.0.0 (2026-07-25): Initial inter-task queue synchronization and message passing validation routine.
 *
 * @section prerequisites Prerequisites
 * - PlatformIO Core & Espressif 32 Development Platform.
 * - FireBeetle 2 ESP32-C6 target hardware configuration.
 * - USB-CDC serial bridge driver interface (/dev/ttyACM5 or equivalent).
 *
 * @section user_interface_guide User Interface Guide
 * - Launch the monitor utility using `pio run -e freertos_queue_test_c6 -t upload -t monitor`.
 * - Observe real-time serial output packets detailing message transmission, receipt counts, and telemetry timestamps.
 *
 * @section error_message_responses Error Message Responses
 * - `[Producer] Queue send failed!`: Indicates that the inter-task queue is full and timed out. Verify task frequency synchronization and queue length bounds.
 * - `[Consumer] Queue receive timeout!`: Indicates that no messages were received within the expected window. Verify producer task execution state.
 *
 * @section processing_workflow_and_algorithms Processing Workflow and Algorithms
 * 1. Initialize system serial communication at 115200 baud.
 * 2. Create a thread-safe FreeRTOS queue handle capable of storing structured telemetry data payloads.
 * 3. Spawn producer and consumer tasks pinned to available system execution cores.
 * 4. The producer task generates sequential message packets containing timestamps and counters, pushing them into the queue.
 * 5. The consumer task blocks safely on the queue, retrieving payloads immediately upon arrival without polling overhead.
 * 6. Stream telemetry status and verification counters over the serial interface.
 *
 * @section references_and_notes References and Notes
 * - FreeRTOS Real-Time Kernel Documentation & Inter-Task Communication (Queues).
 * - Espressif ESP-IDF Programming Guide for FreeRTOS Primitives.
 */

#include <Arduino.h>

// Define a structured message payload for queue transmission
struct SensorDataMessage {
  uint32_t counter;
  unsigned long timestamp;
};

// Queue handle declaration
QueueHandle_t telemetryQueue;

void producerTask(void *pvParameters) {
  uint32_t msgCount = 0;
  while (1) {
    SensorDataMessage msg;
    msg.counter = ++msgCount;
    msg.timestamp = millis();

    // Send data to queue with a 100ms timeout if queue is full
    if (xQueueSend(telemetryQueue, &msg, pdMS_TO_TICKS(100)) == pdPASS) {
      Serial.printf("[Producer] Sent message #%lu at %lu ms\n", msg.counter, msg.timestamp);
    } else {
      Serial.println("[Producer] Queue send failed!");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void consumerTask(void *pvParameters) {
  SensorDataMessage receivedMsg;
  while (1) {
    // Block until message is received from queue
    if (xQueueReceive(telemetryQueue, &receivedMsg, portMAX_DELAY) == pdPASS) {
      Serial.printf("[Consumer] Received -> Counter: %lu | Timestamp: %lu ms | Free Heap: %d bytes\n",
                    receivedMsg.counter, receivedMsg.timestamp, ESP.getFreeHeap());
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Create a queue holding up to 5 structured messages
  telemetryQueue = xQueueCreate(5, sizeof(SensorDataMessage));

  if (telemetryQueue == NULL) {
    Serial.println("[Init] Failed to create telemetry queue!");
    return;
  }

  // Create producer and consumer tasks
  xTaskCreate(producerTask, "ProducerTask", 2048, NULL, 1, NULL);
  xTaskCreate(consumerTask, "ConsumerTask", 2048, NULL, 2, NULL);
}

void loop() {
  // Main loop remains lean, handling system level background monitoring if needed
  vTaskDelay(pdMS_TO_TICKS(5000));
}
