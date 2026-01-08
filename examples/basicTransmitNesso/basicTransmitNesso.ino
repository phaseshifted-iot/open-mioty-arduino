/*
 * Basic mioty Transmission Example
 * 
 * This example demonstrates basic mioty packet transmission using the
 * open-mioty-arduino library on Arduino Nesso N1.
 * 
 * Hardware: Arduino Nesso N1 (ESP32-C6 + SX1262)
 * 
 * ⚠️ IMPORTANT DISCLAIMERS:
 * 
 * Third-Party Modified Version of the Fraunhofer TS-UNB-Lib - NOT PRODUCTION SOFTWARE
 * 
 * • For testing, demonstration, and learning purposes ONLY
 * • NOT production-ready or commercially licensed
 * • NOT certified for regulatory compliance
 * • Commercial use requires patent licenses from Sisvel International S.A.
 * • Use at your own risk - no warranties provided
 * 
 * Legal Requirements:
 * • Ensure compliance with local radio regulations (FCC, ETSI, etc.)
 * • Respect duty cycle limits in your region
 * • Do not exceed regional power limits
 * 
 * For production deployment, contact:
 * • Sisvel International S.A. for patent licensing
 * • Fraunhofer IIS for commercial MIOTY software
 * 
 * Created: January 2026
 */

#include <Arduino_Nesso_N1.h>
#include <OpenMioty.h>

// ============================================================================
// Configuration
// ============================================================================

// Network Key (128-bit AES key shared with base station)
// CHANGE THIS to a uniquely generated value for each device!
const uint8_t NETWORK_KEY[16] = {
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

// Short Address (16-bit)
const uint16_t SHORT_ADDRESS = 0x70FF;

// Initial packet counter
uint32_t packetCounter = 0;

// Transmit interval in milliseconds (60 seconds for duty cycle compliance)
const unsigned long TX_INTERVAL = 60000;

// Select your region by uncommenting the appropriate line:
TsUnb::EU1 miotyNode;  // Europe: 868 MHz, 14 dBm
// TsUnb::US0 miotyNode;  // USA: 915 MHz, 22 dBm
// TsUnb::IN0 miotyNode;  // India: 865 MHz, 14 dBm

// ============================================================================
// Setup
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("====================================");
  Serial.println("  mioty Basic Transmit");
  Serial.println("====================================");
  
  // Enable SX1262 radio module
  TsUnbLib::ESP32::enableRadioModule();
  
  // Initialize mioty node
  Serial.print(F("Initializing... "));
  if (miotyNode.init() < 0) {
    Serial.println(F("FAILED!"));
    while (1) delay(1000);
  }
  Serial.println(F("success!"));
  
  // Configure network settings
  uint8_t eui64[8];
  // Get EUI-64 from ESP32 MAC address
  TsUnbLib::ESP32::getEui64(eui64);
  
  miotyNode.setNetworkKey(NETWORK_KEY);
  miotyNode.setEUI64(eui64);
  miotyNode.setShortAddress(SHORT_ADDRESS);

  // Set initial packet counter to avoid discarded duplicates
  miotyNode.setPacketCounter(packetCounter);
  
  // Display configuration
  char euiStr[24];
  miotyNode.getEUI64String(euiStr);
  
  Serial.println();
  Serial.print(F("EUI-64: "));
  Serial.println(euiStr);
  Serial.print(F("Short Address: 0x"));
  Serial.println(miotyNode.getShortAddress(), HEX);
  Serial.println();
  Serial.println(F("Ready to transmit!"));
  Serial.println("====================================");
  Serial.println();
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  // Create packet with counter
  String message = "Hello mioty! #" + String(miotyNode.getPacketCounter());
  uint8_t payload[32];
  message.toCharArray((char*)payload, sizeof(payload));
  uint8_t payloadLength = message.length() + 1;
  
  // Transmit
  Serial.print(F("[mioty] Transmitting packet... "));
  
  int16_t result = miotyNode.send(payload, payloadLength);
  
  if (result == 0) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(result);
  }
  
  // Wait before next transmission
  delay(TX_INTERVAL);
}
