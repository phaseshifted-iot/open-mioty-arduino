/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief Regional frequency profiles and mioty parameters
 *
 * Based on MIOTY Regional Radio Profiles v1.1.4
 * Frequencies are CENTER FREQUENCIES of 100 kHz channels
 *
 * @file RegionalProfiles.h
 */

#ifndef REGIONAL_PROFILES_H_
#define REGIONAL_PROFILES_H_

#include <stdint.h>

namespace TsUnbLib {
namespace Regional {

// ============================================================================
// MIOTY SPECIFICATION - ETSI TS 103 357
// ============================================================================

/**
 * @brief Core mioty parameters from ETSI TS 103 357 specification
 * 
 * These are the fundamental parameters that ALL implementations must use.
 * Platform and driver layers derive their values from these constants.
 */
struct MiotySpec {
    // Physical layer parameters (Section 6.4)
    static constexpr float    SYMBOL_RATE_HZ = 2380.371f;     // symbols/second (exact)
    static constexpr float    BIT_RATE_KBPS = 2.380371f;      // kbps (for FSK modems)
    static constexpr float    CARRIER_SPACING_HZ = 2380.371f; // Hz (equals symbol rate)
    static constexpr float    FREQ_DEVIATION_HZ = 595.09275f; // Hz (SymbolRate / 4)
    static constexpr uint8_t  CARRIERS_PER_CHANNEL = 24;      // Sub-carriers
    static constexpr uint8_t  SYMBOLS_PER_BURST = 36;         // 24 data + 12 midamble
    
    // Derived values
    static constexpr float    SYMBOL_PERIOD_US = 1000000.0f / SYMBOL_RATE_HZ; // 420.168 µs
    static constexpr uint32_t OCCUPIED_BW_HZ = 57129;         // ~57 kHz actual
    static constexpr uint32_t CHANNEL_BW_HZ = 100000;         // 100 kHz allocation
    
    // Hardware-specific conversions (for driver layer)
    // RFM69 uses Fstep = 61.03515625 Hz (32 MHz / 2^19)
    static constexpr float    RFM69_FREQ_STEP_HZ = 61.03515625f;
    
    // Carrier spacing in RFM69 register units (used by Phy.h from Fraunhofer)
    // Standard TSMA mode: B_c = CARRIER_SPACING_HZ / RFM69_FREQ_STEP_HZ = 2380.371 / 61.03515625 = 39
    static constexpr uint32_t B_c_RFM69 = 39;
    static constexpr uint32_t B_c0_RFM69 = 39;  // Same as B_c for standard mode
    
    // Wide TSMA mode: B_c = 28600.0 / 61.03515625 = 469 (approximately)
    static constexpr uint32_t B_c_WIDE_RFM69 = 469;
    static constexpr uint32_t B_c0_WIDE_RFM69 = 469;  // Same as B_c for wide mode
    
    /**
     * @brief Convert frequency from Hz to RFM69 register units
     * @param freq_hz Frequency in Hz
     * @return Frequency in RFM69 register units (constexpr for compile-time evaluation)
     */
    static constexpr uint32_t toRFM69Reg(uint32_t freq_hz) {
        return static_cast<uint32_t>(freq_hz / RFM69_FREQ_STEP_HZ);
    }
};

// ============================================================================
// EUROPE (EU) - ETSI EN 300 220
// ============================================================================

/**
 * @brief EU0 Profile - Single channel at 868.18 MHz
 * 
 * - Center Frequency: 868.180 MHz
 * - Channel Bandwidth: 100 kHz (24 carriers × 2.38 kHz)
 * - Max Power: +14 dBm ERP
 * - Duty Cycle: <1% per hour
 */
struct EU0_Profile {
    // Frequencies in Hz (hardware-independent)
    static constexpr uint32_t FREQ_CH_A_HZ = 868180000;  // 868.180 MHz
    static constexpr uint32_t FREQ_CH_B_HZ = 868180000;  // Same as A (single channel)
    
