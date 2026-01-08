/* -----------------------------------------------------------------------------
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

Modified for Arduino Nesso N1 (ESP32-C6 + SX1262)
Modification Date: January 2026

Original Copyright (c) 2019 - 2023 Fraunhofer-Gesellschaft
See LICENSE.md for complete license information
----------------------------------------------------------------------------- */

/**
 * @brief Utility functions for ESP32 platform
 *
 * @authors open-mioty-arduino Contributors
 * @file ESP32Utils.h
 */

#ifndef ESP32_UTILS_H_
#define ESP32_UTILS_H_

#include <Arduino.h>
#include <esp_mac.h>

namespace TsUnbLib {
namespace ESP32 {

/**
 * @brief Read ESP32 MAC address as EUI64
 * 
 * Reads the ESP32 WiFi station MAC address to use as the device EUI64.
 * This provides a globally unique identifier for each device.
 * 
 * @param eui64 Output buffer (must be at least 8 bytes)
 */
inline void getEui64(uint8_t* eui64) {
    esp_read_mac(eui64, ESP_MAC_WIFI_STA);
}

/**
 * @brief Initialize LORA_ENABLE pin on Arduino Nesso N1
 * 
 * The LORA_ENABLE signal controls power to the SX1262 radio module.
 * On Arduino Nesso N1, this is GPIO pin that must be HIGH to enable the radio.
 * 
 * This function should be called in setup() before initializing TS-UNB.
 * 
 * @note Requires Arduino Nesso N1 board support package
 */
inline void enableRadioModule() {
    #ifdef LORA_ENABLE
    pinMode(LORA_ENABLE, OUTPUT);
    digitalWrite(LORA_ENABLE, LOW);   // Reset
    delay(50);
    digitalWrite(LORA_ENABLE, HIGH);  // Enable (must stay HIGH)
    delay(200);  // Wait for SX1262 power-up and stabilization
    Serial.println("[Radio] LORA_ENABLE: HIGH (module powered)");
    #else
    // LORA_ENABLE not defined - radio is always powered on this board
    #endif
}

/**
 * @brief Put ESP32 into light sleep mode
 * 
 * Can be used between transmissions to save power
 * 
 * @param duration_ms Sleep duration in milliseconds
 */
inline void sleepMCU(uint32_t duration_ms) {
    if (duration_ms > 0) {
        delay(duration_ms);
        // For deeper sleep, could use esp_light_sleep_start()
        // but requires more configuration
    }
}

/**
 * @brief Calculate frequency register value for SX1262
 * 
 * SX1262 uses 32 MHz crystal with PLL
 * Frequency resolution: 32 MHz / 2^25 = 0.9536743164 Hz
 * 
 * @param frequency_hz Desired frequency in Hz
 * @return 32-bit register value
 */
inline uint32_t calculateFrequencyRegister(uint32_t frequency_hz) {
    // SX1262: FREQ = (FXTAL * regValue) / 2^25
    // regValue = (FREQ * 2^25) / FXTAL
    // With FXTAL = 32 MHz
    const double FREQ_STEP = 32000000.0 / 33554432.0; // 32MHz / 2^25
    return (uint32_t)(frequency_hz / FREQ_STEP);
}

/**
 * @brief Convert Hz to SX1262 frequency register value
 * 
 * For compatibility with Fraunhofer templates that use RFM69 register values,
 * this function converts between the two radio chips.
 * 
 * @param rfm69_register RFM69 frequency register value
 * @return Equivalent SX1262 frequency register value
 */
inline uint32_t rfm69ToSx1262Frequency(uint32_t rfm69_register) {
    // RFM69: Fstep = 61.03515625 Hz (32 MHz / 2^19)
    // SX1262: Fstep = 0.9536743164 Hz (32 MHz / 2^25)
    
    // First convert RFM69 register to actual frequency
    const double RFM69_FSTEP = 61.03515625;
    double frequency_hz = rfm69_register * RFM69_FSTEP;
    
    // Then convert to SX1262 register value
    return calculateFrequencyRegister((uint32_t)frequency_hz);
}

/**
 * @brief Simple deferred task queue for mioty-aware cooperative scheduling
 * 
 * This queue allows interrupt handlers and user code to defer work until
 * after mioty transmission completes. This is CRITICAL because mioty requires
 * <20 ppm timing precision - any interruption can break transmission.
 * 
 * Usage Pattern:
 * ```cpp
 * // Global queue instance
 * TsUnbLib::ESP32::DeferredTaskQueue taskQueue;
 * TsUnbLib::NessoN1::TsUnb_EU1 miotyNode;
 * 
 * // Button ISR - defer work instead of doing it immediately
 * void IRAM_ATTR buttonISR() {
 *     taskQueue.schedule(handleButtonPress);  // Just queue it
 * }
 * 
 * // Actual button handler (runs after transmission)
 * void handleButtonPress() {
 *     Serial.println("Button pressed!");
 *     // ... do slow work here ...
 * }
 * 
 * void loop() {
 *     // Check if we can execute deferred tasks
 *     if (!miotyNode.Tx.Cpu.isTransmitting()) {
 *         taskQueue.executePending();
 *     }
 *     
 *     // Transmit
 *     miotyNode.send(data, len);
 *     
 *     // Execute any tasks queued during transmission
 *     taskQueue.executePending();
 * }
 * ```
 * 
 * @note This is a simple circular buffer - if full, oldest tasks are dropped
 */
class DeferredTaskQueue {
private:
    static constexpr uint8_t QUEUE_SIZE = 16;
    
    typedef void (*TaskCallback)();
    TaskCallback queue[QUEUE_SIZE];
    volatile uint8_t writeIdx;
    volatile uint8_t readIdx;
    
public:
    /**
     * @brief Constructor - initialize empty queue
     */
    DeferredTaskQueue() : writeIdx(0), readIdx(0) {
        for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
            queue[i] = nullptr;
        }
    }
    
    /**
     * @brief Schedule a task to run later
     * 
     * Safe to call from ISR context. If queue is full, oldest task is dropped.
     * 
     * @param callback Function pointer to execute later (must not be nullptr)
     */
    void IRAM_ATTR schedule(TaskCallback callback) {
        if (callback == nullptr) return;
        
        // Add to queue
        queue[writeIdx] = callback;
        writeIdx = (writeIdx + 1) % QUEUE_SIZE;
        
        // If queue full, advance read index (drop oldest)
        if (writeIdx == readIdx) {
            readIdx = (readIdx + 1) % QUEUE_SIZE;
        }
    }
    
    /**
     * @brief Execute all pending tasks
     * 
     * NOT safe from ISR - only call from normal context (loop, setup, etc.)
     * Executes all queued callbacks in FIFO order, then clears the queue.
     */
    void executePending() {
        while (readIdx != writeIdx) {
            TaskCallback callback = queue[readIdx];
            queue[readIdx] = nullptr;
            readIdx = (readIdx + 1) % QUEUE_SIZE;
            
            if (callback != nullptr) {
                callback();  // Execute the deferred task
            }
        }
    }
    
    /**
     * @brief Check if any tasks are pending
     */
    bool hasPending() const {
        return readIdx != writeIdx;
    }
    
    /**
     * @brief Clear all pending tasks without executing them
     */
    void clear() {
        readIdx = writeIdx;
        for (uint8_t i = 0; i < QUEUE_SIZE; i++) {
            queue[i] = nullptr;
        }
    }
};

} // namespace ESP32
} // namespace TsUnbLib

#endif // ESP32_UTILS_H_
