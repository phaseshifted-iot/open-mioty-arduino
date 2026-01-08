/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief Hardware configuration for Arduino Nesso N1 board
 * 
 * This file defines all board-specific hardware parameters for the Arduino Nesso N1.
 * 
 * ARCHITECTURE ROLE:
 * ==================
 * This is the BOARD LAYER - it defines how hardware is connected on this specific board.
 * 
 * - PLATFORM: ESP32-C6 (see ESP32TsUnb.h for MCU-specific implementation)
 * - BOARD: Arduino Nesso N1 (this file defines pin connections)
 * - RADIO: SX1262 (connected via pins defined below)
 * 
 * When creating a new board on the ESP32 platform:
 * 1. Copy this file to a new directory: src/platform/esp32/<your-board>/
 * 2. Update all pin definitions to match your schematic
 * 3. Select the appropriate radio driver (SX1262RadioLib, RFM69HW, etc.)
 * 4. Create Templates.h to combine platform + board + radio + regional settings
 * 
 * @file BoardConfig.h
 */

#ifndef NESSON1_BOARDCONFIG_H_
#define NESSON1_BOARDCONFIG_H_

#include <stdint.h>

namespace TsUnbLib {
namespace NessoN1 {

/**
 * @brief Arduino Nesso N1 Hardware Configuration
 * 
 * The Nesso N1 uses a shared SPI bus for both the LoRa module and the color display.
 * These are non-standard ESP32 pins.
 */
struct BoardConfig {
    // ========================================================================
    // SPI CONFIGURATION
    // ========================================================================
    
    /// SPI clock pin (shared with display)
    static constexpr int8_t SPI_SCK = 20;
    
    /// SPI MISO pin (shared with display)
    static constexpr int8_t SPI_MISO = 22;
    
    /// SPI MOSI pin (shared with display)
    static constexpr int8_t SPI_MOSI = 21;
    
    /// SPI clock frequency for SX1262 communication (4 MHz is safe and reliable)
    static constexpr uint32_t SPI_CLOCK_HZ = 4000000;
    
    // ========================================================================
    // SX1262 RADIO PINS
    // ========================================================================
    
    /// SX1262 chip select (NSS)
    static constexpr int8_t RADIO_CS = 23;
    
    /// SX1262 DIO1 (interrupt)
    static constexpr int8_t RADIO_DIO1 = 15;
    
    /// SX1262 RESET (not used - controlled via LORA_ENABLE on I/O expander)
    /// Must use RADIOLIB_NC when creating SX1262 Module instance
    static constexpr int8_t RADIO_RESET = -2;  // RADIOLIB_NC value
    
    /// SX1262 BUSY pin
    static constexpr int8_t RADIO_BUSY = 19;
    
    // ========================================================================
    // PLATFORM-SPECIFIC SETTINGS
    // ========================================================================
    
    /// Timing offset in PPM to compensate for crystal tolerance (0 = no correction)
    static constexpr int16_t TIMING_OFFSET_PPM = 0;
    
    // ========================================================================
    // BOARD INFORMATION
    // ========================================================================
    
    /// Board name for display/debugging
    static constexpr const char* BOARD_NAME = "Arduino Nesso N1";
    
    /// MCU type
    static constexpr const char* MCU_TYPE = "ESP32-C6";
    
    /// Radio chip
    static constexpr const char* RADIO_TYPE = "SX1262";
    
    // ========================================================================
    // NOTES
    // ========================================================================
    /**
     * IMPORTANT NOTES FOR NESSO N1:
     * 
     * 1. LORA_ENABLE Control:
     *    - The SX1262 is powered/reset via LORA_ENABLE pin on the I/O expander (E0.P7)
     *    - This must be toggled LOW then HIGH before radio initialization
     *    - RadioLib should NOT control the reset pin (use RADIOLIB_NC)
     * 
     * 2. Shared SPI Bus:
     *    - SPI bus is shared between SX1262 radio and the onboard color display
     *    - Uses non-standard pins: SCK=20, MISO=22, MOSI=21
     *    - Each device has its own chip select
     * 
     * 3. External SPI Devices:
     *    - Can be added using the same SPI bus with a separate CS pin
     *    - Available digital pins can be used for CS
     */
};

} // namespace NessoN1
} // namespace TsUnbLib

#endif // NESSON1_BOARDCONFIG_H_
