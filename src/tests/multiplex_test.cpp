#include <Arduino.h>

void secondTask(void *pvParameters) {
    while (1) {
        Serial.printf("[Task 2] Running on core %d, Free Heap: %d bytes\n", xPortGetCoreID(), ESP.getFreeHeap());
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Create a background task pinned to core 0 (or available core)
    xTaskCreate(
      secondTask,   // Task function
       "WorkerTask", // Name
      2048,         // Stack size
      NULL,         // Parameters
      1,            // Priority
      NULL          // Task handle
    );
}

void loop() {
    Serial.printf("[Loop] Main loop running, Uptime: %lu ms\n", millis());
    delay(1000);
}
