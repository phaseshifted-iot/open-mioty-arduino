/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief Platform abstraction interface for TS-UNB implementations
 * 
 * This interface defines the platform-specific methods required by TS-UNB.
 * Each platform (ESP32, STM32, nRF52, etc.) must implement these methods.
 *
 * @file PlatformInterface.h
 */

#ifndef PLATFORM_INTERFACE_H_
#define PLATFORM_INTERFACE_H_

#include <stdint.h>

namespace TsUnbLib {
namespace Platform {

/**
 * @brief Platform abstraction interface
 * 
 * This class defines the interface that all platform implementations must provide.
 * It includes timer control, SPI communication, and watchdog management.
 */
class PlatformInterface {
public:
    virtual ~PlatformInterface() {}

    // Timer control methods
    virtual void initTimer() = 0;
    virtual void startTimer() = 0;
    virtual void stopTimer() = 0;
    virtual void addTimerDelay(const int32_t count) = 0;
    virtual void waitTimer() const = 0;

    // SPI communication methods
    virtual void spiInit() = 0;
    virtual void spiDeinit() = 0;
    virtual void spiSend(const uint8_t* const dataOut, const uint8_t numBytes) = 0;
    virtual void spiSendReceive(uint8_t* const dataInOut, const uint8_t numBytes) = 0;

    // Watchdog/utility methods
    virtual void resetWatchdog() = 0;
};

} // namespace Platform
} // namespace TsUnbLib

#endif // PLATFORM_INTERFACE_H_