    // RFM69 register units (for Phy template compatibility)
    static constexpr uint32_t FREQ_CH_A_REG = MiotySpec::toRFM69Reg(FREQ_CH_A_HZ);
    static constexpr uint32_t FREQ_CH_B_REG = MiotySpec::toRFM69Reg(FREQ_CH_B_HZ);
    
    // mioty parameters (from MiotySpec)
    static constexpr float    SYMBOL_RATE_HZ = MiotySpec::SYMBOL_RATE_HZ;
    static constexpr float    BIT_RATE_KBPS = MiotySpec::BIT_RATE_KBPS;
    static constexpr float    CARRIER_SPACING_HZ = MiotySpec::CARRIER_SPACING_HZ;
    static constexpr float    FREQ_DEVIATION_HZ = MiotySpec::FREQ_DEVIATION_HZ;
    
    // Regional regulatory limits
    static constexpr int8_t   MAX_POWER_DBM = 14;        // ETSI EN 300 220
    static constexpr float    DUTY_CYCLE = 0.01;         // 1% per hour
    static constexpr const char* NAME = "EU0";
    
    // TSMA mode constants (standard mode)
    static constexpr uint32_t B_c_REG = MiotySpec::B_c_RFM69;
    static constexpr uint32_t B_c0_REG = MiotySpec::B_c0_RFM69;
};

/**
 * @brief EU1 Profile - Dual channel 100 kHz apart
 * 
 * - Channel A: 868.180 MHz
 * - Channel B: 868.080 MHz (100 kHz lower)
 * - Channel Bandwidth: 100 kHz each
 * - Max Power: +14 dBm ERP
 * - Duty Cycle: <1% per hour
 */
struct EU1_Profile {
    // Frequencies in Hz (hardware-independent)
    static constexpr uint32_t FREQ_CH_A_HZ = 868180000;  // 868.180 MHz
    static constexpr uint32_t FREQ_CH_B_HZ = 868080000;  // 868.080 MHz
    
    // RFM69 register units (for Phy template compatibility)
    static constexpr uint32_t FREQ_CH_A_REG = MiotySpec::toRFM69Reg(FREQ_CH_A_HZ);
    static constexpr uint32_t FREQ_CH_B_REG = MiotySpec::toRFM69Reg(FREQ_CH_B_HZ);
    
    // mioty parameters (from MiotySpec)
    static constexpr float    SYMBOL_RATE_HZ = MiotySpec::SYMBOL_RATE_HZ;
    static constexpr float    BIT_RATE_KBPS = MiotySpec::BIT_RATE_KBPS;
    static constexpr float    CARRIER_SPACING_HZ = MiotySpec::CARRIER_SPACING_HZ;
    static constexpr float    FREQ_DEVIATION_HZ = MiotySpec::FREQ_DEVIATION_HZ;
    
    // Regional regulatory limits
    static constexpr int8_t   MAX_POWER_DBM = 14;        // ETSI EN 300 220
    static constexpr float    DUTY_CYCLE = 0.01;         // 1% per hour
    static constexpr const char* NAME = "EU1";
    
    // TSMA mode constants (standard mode)
    static constexpr uint32_t B_c_REG = MiotySpec::B_c_RFM69;
    static constexpr uint32_t B_c0_REG = MiotySpec::B_c0_RFM69;
    
    // Carrier offset conversion from RFM69 register units to Hz
    static constexpr double carrierOffsetFromReg(int32_t offsetReg) {
        return (offsetReg * CARRIER_SPACING_HZ) / static_cast<double>(B_c_REG);
    }
};

/**
 * @brief EU2 Profile - Dual channel 800 kHz apart
 * 
 * - Channel A: 867.625 MHz
 * - Channel B: 866.825 MHz (800 kHz lower)
 * - Wider carrier spacing (28.6 kHz - special mode)
 * - Max Power: +14 dBm ERP
 * - Duty Cycle: <1% per hour
 */
struct EU2_Profile {
    // Frequencies in Hz (hardware-independent)
    static constexpr uint32_t FREQ_CH_A_HZ = 867625000;  // 867.625 MHz
    static constexpr uint32_t FREQ_CH_B_HZ = 866825000;  // 866.825 MHz
    
