/**
 * @file freertos_sync_timer_test.cpp
 * @brief Advanced Real-Time Operating System Mutex Synchronization and Software Timer Test Suite.
 *
 * @details
 * Orchestrates concurrent task resource protection using FreeRTOS Mutexes and manages periodic background
 * actions via FreeRTOS Software Timers. Evaluates atomic access control, callback execution contexts,
 * and timing determinism under multi-task workloads without external peripheral dependencies.
 *
 * @copyright Copyright (c) 2026 ParkCircus Productions. All rights reserved.
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
 * - v1.0.0 (2026-07-25): Initial mutual exclusion lock and software timer callback validation routine.
 *
 * @section prerequisites Prerequisites
 * - PlatformIO Core & Espressif 32 Development Platform.
 * - FireBeetle 2 ESP32-C6 target hardware configuration.
 * - USB-CDC serial bridge driver interface (/dev/ttyACM5 or equivalent).
 *
 * @section user_interface_guide User Interface Guide
 * - Launch the monitor utility using `pio run -e freertos_sync_timer_test_c6 -t upload -t monitor`.
 * - Observe synchronized task logs and periodic software timer callback events streaming over the serial interface.
 *
 * @section error_message_responses Error Message Responses
 * - `[Init] Failed to create mutex!`: Indicates heap exhaustion or initialization failure for synchronization primitives.
 * - `[Init] Failed to create software timer!`: Indicates insufficient timer daemon queue space or heap allocation error.
 *
 * @section processing_workflow_and_algorithms Processing Workflow and Algorithms
 * 1. Initialize system serial communication interface at 115200 baud.
 * 2. Create a FreeRTOS Mutex handle for thread-safe access control over shared output streams.
 * 3. Create a FreeRTOS software timer configured for periodic auto-reload execution.
 * 4. Spawn multiple concurrent worker tasks that compete for the shared mutex lock.
 * 5. Inside the critical section protected by the mutex, tasks safely write telemetry data.
 * 6. The software timer triggers its callback function independently at fixed intervals, logging daemon execution ticks.
 *
 * @section references_and_notes References and Notes
 * - FreeRTOS Real-Time Kernel Documentation & Mutex Synchronization Architecture.
 * - Espressif ESP-IDF Programming Guide for Software Timers and Synchronization Primitives.
 */

#include <Arduino.h>

// Mutex handle for protecting shared resources (Serial output)
SemaphoreHandle_t serialMutex;

// Software timer handle
TimerHandle_t statusTimer;

// Software timer callback function (runs in timer daemon task context)
void vTimerCallback(TimerHandle_t xTimer) {
  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(100)) == pdPASS) {
    Serial.printf("[Timer Callback] Periodic software timer event fired at %lu ms | Free Heap: %d bytes\n",
                  millis(), ESP.getFreeHeap());
    xSemaphoreGive(serialMutex);
  }
}

void workerTask(void *pvParameters) {
  const char *taskName = (const char *)pvParameters;
  uint32_t iteration = 0;

  while (1) {
    // Attempt to acquire the mutex with a 500ms timeout
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(500)) == pdPASS) {
      Serial.printf("[%s] Acquired mutex. Iteration #%lu | Uptime: %lu ms\n",
                    taskName, ++iteration, millis());

      // Simulate critical section work
      vTaskDelay(pdMS_TO_TICKS(200));

      // Release the mutex
      xSemaphoreGive(serialMutex);
    } else {
      Serial.printf("[%s] Mutex acquisition timed out!\n", taskName);
    }

    // Yield before next attempt
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Create the mutex
  serialMutex = xSemaphoreCreateMutex();
  if (serialMutex == NULL) {
    Serial.println("[Init] Failed to create mutex!");
    return;
  }

  // Create a periodic software timer (1000ms period, auto-reload enabled = pdTRUE)
  statusTimer = xTimerCreate(
    "StatusTimer",
    pdMS_TO_TICKS(1000),
    pdTRUE,
    (void *)0,
    vTimerCallback
  );

  if (statusTimer != NULL) {
    xTimerStart(statusTimer, 0);
  } else {
    Serial.println("[Init] Failed to create software timer!");
    return;
  }

  // Create two competing worker tasks sharing the mutex
  xTaskCreate(workerTask, "TaskAlpha", 2048, (void *)"Task Alpha", 1, NULL);
  xTaskCreate(workerTask, "TaskBeta", 2048, (void *)"Task Beta", 1, NULL);
}

void loop() {
  // Main execution loop remains idle while background tasks and timers handle operations
  vTaskDelay(pdMS_TO_TICKS(5000));
}
