/**
* @file c4002_systematic_test.cpp
 * @brief Systematic diagnostic test for FireBeetle 2 ESP32-C6 and Fermion C4002
 */

#include <Arduino.h>

// Systematic pin definitions using clear mnemonics
#define C4002_RX_PIN    17
#define C4002_TX_PIN    16
#define C4002_BAUD_RATE 115200

void setup() {
  // Initialize USB CDC Debug Serial
  Serial.begin(115200);
  delay(2500);

  Serial.println("\n[INIT] Starting Systematic C4002 Hardware Validation...");
  Serial.printf("[CONFIG] Target RX Pin: %d | Target TX Pin: %d | Baud: %d\n",
                C4002_RX_PIN, C4002_TX_PIN, C4002_BAUD_RATE);

  // Initialize Hardware Serial1 with explicit ESP32-C6 pin matrix mapping
  // Format: Serial1.begin(baud, protocol, rx_pin, tx_pin)
  Serial1.begin(C4002_BAUD_RATE, SERIAL_8N1, C4002_RX_PIN, C4002_TX_PIN);

  Serial.println("[STATUS] Serial1 channel open. Monitoring data stream...");
  Serial.println("------------------------------------------------------");
}

void loop() {
  // Track raw byte throughput to verify physical communication layer
  static unsigned long byteCounter = 0;

  if (Serial1.available() > 0) {
    uint8_t incomingByte = Serial1.read();
    byteCounter++;

    // Print every incoming byte in Hex format to inspect frames
    Serial.printf("%02X ", incomingByte);

    // Format output neatly every 16 bytes
    if (byteCounter % 16 == 0) {
      Serial.println();
    }
  }
}
