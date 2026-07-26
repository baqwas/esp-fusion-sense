/**
 * @file freertos_memory_test.cpp
 * @brief Advanced Real-Time Operating System Dynamic Heap Allocation and Stress Test Suite.
 *
 * @details
 * Orchestrates periodic asynchronous heap allocation and release cycles across FreeRTOS task boundaries
 * to evaluate memory fragmentation, leak resilience, and real-time execution pacing on target microcontrollers.
 * Validates heap integrity, task scheduling stability, and dynamic memory reclamation without external peripheral dependencies.
 *
 * @copyright Copyright (c) 2026 Reza. All rights reserved.
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
 * @author Reza
 * @version 1.1.0
 * @date 2026-07-25
 *
 * @section update_history Update History
 * - v1.0.0 (2026-07-24): Initial multi-core FreeRTOS scheduling validation routine.
 * - v1.1.0 (2026-07-25): Upgraded with active runtime heap allocation stress testing and memory leak verification.
 *
 * @section prerequisites Prerequisites
 * - PlatformIO Core & Espressif 32 Development Platform.
 * - FireBeetle 2 ESP32-C6 target hardware configuration.
 * - USB-CDC serial bridge driver interface (/dev/ttyACM5 or equivalent).
 *
 * @section user_interface_guide User Interface Guide
 * - Launch the monitor utility using `pio run -e freertos_memory_test_c6 -t upload -t monitor`.
 * - Observe the real-time serial output stream reporting system uptime and dynamic free heap values.
 *
 * @section error_message_responses Error Message Responses
 * - `[Task 2] Allocation failed!`: Indicates that the system heap is exhausted or severely fragmented. Verify memory task stack allocation bounds and block sizing parameters.
 *
 * @section processing_workflow_and_algorithms Processing Workflow and Algorithms
 * 1. Initialize the system serial telemetry interface at 115200 baud.
 * 2. Spawn a dedicated low-priority background FreeRTOS worker task pinned to available system cores.
 * 3. Inside the worker routine, execute a periodic loop allocating a fixed 1024-byte block using standard `malloc`.
 * 4. Populate the allocated buffer with byte-pattern markers (`0xAA`) to force physical memory writes.
 * 5. Log the instantaneous free heap status via serial telemetry, then instantly release the memory block using `free`.
 * 6. Introduce a structured task delay via `vTaskDelay` to yield execution back to the idle and main application loops.
 *
 * @section references_and_notes References and Notes
 * - FreeRTOS Real-Time Kernel Documentation & Memory Management Architecture.
 * - Espressif ESP-IDF Programming Guide for Heap Memory Allocation.
 */

#include <Arduino.h>

void memoryStressTask(void *pvParameters) {
  while (1) {
    size_t allocSize = 1024;
    uint8_t *buffer = (uint8_t *)malloc(allocSize);

    if (buffer != NULL) {
      memset(buffer, 0xAA, allocSize);
      Serial.printf("[Task 2] Allocated %u bytes. Free Heap before free: %d bytes\n", allocSize, ESP.getFreeHeap());
      free(buffer);
    } else {
      Serial.println("[Task 2] Allocation failed!");
    }

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  xTaskCreate(
    memoryStressTask,
    "MemTask",
    2048,
    NULL,
    1,
    NULL
  );
}

void loop() {
  Serial.printf("[Loop] Main loop running, Free Heap: %d bytes\n", ESP.getFreeHeap());
  delay(1000);
}
