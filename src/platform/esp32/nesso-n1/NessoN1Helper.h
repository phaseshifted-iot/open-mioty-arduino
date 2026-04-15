/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: April 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief High-level helper for Arduino Nesso N1 board initialization
 * 
 * Encapsulates the correct initialization sequence for the Nesso N1:
 * - Battery/power management (keeps board alive on battery)
 * - I2C bus and I/O expander BSP lazy-init ordering
 * - SX1262 radio enable via direct I2C (bypasses BSP RMW bugs)
 * - Display initialization (after radio to avoid SPI bus conflicts)
 * 
 * This helper handles all the hardware-specific gotchas so user sketches
 * can focus on application logic.
 * 
 * @file NessoN1Helper.h
 */

#ifndef NESSON1_HELPER_H_
#define NESSON1_HELPER_H_

#include <Arduino_Nesso_N1.h>
#include "../ESP32Utils.h"

namespace TsUnbLib {
namespace NessoN1 {

/**
 * @brief Initialize all Nesso N1 hardware in the correct order.
 * 
 * Must be called at the very beginning of setup(), before miotyNode.init().
 * Handles battery, I2C, BSP lazy-init ordering, and radio power-on.
 * 
 * @code
 * void setup() {
 *     TsUnbLib::NessoN1::boardInit();
 *     miotyNode.init();
 *     // ...
 * }
 * @endcode
 */
inline void boardInit() {
    // 1. Battery: keep board alive when running on LiPo (no USB)
    NessoBattery battery;
    battery.begin();

    // 2. Serial (USB CDC needs a moment to enumerate)
    Serial.begin(115200);
    delay(3000);

    // 3. I2C bus for the two PI4IOE5V6408 I/O expanders
    Wire.begin();

    // 4. Touch any E0 pin via BSP BEFORE enableRadioModule().
    //    The BSP does a full software-reset of the expander on first access.
    //    If this happens AFTER our direct I2C writes, it wipes
    //    LORA_ANTENNA_SWITCH (P6) → ~40 dB signal loss.
    pinMode(KEY1, INPUT_PULLUP);

    // 5. Enable SX1262 radio (direct I2C to E0: sets P5/P6/P7 as outputs, all HIGH)
    ESP32::enableRadioModule();
}

/**
 * @brief Configure mioty node with EUI-64 derived from ESP32 MAC.
 * 
 * Convenience wrapper that reads the hardware EUI-64 and applies
 * network key, short address, and packet counter in one call.
 * 
 * @tparam Node_T  mioty node type (e.g. TsUnb::EU1)
 * @param node          Reference to the mioty node instance
 * @param networkKey    16-byte AES network key
 * @param shortAddress  16-bit short address
 * @param packetCounter Initial packet counter value
 */
template <typename Node_T>
inline void configureNode(Node_T& node,
                          const uint8_t* networkKey,
                          uint16_t shortAddress,
                          uint32_t packetCounter = 0) {
    uint8_t eui64[8];
    ESP32::getEui64(eui64);

    node.setNetworkKey(networkKey);
    node.setEUI64(eui64);
    node.setShortAddress(shortAddress);
    node.setPacketCounter(packetCounter);

    char euiStr[24];
    node.getEUI64String(euiStr);
    Serial.print(F("EUI-64: "));        Serial.println(euiStr);
    Serial.print(F("Short Address: 0x")); Serial.println(shortAddress, HEX);
    Serial.print(F("Packet Counter: ")); Serial.println(packetCounter);
}

} // namespace NessoN1
} // namespace TsUnbLib

#endif // NESSON1_HELPER_H_
