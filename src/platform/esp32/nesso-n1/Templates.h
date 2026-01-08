/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief Pre-configured TS-UNB node templates for Arduino Nesso N1
 *
 * This file combines the board configuration, platform layer, radio driver,
 * and regional profiles into ready-to-use TS-UNB node configurations.
 *
 * ARCHITECTURE LAYERS:
 * ===================
 * 
 * 1. PLATFORM LAYER (ESP32TsUnb):
 *    - MCU-specific implementation (ESP32-C6, RP2040, ATmega, etc.)
 *    - Defines: SPI implementation, hardware timers, clock speeds
 *    - Receives: SPI pins and clock configuration from board layer
 * 
 * 2. BOARD LAYER (BoardConfig):
 *    - Board-specific pin mappings and hardware configuration
 *    - Defines: Which GPIO pins connect to radio (CS, DIO1, BUSY, RESET)
 *    - Defines: SPI pin configuration (SCK, MISO, MOSI for shared buses)
 *    - Example boards on ESP32 platform: Nesso N1, custom boards, etc.
 *    - Example boards on RP2040 platform: HopeRF board, Pico W, etc.
 * 
 * 3. RADIO DRIVER LAYER (SX1262RadioLib, RFM69HW, etc.):
 *    - Radio chip implementation
 *    - Receives: Pin configuration from board layer via template parameters
 *    - Receives: Regional settings (TX power) from regional profiles
 *    - Different boards may use different radios (SX1262, RFM69, etc.)
 * 
 * 4. REGIONAL LAYER (RegionalProfiles):
 *    - Region-specific frequency and power settings
 *    - Defines: TX power limits, frequency bands, carrier spacing
 *    - Examples: EU (14 dBm @ 868 MHz), US (22 dBm @ 915 MHz), etc.
 * 
 * DATA FLOW EXAMPLE (Nesso N1 with EU1):
 * ======================================
 * Regional::EU1_Profile (14 dBm, 868 MHz)
 *           ↓
 * Board::RADIO_CS=23, RADIO_DIO1=15, RADIO_BUSY=19, RADIO_RESET=-2
 *           ↓
 * SX1262RadioLib<pins from board, power from region>
 *           ↓
 * ESP32TsUnb<SPI pins from board, clock config from board>
 *           ↓
 * Hardware: ESP32-C6 SPI communicates with SX1262 radio
 * 
 * ADDING A NEW BOARD:
 * ==================
 * 1. Create new BoardConfig.h in src/platform/<platform>/<board-name>/
 * 2. Define all pin mappings (RADIO_CS, RADIO_DIO1, etc.)
 * 3. Define SPI configuration (SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CLOCK_HZ)
 * 4. Create Templates.h using the pattern below
 * 5. Select appropriate radio driver (SX1262RadioLib, RFM69HW, etc.)
 *
 * @file Templates.h
 */

#ifndef NESSON1_TEMPLATES_H_
#define NESSON1_TEMPLATES_H_

#include "../ESP32TsUnb.h"
#include "BoardConfig.h"
#include "../../../drivers/SX1262RadioLib.h"
#include "../../../core/TsUnb/SimpleNode.h"
#include "../../../core/TsUnb/FixedMac.h"
#include "../../../core/TsUnb/Phy.h"
#include "../../../core/TsUnb/RadioBurst.h"
#include "../../../regional/RegionalProfiles.h"

