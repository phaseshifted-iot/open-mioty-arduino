/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief SX1262 driver wrapper using RadioLib for TS-UNB transmission
 *
 * This driver wraps RadioLib's SX1262 class to provide the interface
 * expected by Fraunhofer's TS-UNB-Lib, similar to the RFM69HW driver.
 *
 * @authors open-mioty-arduino Contributors
 * @file SX1262RadioLib.h
 */

#ifndef SX1262_RADIOLIB_H_
#define SX1262_RADIOLIB_H_

#include <RadioLib.h>
#include "../core/TsUnb/RadioBurst.h"

namespace TsUnbLib {
namespace Drivers {

/**
 * @brief SX1262 driver for TS-UNB burst transmission
 *
 * This class implements burst transmission using the SX1262 transceiver
 * via RadioLib. It configures the radio for FSK modulation matching
 * mioty specifications.
 *
 * IMPORTANT LIMITATION - BIT RATE PRECISION:
 * ==========================================
 * The SX1262 hardware bit rate generator has limited precision and cannot
 * achieve the exact mioty symbol rate of 2.380371 kbps. This driver uses
 * 2.4 kbps (0.8% error) which is the closest supported rate.
 * 
 * For production mioty compliance, consider:
 * 1. Use RFM69HW which supports precise external symbol clock generation
 * 2. Implement direct DIO2 modulation bypass (requires RadioLib modification)
 * 3. Use SX1262 in CW mode with external FSK modulation (complex)
 *
 * Current implementation is suitable for:
 * - Development and testing
 * - Short-range applications where timing precision is less critical
 * - Evaluating mioty protocol behavior
 *
 * NOT suitable for:
 * - Regulatory compliance testing
 * - Long-range production deployments
 * - Interference-sensitive environments
 *
 * Architecture:
 * - Platform layer (Cpu_T): MCU-specific implementation (ESP32, RP2040, etc.)
 * - Board layer: Defines which pins connect to the radio (passed as template params)
 * - Radio driver: This class - receives pin configuration from board
 * - Regional layer: Provides frequencies and modulation parameters in Hz
 *
 * @tparam Cpu_T Platform implementation (e.g., ESP32TsUnb<RegionalProfile>)
 * @tparam RegionalProfile_T Regional profile (EU1_Profile, US0_Profile, etc.)
 * @tparam NSS_PIN SPI chip select pin for SX1262 (board-specific)
 * @tparam DIO1_PIN DIO1 interrupt pin (board-specific)
 * @tparam RESET_PIN Reset pin (use RADIOLIB_NC if controlled externally)
 * @tparam BUSY_PIN Busy indicator pin (board-specific)
 * @tparam RadioBurst_T Radio burst class
 */
template <class Cpu_T,
          typename RegionalProfile_T,
          int8_t NSS_PIN,
          int8_t DIO1_PIN,
          int8_t RESET_PIN,
          int8_t BUSY_PIN,
          class RadioBurst_T = TsUnb::RadioBurst<>>
class SX1262RadioLib {
public:
    Cpu_T Cpu;

    /**
     * @brief Constructor
     * 
     * Pins are configured at compile-time via template parameters from board configuration.
     * All mioty parameters derived from RegionalProfile_T at compile time.
     */
    SX1262RadioLib()
        : module(new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN)), 
          radio(module), 
          nssPin(NSS_PIN), dio1Pin(DIO1_PIN), resetPin(RESET_PIN), busyPin(BUSY_PIN),
          currentChannel(0) {
    }

    ~SX1262RadioLib() {
        delete module;
    }

