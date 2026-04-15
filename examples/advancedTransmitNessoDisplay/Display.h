/*
 * Display Library for Arduino Nesso N1
 * 
 * Two-state display for the mioty demo: idle screen and sending screen.
 * The display and SX1262 radio share a single SPI bus (SPI2/FSPI) with
 * separate chip-select pins (LCD_CS=GPIO17, LORA_CS=GPIO23).
 * 
 * IMPORTANT: Never call any display method while send() is in progress.
 * The transmit path owns the SPI bus for ~1.7 s.
 * 
 * Hardware: Arduino Nesso N1 (ESP32-C6 + ST7789 TFT)
 * Dependencies: M5GFX, Arduino_Nesso_N1
 * 
 * Part of the open-mioty-arduino library
 * For educational and demonstration purposes only
 */

#ifndef NESSON1_DISPLAY_H_
#define NESSON1_DISPLAY_H_

#include <Arduino_Nesso_N1.h>
#include "DisplayConfig.h"

namespace TsUnbLib {
namespace NessoN1 {

class Display {
private:
    NessoDisplay* tft;
    
public:
    Display() : tft(nullptr) {}
    
    void begin(uint8_t rotation = 1) {
        tft = new NessoDisplay();
        tft->begin();  // initializes LCD_RESET, LCD_BACKLIGHT via I/O expander
        tft->setRotation(rotation);
        tft->setTextDatum(MC_DATUM);
        tft->fillScreen(Colors::BLACK);
        ensureDeselected();
    }
    
    /**
     * @brief Idle screen: "mioty Demo" + counter value
     * 
     * Call AFTER send() returns (or at end of setup) to show the
     * resting state. Always call ensureDeselected() after this.
     */
    void showIdle(uint32_t counter) {
        if (!tft) return;
        
        tft->fillScreen(Colors::BLACK);
        
        tft->setTextSize(3);
        tft->setTextColor(Colors::MIOTY_RED, Colors::BLACK);
        tft->drawString("mioty Demo", tft->width() / 2, tft->height() / 2 - 20);
        
        tft->setTextSize(2);
        tft->setTextColor(Colors::WHITE, Colors::BLACK);
        String line = "counter: " + String(counter);
        tft->drawString(line, tft->width() / 2, tft->height() / 2 + 20);
        
        ensureDeselected();
    }
    
    /**
     * @brief Sending screen: "Sending message XX"
     * 
     * Call BEFORE send(). This is the last display operation before
     * the radio takes the SPI bus. Always call ensureDeselected() after.
     */
    void showSending(uint32_t counter) {
        if (!tft) return;
        
        tft->fillScreen(Colors::BLACK);
        
        tft->setTextSize(2);
        tft->setTextColor(Colors::YELLOW, Colors::BLACK);
        String line = "Sending message " + String(counter);
        tft->drawString(line, tft->width() / 2, tft->height() / 2);
        
        ensureDeselected();
    }
    
    void showError(const char* message) {
        if (!tft) return;
        
        tft->fillScreen(Colors::BLACK);
        tft->setTextSize(2);
        tft->setTextColor(Colors::RED, Colors::BLACK);
        tft->drawString(message, tft->width() / 2, tft->height() / 2);
        
        ensureDeselected();
    }
    
    /**
     * @brief Force LCD chip-select HIGH so the radio can use SPI.
     * 
     * Called automatically by showIdle / showSending / showError,
     * but exposed publicly for safety.
     */
    void ensureDeselected() {
        digitalWrite(LCD_CS, HIGH);
    }
};

} // namespace NessoN1
} // namespace TsUnbLib

#endif // NESSON1_DISPLAY_H_
