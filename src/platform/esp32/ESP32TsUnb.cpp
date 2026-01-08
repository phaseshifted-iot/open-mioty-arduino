/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief TS-UNB platform implementation for ESP32-C6
 *
 * @authors open-mioty-arduino Contributors
 * @file ESP32TsUnb.cpp
 */

#include "ESP32TsUnb.h"

namespace TsUnbLib {
namespace ESP32 {

// Global timer flag set by ISR
volatile bool TsUnbTimerFlag = false;

} // namespace ESP32
} // namespace TsUnbLib
