/**
 * @file freertos_unified_test.cpp
 * @brief Consolidated Real-Time Operating System Core Multiplexing and Heap Stress Test Suite.
 *
 * @details
 * Combines multi-core task scheduling validation with active runtime heap allocation and release cycles.
 * Monitors core affinity, task distribution, uptime telemetry, and dynamic memory reclamation without
 * external peripheral dependencies.
 *
 * @copyright Copyright (c) 2026 Reza. All rights reserved.
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
 * @author Reza
 * @version 1.0.0
 * @date 2026-07-25
 *
 * @section update_history Update History
 * - v1.0.0 (2026-07-25): Initial unified RTOS test merging task scheduling and memory allocation stress tests.
 *
 * @section prerequisites Prerequisites
 * - PlatformIO Core & Espressif 32 Development Platform.
 * - FireBeetle 2 ESP32-C6 target hardware configuration.
 * - USB-CDC serial bridge driver interface (/dev/ttyACM5 or equivalent).
 *
 * @section user_interface_guide User Interface Guide
 * - Launch the monitor utility using `pio run -e freertos_unified_test_c6 -t upload -t monitor`.
 * - Observe core execution markers, heap allocation sizes, and system uptime telemetry.
 *
 * @section error_message_responses Error Message Responses
 * - `[WorkerTask] Allocation failed!`: Indicates system heap exhaustion or fragmentation.
 *
 * @section processing_workflow_and_algorithms Processing Workflow and Algorithms
 * 1. Initialize serial communication interface at 115200 baud.
 * 2. Spawn a background FreeRTOS worker task.
 * 3. Worker task periodically allocates a 1024-byte buffer, writes tracking patterns, logs the free heap, and frees the block.
 * 4. Main loop logs core execution metrics and system uptime.
 *
 * @section references_and_notes References and Notes
 * - FreeRTOS Real-Time Kernel Documentation & Memory Management Architecture.
 */

#include <Arduino.h>

void workerTask(void *pvParameters) {
  while (1) {
    // 1. Memory Allocation Stress Test Segment
    size_t allocSize = 1024;
    uint8_t *buffer = (uint8_t *)malloc(allocSize);

    if (buffer != NULL) {
      memset(buffer, 0xAA, allocSize);
      Serial.printf("[WorkerTask] Core %d | Allocated %u bytes | Free Heap: %d bytes\n",
                    xPortGetCoreID(), allocSize, ESP.getFreeHeap());
      free(buffer);
    } else {
      Serial.println("[WorkerTask] Allocation failed!");
    }

    // 2. Task Pacing Delay
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Create unified worker task
  xTaskCreate(
    workerTask,
    "WorkerTask",
    3072,
    NULL,
    1,
    NULL
  );
}

void loop() {
  Serial.printf("[Loop] Main loop running on core %d | Uptime: %lu ms | Free Heap: %d bytes\n",
                xPortGetCoreID(), millis(), ESP.getFreeHeap());
  delay(1000);
}
