#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
}

void loop() {
    Serial.println("\n--- INTERNAL HEALTH DIAGNOSTICS ---");
    Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Free Heap: %d KB\n", ESP.getFreeHeap() / 1024);
    Serial.printf("Min Free Heap: %d KB\n", ESP.getMinFreeHeap() / 1024);
    Serial.printf("Max Alloc Heap: %d KB\n", ESP.getMaxAllocHeap() / 1024);
    Serial.printf("Heap Frag: %d%%\n", ESP.getHeapFragmentation());
    Serial.printf("SDK Version: %s\n", ESP.getSdkVersion());
    Serial.println("------------------------------------");
    delay(5000);
}
