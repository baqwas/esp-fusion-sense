#include <Arduino.h>
#include <LittleFS.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("[FS] Initializing LittleFS...");

    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS Mount Failed!");
        return;
    }

    Serial.printf("[FS] Total space: %d bytes\n", LittleFS.totalBytes());
    Serial.printf("[FS] Used space:  %d bytes\n", LittleFS.usedBytes());

    // Write a test log file
    File file = LittleFS.open("/test_log.txt", FILE_WRITE);
    if (file) {
        file.println("Boot sequence verified successfully.");
        file.close();
        Serial.println("[FS] Test file written successfully.");
    } else {
        Serial.println("[FS] Failed to open file for writing.");
    }

    // Read back the test log file
    file = LittleFS.open("/test_log.txt", FILE_READ);
    if (file) {
        Serial.print("[FS] Reading file contents: ");
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
    }
}

void loop() {
    // Do nothing
}