    /**
     * @brief Initialize SX1262 radio
     *
     * Configures the SX1262 for FSK modulation with mioty parameters from regional profile.
     * All parameters derived from RegionalProfile_T (no hardcoded values).
     *
     * @return 0 if OK, negative value on error
     */
    int16_t init() {
        Cpu.spiInit();
        delay(100);
        
        Serial.println("[SX1262] Initializing radio...");
        Serial.flush();
        
        // SX1262 frequency step: (32 MHz / 2^25) = 0.9536743164 Hz per step
        static constexpr double SX1262_FREQ_STEP_HZ = 32000000.0 / 33554432.0;
        
        // Convert regional profile frequency (Hz) to MHz for RadioLib
        double initFreqMHz = RegionalProfile_T::FREQ_CH_A_HZ / 1000000.0;
        
        // Frequency deviation: convert to kHz
        // SX1262 requires minimum 600 Hz deviation, round 595.09275 Hz to 600 Hz (0.6 kHz)
        double freqDeviationKHz = 0.6;  // Rounded from 0.59509 - hardware minimum
        
        // Use precise bit rate from regional profile (2.380371 kbps)
        // Try the exact value - if SX1262 rejects it, fall back to 2.38
        double bitRateKbps = RegionalProfile_T::BIT_RATE_KBPS;  // Exact: 2.380371 kbps
        
        Serial.print("[SX1262] Parameters: ");
        Serial.print(initFreqMHz, 3);
        Serial.print(" MHz, ");
        Serial.print(bitRateKbps, 6);
        Serial.print(" kbps (target: ");
        Serial.print(RegionalProfile_T::BIT_RATE_KBPS, 6);
        Serial.print("), ");
        Serial.print(freqDeviationKHz, 3);
        Serial.print(" kHz dev, ");
        Serial.print(RegionalProfile_T::MAX_POWER_DBM);
        Serial.println(" dBm");
        Serial.flush();
        
        // Initialize in LoRa mode first (validated configuration)
        Serial.println("[SX1262] Step 1: radio.begin()...");
        Serial.flush();
        int16_t state = radio.begin(initFreqMHz);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: begin() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 1: OK");
        Serial.flush();
        
        // Switch to FSK mode for mioty - ALL values from regional profile
        Serial.println("[SX1262] Step 2: radio.beginFSK()...");
        Serial.flush();
        state = radio.beginFSK(initFreqMHz,                           // Channel A frequency
                               bitRateKbps,                           // Precise bit rate from profile
                               freqDeviationKHz,                      // Freq deviation from profile
                               4.8,                                   // RX bandwidth (mioty: ~3 kHz)
                               RegionalProfile_T::MAX_POWER_DBM,      // TX power from profile
                               16,                                    // Preamble (will disable)
                               0.0);                                  // TCXO voltage = 0.0 for XTAL
        
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: beginFSK() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 2: OK");
        Serial.flush();
        Serial.println("[SX1262] Step 2: OK");
        Serial.flush();
        
        // Configure PA for high power output (14 dBm / 25 mW)
        // PA config: deviceSel=0 (SX1262), paDutyCycle=0x04, hpMax=0x07
        // This enables the high power PA and sets maximum output power
        Serial.println("[SX1262] Step 3: setPaConfig()...");
        Serial.flush();
        state = radio.setPaConfig(0x04, 0x07, 0x00, 0x01);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setPaConfig() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 3: OK");
        Serial.flush();
        
        // Set output power explicitly from regional profile
        Serial.println("[SX1262] Step 4: setOutputPower()...");
        Serial.flush();
        state = radio.setOutputPower(RegionalProfile_T::MAX_POWER_DBM);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setOutputPower() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 4: OK");
        Serial.flush();
        
        // Configure for mioty - no shaping, no sync, no CRC
        Serial.println("[SX1262] Step 5: setDataShaping()...");
        Serial.flush();
        state = radio.setDataShaping(RADIOLIB_SHAPING_NONE);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setDataShaping() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 5: OK");
        Serial.flush();
        
        Serial.println("[SX1262] Step 6: fixedPacketLengthMode()...");
        Serial.flush();
        state = radio.fixedPacketLengthMode(255);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: fixedPacketLengthMode() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 6: OK");
        Serial.flush();
        
        Serial.println("[SX1262] Step 7: setSyncWord()...");
        Serial.flush();
        uint8_t syncWord[] = {0x00};
        state = radio.setSyncWord(syncWord, 0);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setSyncWord() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 7: OK");
        Serial.flush();
        
        Serial.println("[SX1262] Step 8: setCRC()...");
        Serial.flush();
        state = radio.setCRC(false);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setCRC() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 8: OK");
        Serial.flush();
        Serial.println("[SX1262] Step 8: OK");
        Serial.flush();
        
        // Frequency already set in beginFSK() above
        // Set to standby mode to save power
        Serial.println("[SX1262] Step 9: standby()...");
        Serial.flush();
        radio.standby();
        
        Serial.println("[SX1262] ✓ Initialization complete!");
        Serial.flush();
        
        Cpu.spiDeinit();
        return 0;
        
#if 0  // End of disabled old FSK code
        if (state == RADIOLIB_ERR_NONE) {
            Serial.println("[SX1262] Step 8c: FSK mode active! Now applying mioty settings...");
            Serial.flush();
            
            // Convert CHAN_A register value to MHz for RadioLib
            const double SX1262_FREQ_STEP = 32000000.0 / 33554432.0;  // ~0.9537 Hz/step
            double initFreqMHz = (CHAN_A_REG * SX1262_FREQ_STEP) / 1000000.0;
            
            // Set initial frequency to Channel A (regional profile)
            state = radio.setFrequency(initFreqMHz);
            if (state != RADIOLIB_ERR_NONE) {
                Serial.print("[SX1262] ERROR: setFrequency failed: ");
                Serial.println(state);
                Serial.flush();
                return -1;
            }
            Serial.print("[SX1262]   setFrequency(");
            Serial.print(initFreqMHz, 3);
            Serial.println(" MHz) OK");
            Serial.flush();
            
            // Now reconfigure for mioty parameters
            state = radio.setBitRate(2.38);
            if (state != RADIOLIB_ERR_NONE) {
                Serial.print("[SX1262] ERROR: setBitRate(2.38) failed: ");
                Serial.println(state);
                Serial.flush();
                return -1;
            }
            Serial.println("[SX1262]   setBitRate(2.38) OK");
            Serial.flush();
            
            state = radio.setFrequencyDeviation(FREQ_DEV_HZ / 1000.0);
            if (state != RADIOLIB_ERR_NONE) {
                Serial.print("[SX1262] ERROR: setFrequencyDeviation failed: ");
                Serial.println(state);
                Serial.flush();
                return -1;
            }
            Serial.println("[SX1262]   setFrequencyDeviation OK");
            Serial.flush();
            
            state = radio.setRxBandwidth(125.0);
            if (state != RADIOLIB_ERR_NONE) {
                Serial.print("[SX1262] ERROR: setRxBandwidth failed: ");
                Serial.println(state);
                Serial.flush();
                return -1;
            }
            Serial.println("[SX1262]   setRxBandwidth OK");
            Serial.flush();
            
            Serial.println("[SX1262] Step 8b: All mioty parameters applied successfully!");
            state = RADIOLIB_ERR_NONE;  // Success
        }
        
        Serial.println("[SX1262] Step 9: Configuration complete");
        Serial.print("[SX1262]          Final result: ");
        Serial.println(state);
        Serial.flush();
        
        // Decode common RadioLib error codes
        if (state != RADIOLIB_ERR_NONE) {
            Serial.println("[SX1262] Step 10: ERROR DETECTED!");
            Serial.flush();
            Serial.println("[SX1262] ============ ERROR DETAILS ============");
            Serial.print("[SX1262] Error code: ");
            Serial.println(state);
            Serial.flush();
            
            // Common RadioLib error codes
            Serial.println("[SX1262] Error code meanings:");
            Serial.flush();
            
            if (state == -2) {
                Serial.println("[SX1262]   -2: CHIP_NOT_FOUND - SPI not working");
            } else if (state == -706) {
                Serial.println("[SX1262]   -706: SPI_CMD_FAILED - Check TCXO/XTAL setting");
            } else if (state == -707) {
                Serial.println("[SX1262]   -707: INVALID_BIT_RATE or INVALID_TCXO");
                Serial.println("[SX1262]        Possible causes:");
                Serial.println("[SX1262]        - Bit rate out of range (0.6-300 kbps)");
                Serial.println("[SX1262]        - TCXO voltage set for XTAL board");
                Serial.println("[SX1262]        - Invalid frequency deviation");
            } else if (state == -709) {
                Serial.println("[SX1262]   -709: Invalid frequency");
            } else {
                Serial.print("[SX1262]   Error ");
                Serial.print(state);
                Serial.println(" - Check RadioLib docs");
            }
            Serial.flush();
            
            Serial.println("[SX1262] ========================================");
            Serial.flush();
            
            return -1;
        }
        
        Serial.println("[SX1262] Step 10: SUCCESS! beginFSK OK");
        Serial.flush();

        // Configure for continuous transmission mode (mioty bursts)
        // Disable preamble and sync word for direct mioty data transmission
        Serial.println("[SX1262] Step 11: Configuring data shaping...");
        Serial.flush();
        state = radio.setDataShaping(RADIOLIB_SHAPING_NONE); // No Gaussian shaping
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setDataShaping failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -2;
        }