    // RFM69 register units (for Phy template compatibility)
    static constexpr uint32_t FREQ_CH_A_REG = MiotySpec::toRFM69Reg(FREQ_CH_A_HZ);
    static constexpr uint32_t FREQ_CH_B_REG = MiotySpec::toRFM69Reg(FREQ_CH_B_HZ);
    
    // EU2 uses wider carrier spacing (special mode)
    static constexpr float    SYMBOL_RATE_HZ = MiotySpec::SYMBOL_RATE_HZ;
    static constexpr float    BIT_RATE_KBPS = MiotySpec::BIT_RATE_KBPS;
    static constexpr float    CARRIER_SPACING_HZ = 28600.0f;  // 28.6 kHz (wider)
    static constexpr float    FREQ_DEVIATION_HZ = MiotySpec::FREQ_DEVIATION_HZ;
    
    // Regional regulatory limits
    static constexpr int8_t   MAX_POWER_DBM = 14;        // ETSI EN 300 220
    static constexpr float    DUTY_CYCLE = 0.01;         // 1% per hour
    static constexpr const char* NAME = "EU2";
    
    // TSMA mode constants (wide mode)
    static constexpr uint32_t B_c_REG = MiotySpec::B_c_WIDE_RFM69;
    static constexpr uint32_t B_c0_REG = MiotySpec::B_c0_WIDE_RFM69;
    
    // Carrier offset conversion from RFM69 register units to Hz
    static constexpr double carrierOffsetFromReg(int32_t offsetReg) {
        return (offsetReg * CARRIER_SPACING_HZ) / static_cast<double>(B_c_REG);
    }
};

// ============================================================================
// UNITED STATES (US) - FCC Part 15.247
// ============================================================================

/**
 * @brief US0 Profile - Dual channel with wide TSMA mode
 * 
 * - Channel A: 916.400 MHz
 * - Channel B: 915.600 MHz (800 kHz lower)
 * - Channel Bandwidth: 100 kHz each
 * - Wider carrier spacing (28.6 kHz - wide TSMA mode)
 * - Max Power: +30 dBm EIRP
 * - No duty cycle limit
 */
struct US0_Profile {
    // Frequencies in Hz (hardware-independent)
    static constexpr uint32_t FREQ_CH_A_HZ = 916400000;  // 916.400 MHz
    static constexpr uint32_t FREQ_CH_B_HZ = 915600000;  // 915.600 MHz
    
    // RFM69 register units (for Phy template compatibility)
    static constexpr uint32_t FREQ_CH_A_REG = MiotySpec::toRFM69Reg(FREQ_CH_A_HZ);
    static constexpr uint32_t FREQ_CH_B_REG = MiotySpec::toRFM69Reg(FREQ_CH_B_HZ);
    
    // US0 uses wider carrier spacing (wide TSMA mode)
    static constexpr float    SYMBOL_RATE_HZ = MiotySpec::SYMBOL_RATE_HZ;
    static constexpr float    BIT_RATE_KBPS = MiotySpec::BIT_RATE_KBPS;
    static constexpr float    CARRIER_SPACING_HZ = 28600.0f;  // 28.6 kHz (wider)
    static constexpr float    FREQ_DEVIATION_HZ = MiotySpec::FREQ_DEVIATION_HZ;
    
    // Regional regulatory limits
    static constexpr int8_t   MAX_POWER_DBM = 22;        // SX1262 max
    static constexpr float    DUTY_CYCLE = 1.0;          // No limit
    static constexpr const char* NAME = "US0";
    
