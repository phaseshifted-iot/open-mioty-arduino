/*
 * Advanced mioty Transmission Example with Display + Battery
 * 
 * Full-featured mioty demo for Arduino Nesso N1:
 * - Color TFT display with sending/idle states
 * - Button-triggered and periodic transmission
 * - Battery-powered operation (LiPo)
 * - Serial logging
 * 
 * Press the front button (KEY1) or wait 5 minutes for automatic TX.
 * 
 * Hardware: Arduino Nesso N1 (ESP32-C6 + SX1262 + ST7789 Display)
 * Dependencies: OpenMioty, RadioLib, Arduino_Nesso_N1, M5GFX
 * 
 * ⚠️ Third-Party Modified Version of the Fraunhofer TS-UNB-Lib
 *    For testing and learning only — NOT production software.
 *    See LICENSE.md for details.
 * 
 * Created: April 2026
 */

#include <OpenMioty.h>
#include "Display.h"

// ============================================================================
// Configuration — CHANGE THESE to match your base station
// ============================================================================

const uint8_t NETWORK_KEY[16] = {
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

const uint16_t SHORT_ADDRESS  = 0x70FF;
const uint32_t PACKET_COUNTER = 25;

// Transmit interval (milliseconds). 5 minutes as a keep-alive
// More transmits can be triggered with the button
const unsigned long TX_INTERVAL = 300000;

// Region — uncomment ONE line:
TsUnb::EU1 miotyNode;   // Europe 868 MHz
// TsUnb::US0 miotyNode;   // USA 915 MHz
// TsUnb::IN0 miotyNode;   // India 865 MHz

// ============================================================================
// Global state
// ============================================================================

TsUnbLib::NessoN1::Display display;
unsigned long lastTxTime = 0;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_MS = 200;

// ============================================================================
// Setup
// ============================================================================

void setup() {
  // Initialize all Nesso N1 hardware (battery, I2C, radio, button)
  TsUnbLib::NessoN1::boardInit();

  Serial.println(F("===================================="));
  Serial.println(F("  mioty Advanced TX + Display"));
  Serial.println(F("===================================="));

  // Initialize mioty radio
  Serial.print(F("Initializing mioty... "));
  if (miotyNode.init() < 0) {
    Serial.println(F("FAILED!"));
    while (1) { Serial.println(F("ERROR: init failed")); delay(5000); }
  }
  Serial.println(F("success!"));

  // Initialize display (must be AFTER radio to share SPI bus correctly)
  display.begin();

  // Configure network identity
  TsUnbLib::NessoN1::configureNode(miotyNode, NETWORK_KEY,
                                   SHORT_ADDRESS, PACKET_COUNTER);

  Serial.println(F("Ready to transmit!"));
  Serial.println(F("====================================\n"));

  display.showIdle(miotyNode.getPacketCounter());
  lastTxTime = millis();
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  unsigned long now = millis();
  bool trigger = false;

  // Button press (KEY1 = front button)
  if (digitalRead(KEY1) == LOW && (now - lastButtonPress > DEBOUNCE_MS)) {
    lastButtonPress = now;
    trigger = true;
    Serial.println(F("[button] KEY1 pressed"));
  }

  // Periodic timer
  if (now - lastTxTime >= TX_INTERVAL) {
    trigger = true;
    Serial.println(F("[timer] Interval elapsed"));
  }

  if (trigger) {
    uint32_t counter = miotyNode.getPacketCounter();

    // Show "Sending" on display, then return to idle before TX starts
    display.showSending(counter);
    delay(1000);
    display.showIdle(counter);

    // Build payload
    String msg = "Hello mioty! #" + String(counter);
    uint8_t payload[32];
    msg.toCharArray((char*)payload, sizeof(payload));

    // Transmit (blocks ~1.7 s)
    Serial.print(F("[mioty] TX #")); Serial.print(counter); Serial.print(F("... "));
    int16_t result = miotyNode.send(payload, msg.length() + 1);
    if (result == 0) { Serial.println(F("OK")); }
    else { Serial.print(F("err ")); Serial.println(result); }

    lastTxTime = millis();
  }

  delay(50);
}
