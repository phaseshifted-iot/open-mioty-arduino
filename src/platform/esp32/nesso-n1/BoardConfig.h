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
    static constexpr int8_t RADIO_RESET = -1;  // RADIOLIB_NC value
    
    /// SX1262 BUSY pin
    static constexpr int8_t RADIO_BUSY = 19;
    
    // ========================================================================
    // PLATFORM-SPECIFIC SETTINGS
    // ========================================================================
    
    /// Timing offset in PPM to compensate for crystal tolerance (0 = no correction)
    static constexpr int16_t TIMING_OFFSET_PPM = 0;
    
    /// Antenna loss compensation in dBm (positive = loss, negative = gain)
    /// Nesso N1 has poor antenna with ~8 dB loss, so we compensate by adding 8 dBm
    /// Effective radiated power = min(chip_max, regional_max + ANTENNA_LOSS_DBM)
    static constexpr int8_t ANTENNA_LOSS_DBM = 8;
    
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
     *    - Use TsUnbLib::ESP32::enableRadioModule() in setup()
     * 
     * 2. Shared SPI Bus:
     *    - The LoRa module and color display share the same SPI bus
     *    - Each device has its own chip select: RADIO_CS=23, TFT_CS=5
     *    - Initialize display FIRST in setup(), then radio
     *    - Display operations are safe - no interrupts, all polled I/O
     * 
     * 3. Pin Compatibility with Arduino Nesso N1 BSP:
     *    - These pin definitions match the Arduino_Nesso_N1.h board support
     *    - SPI pins (20, 21, 22) are non-default for ESP32 - specified explicitly
     *    - LORA_ENABLE is managed by Arduino BSP - use enableRadioModule()
     * 
     * 4. CRITICAL: mioty Timing Protection
     *    =====================================
     *    mioty requires <20 ppm timing precision. User code MUST cooperate
     *    to avoid breaking transmissions. Follow these patterns:
     * 
     *    ┌─────────────────────────────────────────────────────────────┐
     *    │ DO ✓ - Check transmission status before long operations    │
     *    └─────────────────────────────────────────────────────────────┘
     *    ```cpp
     *    // Safe: Check before doing slow work
     *    if (!miotyNode.Tx.Cpu.isTransmitting()) {
     *        display.updateStatus(count, true);  // OK when not transmitting
     *        readSensor();                        // OK when not transmitting
     *    }
     *    ```
     * 
     *    ┌─────────────────────────────────────────────────────────────┐
     *    │ DO ✓ - Defer work in interrupt handlers                    │
     *    └─────────────────────────────────────────────────────────────┘
     *    ```cpp
     *    // Global deferred task queue
     *    TsUnbLib::ESP32::DeferredTaskQueue taskQueue;
     *    
     *    // Button ISR - CORRECT: Just schedule, don't do work
     *    void IRAM_ATTR buttonISR() {
     *        taskQueue.schedule(handleButtonPress);  // Deferred execution
     *    }
     *    
     *    // Handler runs after transmission completes
     *    void handleButtonPress() {
     *        Serial.println("Button pressed!");
     *        // ... can do slow work here ...
     *    }
     *    
     *    void loop() {
     *        // Execute deferred tasks before transmission
     *        taskQueue.executePending();
     *        
     *        // Transmit (display/tasks freeze during this)
     *        miotyNode.send(data, len);
     *        
     *        // Execute any tasks queued during transmission
     *        taskQueue.executePending();
     *        
     *        delay(60000);
     *    }
     *    ```
     * 
     *    ┌─────────────────────────────────────────────────────────────┐
     *    │ DO ✓ - Display updates before/after transmission           │
     *    └─────────────────────────────────────────────────────────────┘
     *    ```cpp
     *    // Display operations are FAST and SAFE (no interrupts)
     *    display.showTransmitting();      // BEFORE transmission
     *    miotyNode.send(data, len);       // Display frozen during TX
     *    display.updateStatus(cnt, true); // AFTER transmission
     *    ```
     * 
     *    ┌─────────────────────────────────────────────────────────────┐
     *    │ DON'T ✗ - Never do slow work in ISRs                       │
     *    └─────────────────────────────────────────────────────────────┘
     *    ```cpp
     *    // WRONG! This will break mioty timing
     *    void IRAM_ATTR buttonISR() {
     *        Serial.println("Button");  // ✗ BREAKS TIMING!
     *        readSensor();              // ✗ BREAKS TIMING!
     *        delay(100);                // ✗ BREAKS TIMING!
     *    }
     *    ```
     * 
     *    ┌─────────────────────────────────────────────────────────────┐
     *    │ DON'T ✗ - Never interrupt transmissions                    │
     *    └─────────────────────────────────────────────────────────────┘
     *    ```cpp
     *    // WRONG! Don't check sensor during transmission
     *    void loop() {
     *        miotyNode.send(data, len);  // Transmission active
     *        readSensor();  // ✗ Could still be transmitting!
     *    }
     *    
     *    // CORRECT: Wait for completion
     *    void loop() {
     *        int16_t result = miotyNode.send(data, len);
     *        // send() blocks until complete - safe to continue
     *        readSensor();  // ✓ OK now
     *    }
     *    ```
     * 
     *    Why this matters:
     *    - Transmission duration: ~1.7 seconds for 250-byte payload
     *    - Symbol timing: 420 µs ±8.4 µs (20 ppm tolerance)
     *    - Flash cache miss: ~100+ µs delay → breaks timing
     *    - Serial.print(): ~1000+ µs → breaks timing
     *    - Sensor I2C read: ~500+ µs → breaks timing
     * 
     *    The display example (basicTransmitNesso_Display.ino) demonstrates
     *    the correct cooperative pattern - study it for reference!
     */
};

} // namespace NessoN1
} // namespace TsUnbLib

#endif // NESSON1_BOARDCONFIG_H_