    // TSMA mode constants (wide mode)
    static constexpr uint32_t B_c_REG = MiotySpec::B_c_WIDE_RFM69;
    static constexpr uint32_t B_c0_REG = MiotySpec::B_c0_WIDE_RFM69;
    
    // Carrier offset conversion from RFM69 register units to Hz
    static constexpr double carrierOffsetFromReg(int32_t offsetReg) {
        return (offsetReg * CARRIER_SPACING_HZ) / static_cast<double>(B_c_REG);
    }
};

// ============================================================================
// INDIA (IN) - WPC Guidelines
// ============================================================================

/**
 * @brief IN0 Profile - Dual channel (IN866 band)
 * 
 * - Channel A: 866.875 MHz
 * - Channel B: 866.775 MHz (100 kHz lower)
 * - Channel Bandwidth: 100 kHz each
 * - Max Power: +30 dBm EIRP
 * - No duty cycle limit
 */
struct IN0_Profile {
    // Frequencies in Hz (hardware-independent)
    static constexpr uint32_t FREQ_CH_A_HZ = 866875000;  // 866.875 MHz
    static constexpr uint32_t FREQ_CH_B_HZ = 866775000;  // 866.775 MHz
    
    // RFM69 register units (for Phy template compatibility)
    static constexpr uint32_t FREQ_CH_A_REG = MiotySpec::toRFM69Reg(FREQ_CH_A_HZ);
    static constexpr uint32_t FREQ_CH_B_REG = MiotySpec::toRFM69Reg(FREQ_CH_B_HZ);
    
    // mioty parameters (from MiotySpec)
    static constexpr float    SYMBOL_RATE_HZ = MiotySpec::SYMBOL_RATE_HZ;
    static constexpr float    BIT_RATE_KBPS = MiotySpec::BIT_RATE_KBPS;
    static constexpr float    CARRIER_SPACING_HZ = MiotySpec::CARRIER_SPACING_HZ;
    static constexpr float    FREQ_DEVIATION_HZ = MiotySpec::FREQ_DEVIATION_HZ;
    
    // Regional regulatory limits
    static constexpr int8_t   MAX_POWER_DBM = 22;        // SX1262 max
    static constexpr float    DUTY_CYCLE = 1.0;          // No limit
    static constexpr const char* NAME = "IN0";
    
    // TSMA mode constants (standard mode)
    static constexpr uint32_t B_c_REG = MiotySpec::B_c_RFM69;
    static constexpr uint32_t B_c0_REG = MiotySpec::B_c0_RFM69;
    
    // Carrier offset conversion from RFM69 register units to Hz
    static constexpr double carrierOffsetFromReg(int32_t offsetReg) {
        return (offsetReg * CARRIER_SPACING_HZ) / static_cast<double>(B_c_REG);
    }
};

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

/**
 * ARCHITECTURE:
 * 
 * 1. REGIONAL LAYER (this file):
 *    - Stores all parameters in Hz (hardware-independent)
 *    - Defines ETSI TS 103 357 specification values
 *    - Defines regional regulatory limits (power, duty cycle)
 * 
 * 2. PLATFORM LAYER (e.g., ESP32TsUnb.h):
 *    - Derives timer tick values from SYMBOL_RATE_HZ
 *    - Converts Hz to MCU-specific timer counts
 *    - Example: symbol_period_ticks = (timer_freq / SYMBOL_RATE_HZ)
 * 
 * 3. DRIVER LAYER (e.g., SX1262RadioLib.h):
 *    - Converts Hz to radio-specific register values
 *    - Example: freq_reg = (freq_hz * 2^25) / 32000000 for SX1262
 *    - Applies hardware-specific constraints (e.g., min freq deviation)
 * 
 * NO HARDCODED VALUES should exist outside this file except:
 *    - Hardware constants (e.g., MCU timer frequency, radio chip crystal)
 *    - Board-specific pins (defined in BoardConfig.h)
 */

} // namespace Regional
} // namespace TsUnbLib

#endif // REGIONAL_PROFILES_H_
