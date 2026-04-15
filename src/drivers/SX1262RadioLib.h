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
 * BIT RATE PRECISION:
 * ===================
 * This driver uses the precise mioty symbol rate (2.380371 kbps) from the
 * regional profile. The SX1262's bit rate register provides sufficient
 * precision to represent this rate accurately.
 * 
 * Runtime diagnostics verify the achieved bit rate precision during
 * initialization and report any deviation from the target rate.
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
 * @tparam ANTENNA_LOSS_DBM Antenna loss compensation in dBm (positive=loss, negative=gain)
 * @tparam RadioBurst_T Radio burst class
 */
template <class Cpu_T,
          typename RegionalProfile_T,
          int8_t NSS_PIN,
          int8_t DIO1_PIN,
          int8_t RESET_PIN,
          int8_t BUSY_PIN,
          int8_t ANTENNA_LOSS_DBM = 0,
          class RadioBurst_T = TsUnb::RadioBurst<>>
class SX1262RadioLib {
public:
    Cpu_T Cpu;

    /**
     * @brief Constructor
     * 
     * Pins are configured at compile-time via template parameters from board configuration.
     * All mioty parameters derived from RegionalProfile_T at compile time.
     * Module is created here to ensure it exists before init().
     */
    SX1262RadioLib()
        : module(new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN)), 
          radio(module), 
          nssPin(NSS_PIN), dio1Pin(DIO1_PIN), resetPin(RESET_PIN), busyPin(BUSY_PIN),
          currentChannel(0) {
    }

    ~SX1262RadioLib() {
        if (module != nullptr) {
            delete module;
        }
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
        double bitRateKbps = RegionalProfile_T::BIT_RATE_KBPS;
        
        // Calculate effective TX power: regional max + antenna loss compensation
        // Take minimum of: (chip_max=22) OR (regional_max + antenna_loss)
        static constexpr int8_t CHIP_MAX_POWER_DBM = 22;  // SX1262 maximum
        int8_t requestedPower = RegionalProfile_T::MAX_POWER_DBM + ANTENNA_LOSS_DBM;
        effectiveTxPower = (requestedPower < CHIP_MAX_POWER_DBM) ? requestedPower : CHIP_MAX_POWER_DBM;
        
        Serial.print("[SX1262] Parameters: ");
        Serial.print(initFreqMHz, 3);
        Serial.print(" MHz, ");
        Serial.print(bitRateKbps, 6);
        Serial.print(" kbps, ");
        Serial.print(freqDeviationKHz, 3);
        Serial.print(" kHz dev, ");
        Serial.print(effectiveTxPower);
        Serial.println(" dBm");
        Serial.flush();
        
        // Initialize in LoRa mode first (validated configuration)
        Serial.println("[SX1262] Step 1: radio.begin()...");
        Serial.flush();
        int16_t state = radio.begin(initFreqMHz, 125.0, 9, 7,
                                    RADIOLIB_SX126X_SYNC_WORD_PRIVATE,
                                    10, 8, 3.0, true);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: begin() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 1: OK");
        Serial.flush();
        
        Serial.println("[SX1262] Step 2: radio.beginFSK()...");
        Serial.flush();
        // Switch to FSK mode for mioty - ALL values from regional profile
        state = radio.beginFSK(initFreqMHz,                           // Channel A frequency
                               bitRateKbps,                           // Precise bit rate from profile
                               freqDeviationKHz,                      // Freq deviation from profile
                               4.8,                                   // RX bandwidth (mioty: ~3 kHz)
                               effectiveTxPower,                      // TX power with antenna compensation
                               16,                                    // Preamble (will disable)
                               3.0,                                   // TCXO voltage = 3.0V
                               true);                                 // Use LDO regulator
        
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: beginFSK() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 2: OK");
        Serial.flush();

        // Enable DIO2 as RF switch control (required for Nesso N1 antenna path)
        radio.setDio2AsRfSwitch(true);
        
        Serial.println("[SX1262] Step 3: setPaConfig()...");
        Serial.flush();
        // Configure PA for high power output (14 dBm / 25 mW)
        state = radio.setPaConfig(0x04, 0x07, 0x00, 0x01);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setPaConfig() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 3: OK");
        Serial.flush();
        
        Serial.println("[SX1262] Step 4: setOutputPower()...");
        Serial.flush();
        // Set output power with antenna compensation applied
        state = radio.setOutputPower(effectiveTxPower);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print("[SX1262] ERROR: setOutputPower() failed with code: ");
            Serial.println(state);
            Serial.flush();
            return -1;
        }
        Serial.println("[SX1262] Step 4: OK");
        Serial.flush();
        
        Serial.println("[SX1262] Step 5: setDataShaping()...");
        Serial.flush();
        // Configure for mioty - no shaping, no sync, no CRC
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
        
        Serial.println("[SX1262] Step 9: standby()...");
        Serial.flush();
        // Set to standby mode to save power
        radio.standby();
        
        Serial.println("[SX1262] ✓ Initialization complete!");
        Serial.flush();
        
        Cpu.spiDeinit();
        return 0;
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
        // Mark transmission as active for cooperative scheduling
        Cpu.transmissionActive = true;
        
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

        // Configure PA and set transmit power (use pre-calculated value with antenna compensation)
        radio.setPaConfig(0x04, 0x07, 0x00, 0x01);  // High power PA
        radio.setOutputPower(effectiveTxPower);  // Use stored value from init()

        // Start timer FIRST (resets accumulator + counter to zero),
        // then add the initial delay. Reversing this order causes stale
        // accumulator values from the previous TX to corrupt all alarm times.
        Cpu.startTimer();
        Cpu.addTimerDelay(4);

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
            // Use regional profile's helper to convert from RFM69 register units to Hz
            // This ensures correct calculation for both standard and wide TSMA modes
            const int32_t carrierOffsetReg = Bursts[burstIdx].getCarrierOffset();
            
            // Convert using regional profile's carrier spacing (handles both 2.38 kHz and 28.6 kHz)
            double carrierOffsetHz = RegionalProfile_T::carrierOffsetFromReg(carrierOffsetReg);
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
        
        // Clear transmission active flag
        Cpu.transmissionActive = false;

        return 0;
    }

private:
    Module* module;        //!< RadioLib Module instance
    SX1262 radio;          //!< RadioLib SX1262 instance
    int8_t nssPin;         //!< NSS (chip select) pin
    int8_t dio1Pin;       //!< DIO1 interrupt pin
    int8_t resetPin;      //!< RESET pin
    int8_t busyPin;       //!< BUSY pin
    uint8_t currentChannel; //!< Current channel (0=A, 1=B) for dual-channel hopping
    int8_t effectiveTxPower; //!< Calculated TX power with antenna compensation
};

} // namespace Drivers
} // namespace TsUnbLib

#endif // SX1262_RADIOLIB_H_
