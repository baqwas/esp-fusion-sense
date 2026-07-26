#include <Arduino.h>
#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Set Wi-Fi to station mode and disconnect from an AP if it was previously stored
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("[Wi-Fi] Initializing scan for local access points...");
}

void loop() {
    Serial.println("[Wi-Fi] Starting scan...");
    int n = WiFi.scanNetworks();

    if (n == 0) {
        Serial.println("[Wi-Fi] No networks found.");
    } else {
        Serial.printf("[Wi-Fi] Found %d networks:\n", n);
        for (int i = 0; i < n; ++i) {
            Serial.printf("  %d: %s (Signal: %dBm) [Channel: %d]\n",
                          i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
            delay(10);
        }
    }

    WiFi.scanDelete();
    Serial.println("[Wi-Fi] Scan complete. Waiting 10 seconds for next pass...\n");
    delay(10000);
}
