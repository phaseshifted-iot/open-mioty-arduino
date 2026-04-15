/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

OpenMioty - Open-source mioty® protocol library for Arduino

⚠️ IMPORTANT DISCLAIMERS - READ BEFORE USE ⚠️

Third-Party Modified Version of the Fraunhofer TS-UNB-Lib - NOT PRODUCTION SOFTWARE

Status:
  • For testing, demonstration, and learning purposes ONLY
  • NOT production-ready or commercially licensed
  • NOT certified or validated for regulatory compliance
  • NOT affiliated with or endorsed by Fraunhofer IIS or Sisvel

Legal Requirements:
  • Commercial use requires patent licenses from Sisvel International S.A.
  • You are responsible for regulatory compliance (FCC, ETSI, etc.)
  • Use at your own risk - no warranties provided

For production deployment:
  • Contact Sisvel for patent licensing
  • Contact Fraunhofer IIS for commercial-grade mioty® software

Modified for Arduino platforms (ESP32, RP2040, ATmega, and more)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @mainpage OpenMioty Library
 * 
 * @section intro Introduction
 * 
 * OpenMioty implements the ETSI TS 103 357 TS-UNB standard (mioty®)
 * for Arduino-compatible platforms with a clean, modular architecture.
 * 
 * @section features Features
 * 
 * - Platform-independent: ESP32, RP2040, ATmega328p support
 * - Multiple radio chips: SX1262 (RadioLib), RFM69HW (planned)
 * - Uplink transmission only (standard mioty® mode)
 * - Supports EU, US, and IN frequency bands
 * - Based on Fraunhofer TS-UNB-Lib reference implementation
 * - Clean layered architecture for easy porting
 * - Hardware timer for precise symbol timing
 * 
 * @section license License
 * 
 * For academic and non-commercial use only.
 * Commercial use requires patent licenses from Sisvel International S.A.
 * Third-Party Modified Version of the Fraunhofer TS-UNB-Lib, NOT production software.
 * See LICENSE.md and NOTICE.md for complete information.
 * 
 * @section usage Basic Usage
 * 
 * @code
 * #include <OpenMioty.h>
 * 
 * // Simple one-liner - board auto-detected, just pick your region!
 * TsUnb::EU1 node;
 * 
 * void setup() {
 *   node.init();
 * }
 * 
 * void loop() {
 *   uint8_t data[] = "Hello mioty";
 *   node.send(data, sizeof(data));
 *   delay(60000); // Wait 60s (respect duty cycle)
 * }
 * @endcode
 * 
 * Available regions: EU0, EU1, EU2, US0, IN0
 * 
 * @file OpenMioty.h
 */

#ifndef OPEN_MIOTY_H_
#define OPEN_MIOTY_H_

// Version information
#define OPEN_MIOTY_VERSION "0.1.1"
#define OPEN_MIOTY_VERSION_MAJOR 0
#define OPEN_MIOTY_VERSION_MINOR 1
#define OPEN_MIOTY_VERSION_PATCH 1

// Core TS-UNB protocol (order matters - dependencies first)
#include "core/Utils/BitAccess.h"
#include "core/Encryption/Aes128.h"
#include "core/TsUnb/RadioBurst.h"
#include "core/TsUnb/FixedMac.h"
#include "core/TsUnb/Phy.h"
#include "core/TsUnb/SimpleNode.h"

// Platform layer (microcontroller-specific)
#include "platform/esp32/ESP32TsUnb.h"
#include "platform/esp32/ESP32Utils.h"

// Radio drivers
#include "drivers/SX1262RadioLib.h"

// Regional profiles (frequency bands, power limits)
#include "regional/RegionalProfiles.h"

// Board-specific templates and helpers (under platform/<mcu>/<board>/)
#include "platform/esp32/nesso-n1/BoardConfig.h"
#include "platform/esp32/nesso-n1/Templates.h"
#include "platform/esp32/nesso-n1/NessoN1Helper.h"

// ============================================================================
// CONVENIENCE NAMESPACE - Simple API for users
// ============================================================================

/**
 * @brief Simplified namespace for easy board usage
 * 
 * Instead of: TsUnbLib::NessoN1::TsUnb_EU1 node;
 * Just use:   TsUnb::EU1 node;
 * 
 * The library automatically uses the correct board configuration.
 */
namespace TsUnb {
    // Arduino Nesso N1 templates (default for now)
    using EU0 = TsUnbLib::NessoN1::TsUnb_EU0;
    using EU1 = TsUnbLib::NessoN1::TsUnb_EU1;
    using EU2 = TsUnbLib::NessoN1::TsUnb_EU2;
    using US0 = TsUnbLib::NessoN1::TsUnb_US0;
    using IN0 = TsUnbLib::NessoN1::TsUnb_IN0;
}

/**
 * @namespace TsUnbLib
 * @brief Main namespace for TS-UNB library
 */

/**
 * @namespace TsUnbLib::TsUnb
 * @brief Core TS-UNB protocol implementation (from Fraunhofer)
 */

/**
 * @namespace TsUnbLib::ESP32
 * @brief ESP32 platform abstraction layer
 */

/**
 * @namespace TsUnbLib::Drivers
 * @brief Radio driver implementations
 */

/**
 * @namespace TsUnbLib::Regional
 * @brief Regional frequency profiles
 */

/**
 * @namespace TsUnbLib::NessoN1
 * @brief Pre-configured templates for Arduino Nesso N1
 */

#endif // OPEN_MIOTY_H_