namespace TsUnbLib {
namespace NessoN1 {

// Import board configuration
using Board = BoardConfig;

// ============================================================================
// EUROPE (EU) TEMPLATES
// ============================================================================

/**
 * @brief EU0 configuration for Arduino Nesso N1
 * 
 * - Single channel at 868.18 MHz
 * - Standard mode (UPG1)
 * - 14 dBm transmit power
 * 
 * Usage:
 * @code
 * TsUnbLib::NessoN1::TsUnb_EU0 node;
 * node.init();
 * uint8_t data[] = "Hello mioty";
 * node.send(data, sizeof(data));
 * @endcode
 */
using TsUnb_EU0 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<Regional::EU0_Profile::FREQ_CH_A_REG, 
               Regional::EU0_Profile::FREQ_CH_B_REG,
               Regional::EU0_Profile::B_c_REG,
               Regional::EU0_Profile::B_c0_REG,
               TsUnb::TsUnb_UPG1,
               3,
               TsUnb::RadioBurst<2, 2>>,
    Drivers::SX1262RadioLib<ESP32::ESP32TsUnb<Regional::EU0_Profile,
                                              Board::RADIO_CS,
                                              Board::TIMING_OFFSET_PPM,
                                              Board::SPI_CLOCK_HZ,
                                              Board::SPI_SCK,
                                              Board::SPI_MISO,
                                              Board::SPI_MOSI>,
                            Regional::EU0_Profile,
                            Board::RADIO_CS,
                            Board::RADIO_DIO1,
                            Board::RADIO_RESET,
                            Board::RADIO_BUSY,
                            Board::ANTENNA_LOSS_DBM,
                            TsUnb::RadioBurst<2, 2>>,
    false>;

/**
 * @brief EU1 configuration for Arduino Nesso N1
 * 
 * - Dual channel (868.18 MHz and 868.08 MHz)
 * - Standard mode (UPG1)
 * - 14 dBm transmit power
 */
using TsUnb_EU1 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<Regional::EU1_Profile::FREQ_CH_A_REG,
               Regional::EU1_Profile::FREQ_CH_B_REG,
               Regional::EU1_Profile::B_c_REG,
               Regional::EU1_Profile::B_c0_REG,
               TsUnb::TsUnb_UPG1,
               3,
               TsUnb::RadioBurst<2, 2>>,
    Drivers::SX1262RadioLib<ESP32::ESP32TsUnb<Regional::EU1_Profile,
                                              Board::RADIO_CS,
                                              Board::TIMING_OFFSET_PPM,
                                              Board::SPI_CLOCK_HZ,
                                              Board::SPI_SCK,
                                              Board::SPI_MISO,
                                              Board::SPI_MOSI>,
                            Regional::EU1_Profile,
                            Board::RADIO_CS,
                            Board::RADIO_DIO1,
                            Board::RADIO_RESET,
                            Board::RADIO_BUSY,
                            Board::ANTENNA_LOSS_DBM,
                            TsUnb::RadioBurst<2, 2>>,
    false>;

/**
 * @brief EU2 configuration for Arduino Nesso N1
 * 
 * - Dual channel (867.625 MHz and 866.825 MHz)
 * - Wide TSMA mode (UPG1 with 28.6 kHz carrier spacing)
 * - 14 dBm transmit power
 */
using TsUnb_EU2 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<Regional::EU2_Profile::FREQ_CH_A_REG,
               Regional::EU2_Profile::FREQ_CH_B_REG,
               Regional::EU2_Profile::B_c_REG,
               Regional::EU2_Profile::B_c0_REG,
               TsUnb::TsUnb_UPG1,
               3,
               TsUnb::RadioBurst<2, 2>>,
    Drivers::SX1262RadioLib<ESP32::ESP32TsUnb<Regional::EU2_Profile,
                                              Board::RADIO_CS,
                                              Board::TIMING_OFFSET_PPM,
                                              Board::SPI_CLOCK_HZ,
                                              Board::SPI_SCK,
                                              Board::SPI_MISO,
                                              Board::SPI_MOSI>,
                            Regional::EU2_Profile,
                            Board::RADIO_CS,
                            Board::RADIO_DIO1,
                            Board::RADIO_RESET,
                            Board::RADIO_BUSY,
                            Board::ANTENNA_LOSS_DBM,
                            TsUnb::RadioBurst<2, 2>>,
    false>;

// ============================================================================
// UNITED STATES (US) TEMPLATES
// ============================================================================

/**
 * @brief US0 configuration for Arduino Nesso N1
 * 
 * - Dual channel (916.4 MHz and 915.6 MHz)
 * - Wide TSMA mode (UPG1 with 28.6 kHz carrier spacing)
 * - 22 dBm transmit power (max for SX1262)
 */
using TsUnb_US0 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<Regional::US0_Profile::FREQ_CH_A_REG,
               Regional::US0_Profile::FREQ_CH_B_REG,
               Regional::US0_Profile::B_c_REG,
               Regional::US0_Profile::B_c0_REG,
               TsUnb::TsUnb_UPG1,
               3,
               TsUnb::RadioBurst<2, 2>>,
    Drivers::SX1262RadioLib<ESP32::ESP32TsUnb<Regional::US0_Profile,
                                              Board::RADIO_CS,
                                              Board::TIMING_OFFSET_PPM,
                                              Board::SPI_CLOCK_HZ,
                                              Board::SPI_SCK,
                                              Board::SPI_MISO,
                                              Board::SPI_MOSI>,
                            Regional::US0_Profile,
                            Board::RADIO_CS,
                            Board::RADIO_DIO1,
                            Board::RADIO_RESET,
                            Board::RADIO_BUSY,
                            Board::ANTENNA_LOSS_DBM,
                            TsUnb::RadioBurst<2, 2>>,
    false>;

// ============================================================================
// INDIA (IN) TEMPLATES
// ============================================================================

/**
 * @brief IN0 configuration for Arduino Nesso N1
 * 
 * - Dual channel (866.875 MHz and 866.775 MHz)
 * - Standard mode (UPG1)
 * - 22 dBm transmit power
 */
using TsUnb_IN0 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<Regional::IN0_Profile::FREQ_CH_A_REG,
               Regional::IN0_Profile::FREQ_CH_B_REG,
               Regional::IN0_Profile::B_c_REG,
               Regional::IN0_Profile::B_c0_REG,
               TsUnb::TsUnb_UPG1,
               3,
               TsUnb::RadioBurst<2, 2>>,
    Drivers::SX1262RadioLib<ESP32::ESP32TsUnb<Regional::IN0_Profile,
                                              Board::RADIO_CS,
                                              Board::TIMING_OFFSET_PPM,
                                              Board::SPI_CLOCK_HZ,
                                              Board::SPI_SCK,
                                              Board::SPI_MISO,
                                              Board::SPI_MOSI>,
                            Regional::IN0_Profile,
                            Board::RADIO_CS,
                            Board::RADIO_DIO1,
                            Board::RADIO_RESET,
                            Board::RADIO_BUSY,
                            Board::ANTENNA_LOSS_DBM,
                            TsUnb::RadioBurst<2, 2>>,
    false>;

} // namespace NessoN1
} // namespace TsUnbLib

#endif // NESSON1_TEMPLATES_H_
