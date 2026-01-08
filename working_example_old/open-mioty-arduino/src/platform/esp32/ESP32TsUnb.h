/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief TS-UNB platform abstraction for ESP32-C6 (Arduino Nesso N1)
 *
 * This class implements the platform-specific functionality for ESP32-C6
 * including SPI communication and hardware timer for precise symbol timing.
 *
 * @authors open-mioty-arduino Contributors
 * @file ESP32TsUnb.h
 */

#ifndef ESP32_TSUNB_H_
#define ESP32_TSUNB_H_

#include <Arduino.h>
#include <SPI.h>
#include <driver/gptimer.h>
#include "../../regional/RegionalProfiles.h"
#include "../../core/TsUnb/RadioBurst.h"
#include "../../core/TsUnb/FixedMac.h"
#include "../../core/TsUnb/Phy.h"
#include "../../core/TsUnb/SimpleNode.h"

namespace TsUnbLib {
namespace ESP32 {

// Global timer flag (set by ISR)
extern volatile bool TsUnbTimerFlag;

/**
 * @brief Platform dependent TS-UNB implementation for ESP32-C6
 *
 * This class implements all platform dependent methods for TS-UNB on ESP32-C6.
 * It provides SPI communication and a hardware timer to generate the TS-UNB symbol clock.
 *
 * @tparam RegionalProfile_T Regional profile (EU1_Profile, US0_Profile, etc.) - provides SYMBOL_RATE_HZ
 * @tparam CS_PIN               SPI chip select pin
 * @tparam TIMING_OFFSET_PPM    Timing offset to correct crystal frequency (default 0)
 * @tparam SPI_CLOCK_HZ         SPI clock frequency in Hz (default 4 MHz)
 * @tparam SPI_SCK              SPI clock pin (board-specific)
 * @tparam SPI_MISO             SPI MISO pin (board-specific)
 * @tparam SPI_MOSI             SPI MOSI pin (board-specific)
 */
template <typename RegionalProfile_T,
          uint8_t CS_PIN = 5,
          int16_t TIMING_OFFSET_PPM = 0,
          uint32_t SPI_CLOCK_HZ = 4000000,
          int8_t SPI_SCK = -1,
          int8_t SPI_MISO = -1,
          int8_t SPI_MOSI = -1>
class ESP32TsUnb {
public:
    ESP32TsUnb() : preciseTsUnbTimer(0.0f), timerHandle(nullptr) {
    }

    ~ESP32TsUnb() {
        cleanupTimer();
    }

    /**
     * @brief Calculate bit/symbol duration in microseconds
     * 
     * Derived from regional profile's SYMBOL_RATE_HZ parameter.
     * For standard mioty mode: 2380.371 symbols/second → 420.168 microseconds
     * 
     * This is adjusted for crystal tolerance via TIMING_OFFSET_PPM.
     */
    static constexpr float TS_UNB_SYMBOL_DURATION_US = 
        (1000000.0f / RegionalProfile_T::SYMBOL_RATE_HZ) * (1.0 + 1.0e-6 * TIMING_OFFSET_PPM);

    /**
     * @brief Initialize the hardware timer
     * 
     * Creates and configures ESP32 general purpose timer for symbol timing.
     * Uses XTAL clock source (40 MHz external crystal) for maximum precision.
     * 
     * CRITICAL: mioty requires < 20 ppm timing accuracy.
     * XTAL provides stable 40 MHz reference directly from external crystal.
     * APB_CLK would have jitter from CPU clock variations.
     * 
     * NOTE: Timer is reused if already created to avoid resource exhaustion.
     */
    void initTimer() {
        // Only create timer if it doesn't exist yet
        if (timerHandle != nullptr) {
            return;
        }
        
        // Timer configuration - USE XTAL FOR PRECISION
        // XTAL = 40 MHz, but hardware requires divider >= 2
        // So max resolution = 20 MHz (50 ns per tick)
        gptimer_config_t timer_config = {
            .clk_src = GPTIMER_CLK_SRC_XTAL,  // 40 MHz external crystal
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 20000000, // 20 MHz (50 ns resolution, divider = 2)
        };
        
        ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timerHandle));
        
        // Alarm configuration
        gptimer_alarm_config_t alarm_config = {
            .alarm_count = 0,
            .reload_count = 0,
            .flags = {
                .auto_reload_on_alarm = false,
            }
        };
        ESP_ERROR_CHECK(gptimer_set_alarm_action(timerHandle, &alarm_config));
        
        // Event callback configuration
        gptimer_event_callbacks_t cbs = {
            .on_alarm = timerCallback,
        };
        ESP_ERROR_CHECK(gptimer_register_event_callbacks(timerHandle, &cbs, nullptr));
        
        // Enable timer
        ESP_ERROR_CHECK(gptimer_enable(timerHandle));
        
        preciseTsUnbTimer = 0.0f;
    }

    /**
     * @brief Start the timer
     */
    void startTimer() {
        TsUnbTimerFlag = false;
        ESP_ERROR_CHECK(gptimer_start(timerHandle));
    }

    /**
     * @brief Stop the timer to save energy
     * 
     * Stops the timer but keeps it allocated for reuse.
     */
    void stopTimer() {
        if (timerHandle != nullptr) {
            // Stop timer - ignore error if already stopped
            gptimer_stop(timerHandle);
        }
    }

