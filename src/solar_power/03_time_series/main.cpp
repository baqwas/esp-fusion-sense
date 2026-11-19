/**
 * @file main.cpp
 * @brief Phase 3: MQTT Time-Series Streaming for FireBeetle 2 ESP32-C6 Solar Node.
 * @path ~/esp-fusion-sense/src/solar_power/03_time_series/main.cpp
 *
 * @version 1.0.0
 * @date 2026-09-17
 *
 * @author Matha Goram
 * @copyright Copyright (c) 2026 ParkCircus Productions
 *
 * ============================================================================
 * PROCESSING WORKFLOW & ALGORITHMS
 * ============================================================================
 * 1. Connects to local Wi-Fi network using credentials.
 * 2. Establishes session connection with the target MQTT broker.
 * 3. Samples analog battery metrics every 10 seconds.
 * 4. Publishes structured JSON payload directly to the telemetry topic.
 *
 * ============================================================================
 * ERROR MESSAGE RESPONSES
 * ============================================================================
 * - {"error": "wifi_disconnected"} -> Wi-Fi link dropped; triggers reconnect sequence.
 * - {"error": "mqtt_connect_failed"} -> Broker unreachable or authentication rejected.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Network Credentials (adjust as needed for your local SOHO infrastructure)
constexpr char WIFI_SSID[] = "Your_SOHO_SSID";
constexpr char WIFI_PASSWORD[] = "Your_SOHO_Password";
constexpr char MQTT_BROKER_IP[] = "192.168.1.100"; // Local broker (e.g., Mosquitto)
constexpr uint16_t MQTT_PORT = 1883;

constexpr char MQTT_TOPIC_TELEMETRY[] = "telemetry/solar_node_01/status";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

constexpr uint8_t BATTERY_ADC_PIN = A0;
constexpr unsigned long TELEMETRY_INTERVAL_MS = 10000;
constexpr float ADC_MAX_RESOLUTIONS = 4095.0f;
constexpr float ESP32_VREF = 3.3f;
constexpr float VOLTAGE_DIVIDER_RATIO = 2.0f;

unsigned long previousMillis = 0;

void setupWiFi() {
    delay(10);
    Serial.printf("\n[INFO] Connecting to Wi-Fi SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[INFO] WiFi connected. IP address: %s\n", WiFi.localIP().toString().c_str());
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("[INFO] Attempting MQTT connection...");
        if (mqttClient.connect("FireBeetleSolarNode01")) {
            Serial.println("[INFO] MQTT connected successfully.");
        } else {
            Serial.printf("[ERROR] mqtt_connect_failed, rc=%d. Retrying in 5 seconds...\n", mqttClient.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    setupWiFi();
    mqttClient.setServer(MQTT_BROKER_IP, MQTT_PORT);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERROR] wifi_disconnected. Reconnecting...");
        setupWiFi();
    }

    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= TELEMETRY_INTERVAL_MS) {
        previousMillis = currentMillis;

        unsigned long uptime = currentMillis / 1000;
        int rawAdc = analogRead(BATTERY_ADC_PIN);
        float batteryVoltage = (static_cast<float>(rawAdc) / ADC_MAX_RESOLUTIONS)
                                 * ESP32_VREF
                                 * VOLTAGE_DIVIDER_RATIO;

        // Build JSON payload string
        char payload[128];
        snprintf(payload, sizeof(payload),
                 "{\"device\":\"firebeetle-esp32c6\",\"uptime_s\":%lu,\"battery_v\":%.2f,\"raw_adc\":%d,\"status\":\"active\"}",
                 uptime, batteryVoltage, rawAdc);

        // Publish to broker
        if (mqttClient.publish(MQTT_TOPIC_TELEMETRY, payload)) {
            Serial.printf("[INFO] Published message: %s\n", payload);
        } else {
            Serial.println("[ERROR] Failed to publish MQTT message.");
        }
    }
}
