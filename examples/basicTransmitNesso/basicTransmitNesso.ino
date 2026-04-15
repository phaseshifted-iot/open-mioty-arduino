/*
 * Basic mioty Transmission Example (no display)
 * 
 * Minimal mioty transmit sketch for Arduino Nesso N1.
 * Sends a packet every 60 seconds with Serial output only.
 * 
 * Hardware: Arduino Nesso N1 (ESP32-C6 + SX1262)
 * Dependencies: OpenMioty, RadioLib, Arduino_Nesso_N1
 * 
 * ⚠️ Third-Party Modified Version of the Fraunhofer TS-UNB-Lib
 *    For testing and learning only — NOT production software.
 *    See LICENSE.md for details.
 * 
 * Created: January 2026
 */

#include <OpenMioty.h>

// ============================================================================
// Configuration — CHANGE THESE to match your base station
// ============================================================================

const uint8_t NETWORK_KEY[16] = {
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

const uint16_t SHORT_ADDRESS  = 0x70FF;
const uint32_t PACKET_COUNTER = 0;

// Transmit interval (milliseconds). 60 seconds respects EU duty cycle.
const unsigned long TX_INTERVAL = 60000;

// Region — uncomment ONE line:
TsUnb::EU1 miotyNode;   // Europe 868 MHz
// TsUnb::US0 miotyNode;   // USA 915 MHz
// TsUnb::IN0 miotyNode;   // India 865 MHz

// ============================================================================
// Setup
// ============================================================================

void setup() {
  // Initialize all Nesso N1 hardware (battery, I2C, radio, button)
  TsUnbLib::NessoN1::boardInit();

  Serial.println(F("===================================="));
  Serial.println(F("  mioty Basic Transmit"));
  Serial.println(F("===================================="));

  // Initialize mioty radio
  Serial.print(F("Initializing mioty... "));
  if (miotyNode.init() < 0) {
    Serial.println(F("FAILED!"));
    while (1) { Serial.println(F("ERROR: init failed")); delay(5000); }
  }
  Serial.println(F("success!"));

  // Configure network identity
  TsUnbLib::NessoN1::configureNode(miotyNode, NETWORK_KEY,
                                   SHORT_ADDRESS, PACKET_COUNTER);

  Serial.println(F("Ready to transmit!"));
  Serial.println(F("====================================\n"));
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  String msg = "Hello mioty! #" + String(miotyNode.getPacketCounter());
  uint8_t payload[32];
  msg.toCharArray((char*)payload, sizeof(payload));

  Serial.print(F("[mioty] TX #"));
  Serial.print(miotyNode.getPacketCounter());
  Serial.print(F("... "));

  int16_t result = miotyNode.send(payload, msg.length() + 1);
  if (result == 0) { Serial.println(F("OK")); }
  else { Serial.print(F("err ")); Serial.println(result); }

  delay(TX_INTERVAL);
}