    /**
     * @brief Cleanup timer and release hardware resources
     * 
     * Called by destructor. Fully releases the timer hardware.
     */
    void cleanupTimer() {
        if (timerHandle != nullptr) {
            // Stop timer (might already be stopped)
            gptimer_stop(timerHandle);
            
            // Disable timer (might already be disabled)
            esp_err_t err = gptimer_disable(timerHandle);
            if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
                // Delete timer
                err = gptimer_del_timer(timerHandle);
                if (err == ESP_OK) {
                    timerHandle = nullptr;
                }
            }
        }
    }

    /**
     * @brief Add the counter compare value for the next interrupt
     *
     * @param count Delay in TX symbols
     */
    void addTimerDelay(const int32_t count) {
        preciseTsUnbTimer += TS_UNB_SYMBOL_DURATION_US * count;
        
        // Convert microseconds to 20 MHz timer ticks (20 ticks per microsecond)
        uint64_t alarmValue = (uint64_t)((int64_t)(preciseTsUnbTimer * 20.0f + 0.5f));
        
        // Set alarm
        gptimer_alarm_config_t alarm_config = {
            .alarm_count = alarmValue,
            .reload_count = 0,
            .flags = {
                .auto_reload_on_alarm = false,
            }
        };
        ESP_ERROR_CHECK(gptimer_set_alarm_action(timerHandle, &alarm_config));
        
        // Handle timer wraparound (unlikely but safe)
        const uint64_t MAX_TIMER_VALUE = 0xFFFFFFFFFFFFULL; // 48-bit counter
        while (preciseTsUnbTimer >= MAX_TIMER_VALUE) {
            preciseTsUnbTimer -= MAX_TIMER_VALUE;
        }
    }

    /**
     * @brief Wait until the timer value expires
     */
    void waitTimer() const {
        while (!TsUnbTimerFlag) {
            // Yield to other tasks while waiting
            delay(0);
        }
        TsUnbTimerFlag = false;
    }

    /**
     * @brief Initialize SPI interface
     * 
     * Initializes SPI with board-specific pins configured via template parameters.
     * If SPI pins are set to -1, uses the default pins from the board's variant file.
     * 
     * @note CS pin is handled separately in spiSend/spiSendReceive functions
     */
    void spiInit() {
        // Initialize SPI with board-specific or default pins
        if (SPI_SCK != -1 && SPI_MISO != -1 && SPI_MOSI != -1) {
            // Use board-specific custom pins
            SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, -1);
        } else {
            // Use default pins from board variant
            SPI.begin();
        }
        
        // Configure CS pin manually (we handle it in spiSend/spiSendReceive)
        pinMode(CS_PIN, OUTPUT);
        digitalWrite(CS_PIN, HIGH);
    }

    /**
     * @brief Deinitialize SPI interface
     */
    void spiDeinit() {
        pinMode(CS_PIN, INPUT);
        SPI.end();
    }

    /**
     * @brief Send multiple bytes via SPI
     * 
     * @param dataOut Bytes to be transmitted
     * @param numBytes Number of bytes to transmit
     */
    void spiSend(const uint8_t* const dataOut, const uint8_t numBytes) {
        SPI.beginTransaction(SPISettings(SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));
        digitalWrite(CS_PIN, LOW);
        
        for (uint8_t i = 0; i < numBytes; ++i) {
            SPI.transfer(dataOut[i]);
        }
        
        digitalWrite(CS_PIN, HIGH);
        SPI.endTransaction();
    }

    /**
     * @brief Send and receive multiple bytes via SPI
     *
     * Note: Read data has a delay of one byte. First returned byte usually has no meaning.
     * 
     * @param dataInOut Bytes to transmit and buffer for received data
     * @param numBytes Number of bytes to exchange
     */
    void spiSendReceive(uint8_t* const dataInOut, const uint8_t numBytes) {
        SPI.beginTransaction(SPISettings(SPI_CLOCK_HZ, MSBFIRST, SPI_MODE0));
        digitalWrite(CS_PIN, LOW);
        
        for (uint8_t i = 0; i < numBytes; ++i) {
            dataInOut[i] = SPI.transfer(dataInOut[i]);
        }
        
        digitalWrite(CS_PIN, HIGH);
        SPI.endTransaction();
    }

    /**
     * @brief Reset watchdog timer (no-op for ESP32, handled by framework)
     */
    void resetWatchdog() {
        // ESP32 Arduino framework handles watchdog automatically
        // Can add explicit feed if needed: esp_task_wdt_reset();
    }

private:
    /**
     * @brief Timer callback function (ISR)
     */
    static bool IRAM_ATTR timerCallback(gptimer_handle_t timer, 
                                        const gptimer_alarm_event_data_t *edata, 
                                        void *user_ctx) {
        TsUnbTimerFlag = true;
        return false; // No need to yield
    }

    //! Precise state of TsUnb timer in microseconds
    float preciseTsUnbTimer;
    
    //! ESP32 timer handle
    gptimer_handle_t timerHandle;
};

} // namespace ESP32
} // namespace TsUnbLib

#endif // ESP32_TSUNB_H_