        // Set to fixed packet length mode with maximum length
        Serial.println("[SX1262] Setting fixed packet length mode...");
        Serial.flush();
        state = radio.fixedPacketLengthMode(255);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: fixedPacketLengthMode failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -3;
        }

        // Disable sync word
        Serial.println("[SX1262] Disabling sync word...");
        Serial.flush();
        uint8_t syncWord[] = {0x00};
        state = radio.setSyncWord(syncWord, 0);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setSyncWord failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -4;
        }

        // Disable CRC (mioty handles this)
        Serial.println("[SX1262] Disabling CRC...");
        Serial.flush();
        state = radio.setCRC(false);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setCRC failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -5;
        }

        // Set to standby mode to save power
        Serial.println("[SX1262] Setting standby mode...");
        Serial.flush();
        radio.standby();

        Serial.println("[SX1262] Initialization complete!");
        Serial.println("[SX1262] Deinitializing SPI...");
        Serial.flush();
        Cpu.spiDeinit();
        
        Serial.println("[SX1262] === INIT SUCCESS ===");
        Serial.flush();
        return 0;
#endif  // End of disabled FSK code
    }

    /**
     * @brief Transmit mioty radio bursts
     *
     * TIME-CRITICAL: No Serial.print() calls during transmission!
     * All bursts must be transmitted with precise timing.
     *
     * Frequency management:
     * - frequency parameter from Phy::calcFreqReg() contains base frequency
     *   in legacy RFM69 register units (for compatibility with existing Phy code)
     * - Burst carrier offsets also in RFM69 units (carrier_index * B_c)
     * - This function converts to Hz using regional profile's CARRIER_SPACING_HZ
     *
     * @param Bursts Pointer to array of radio bursts
     * @param numTxBursts Number of bursts to transmit
     * @param frequency Base frequency f0 in RFM69 register units (legacy)
     * @return 0 if OK, negative value on error
     */
    int16_t transmit(const RadioBurst_T* const Bursts, 
                     const uint16_t numTxBursts, 
                     const uint32_t frequency) {
        Cpu.spiInit();
        Cpu.initTimer();

        // Legacy RFM69 frequency step (for compatibility with existing Phy code)
        // The Phy layer still uses RFM69 register format
        static constexpr double RFM69_FREQ_STEP_HZ = Regional::MiotySpec::RFM69_FREQ_STEP_HZ;
        
        // Carrier spacing from regional profile (in Hz)
        static constexpr double CARRIER_SPACING_HZ = RegionalProfile_T::CARRIER_SPACING_HZ;
        
        // Convert base frequency from RFM69 register units to Hz
        double baseFreqHz = frequency * RFM69_FREQ_STEP_HZ;
        double baseFreqMHz = baseFreqHz / 1000000.0;

        // Configure PA and set transmit power from regional profile
        radio.setPaConfig(0x04, 0x07, 0x00, 0x01);  // High power PA
        radio.setOutputPower(RegionalProfile_T::MAX_POWER_DBM);

        // Initial delay for system initialization (~4 symbol periods)
        Cpu.addTimerDelay(4);
        Cpu.startTimer();

        // ========================================================================
        // CRITICAL TIMING SECTION
        // ========================================================================
        // Using XTAL-based 20 MHz timer for precision (50 ns resolution)
        // No interrupt blocking needed - timer provides accurate timing
        
        for (uint16_t burstIdx = 0; burstIdx < numTxBursts; ++burstIdx) {
            // Handle zero-length bursts (gaps)
            if (Bursts[burstIdx].getBurstLength() == 0) {
                Cpu.waitTimer();
                if (burstIdx + 1 < numTxBursts) {
                    Cpu.addTimerDelay((int16_t)Bursts[burstIdx].get_T_RB() - 
                                     Bursts[burstIdx].getBurstLength());
                }
                continue;
            }

            // Calculate burst frequency with carrier offset
            // carrierOffset is number of carriers * RFM69 B_c register value
            // But B_c was chosen to equal carrier_spacing in Hz / RFM69_step
            // So: actual_offset_Hz = (carrierOffset_reg * RFM69_step)
            // To be hardware-independent, derive from regional profile:
            const int32_t carrierOffsetReg = Bursts[burstIdx].getCarrierOffset();
            
            // Convert RFM69 register offset to actual Hz using carrier spacing
            // carrierOffsetReg = carrier_index * B_c (where B_c ≈ CARRIER_SPACING_HZ / RFM69_FREQ_STEP_HZ)
            double carrierOffsetHz = carrierOffsetReg * RFM69_FREQ_STEP_HZ;
            double burstFreqMHz = (baseFreqHz + carrierOffsetHz) / 1000000.0;

            // Wait for burst timing
            Cpu.waitTimer();

            // Set burst frequency
            radio.setFrequency(burstFreqMHz);

            // Prepare data buffer
            uint8_t burstData[256];
            const uint8_t* srcData = Bursts[burstIdx].getBurst();
            uint16_t burstLengthBytes = Bursts[burstIdx].getBurstLengthBytes();
            
            for (uint16_t i = 0; i < burstLengthBytes && i < 256; ++i) {
                burstData[i] = srcData[i];
            }

            // Small delay before transmission
            Cpu.addTimerDelay(2);
            Cpu.waitTimer();

            // Transmit burst (blocking - critical timing!)
            radio.transmit(burstData, burstLengthBytes);

            // Wait for burst duration
            Cpu.addTimerDelay(Bursts[burstIdx].getBurstLength());
            Cpu.waitTimer();

            // Return to standby
            radio.standby();

            // Schedule next burst (wake up 2 symbols early)
            if (burstIdx + 1 < numTxBursts) {
                Cpu.addTimerDelay((int16_t)Bursts[burstIdx].get_T_RB() - 
                                 Bursts[burstIdx].getBurstLength() - 2);
            }
        }
        
        // ========================================================================
        // END CRITICAL TIMING SECTION
        // ========================================================================

        // Cleanup after all bursts complete
        radio.standby();
        Cpu.stopTimer();
        Cpu.spiDeinit();

        return 0;
    }

private:
    Module* module;       //!< RadioLib Module instance
    SX1262 radio;         //!< RadioLib SX1262 instance
    int8_t nssPin;        //!< NSS (chip select) pin
    int8_t dio1Pin;       //!< DIO1 interrupt pin
    int8_t resetPin;      //!< RESET pin
    int8_t busyPin;       //!< BUSY pin
    uint8_t currentChannel; //!< Current channel (0=A, 1=B) for dual-channel hopping
};

} // namespace Drivers
} // namespace TsUnbLib

#endif // SX1262_RADIOLIB_H_
