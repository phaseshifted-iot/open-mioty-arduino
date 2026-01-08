# Board Configurations

This directory contains board-specific hardware configurations for different boards using open-mioty-arduino.

## Directory Structure

```
boards/
├── esp32/
│   ├── nesso-n1/          # Arduino Nesso N1 (ESP32-C6 + SX1262)
│   │   └── NessoN1Board.h
│   └── your-board/        # Add your ESP32-based board here
│       └── YourBoardConfig.h
└── rp2040/
    └── your-board/        # Add your RP2040-based board here
        └── YourBoardConfig.h
```

## Architecture Overview

### 1. **Platform Layer** (`src/platform/`)
   - **Purpose**: Microcontroller-specific implementations
   - **Examples**: `esp32/`, `rp2040/`
   - **Defines**: SPI communication, timers, HAL for specific MCU families
   - **Reusable**: Same platform code works for all boards using that MCU

### 2. **Board Layer** (`src/boards/`) ← THIS DIRECTORY
   - **Purpose**: Board-specific hardware configuration
   - **Examples**: `esp32/nesso-n1/`, `esp32/heltec-v3/`
   - **Defines**: Pin assignments, SPI pins, radio pins, board-specific constants
   - **Board-specific**: Each board has unique configuration

### 3. **Driver Layer** (`src/drivers/`)
   - **Purpose**: Radio chip drivers (SX1262, RFM69, etc.)
   - **Generic**: Works with any platform and board combination

### 4. **Regional Layer** (`src/regional/`)
   - **Purpose**: Region-specific radio parameters and complete node templates
   - **Examples**: `NessoN1Templates.h` (combines Board + Platform + Region)
   - **Defines**: Frequency plans, power limits for EU, US, IN, etc.

## Adding a New Board

### Example: Adding a Heltec WiFi LoRa 32 V3 (ESP32-S3 + SX1262)

1. **Create board directory:**
   ```
   mkdir -p boards/esp32/heltec-v3
   ```

2. **Create board config file:** `boards/esp32/heltec-v3/HeltecV3Board.h`
   ```cpp
   namespace TsUnbLib {
   namespace Boards {
   namespace HeltecV3 {
   
   struct BoardConfig {
       // SPI Configuration (Heltec V3 uses default ESP32-S3 SPI pins)
       static constexpr int8_t SPI_SCK = -1;   // Use default
       static constexpr int8_t SPI_MISO = -1;  // Use default
       static constexpr int8_t SPI_MOSI = -1;  // Use default
       static constexpr uint32_t SPI_CLOCK_HZ = 4000000;
       
       // SX1262 Radio Pins (example - verify with schematic!)
       static constexpr int8_t RADIO_CS = 8;
       static constexpr int8_t RADIO_DIO1 = 14;
       static constexpr int8_t RADIO_RESET = 12;
       static constexpr int8_t RADIO_BUSY = 13;
       
       // Platform settings
       static constexpr uint16_t SYMBOL_RATE_MULT = 48;
       static constexpr int16_t TIMING_OFFSET_PPM = 0;
       
       // Board info
       static constexpr const char* BOARD_NAME = "Heltec WiFi LoRa 32 V3";
       static constexpr const char* MCU_TYPE = "ESP32-S3";
       static constexpr const char* RADIO_TYPE = "SX1262";
   };
   
   } // namespace HeltecV3
   } // namespace Boards
   } // namespace TsUnbLib
   ```

3. **Create templates file:** `regional/HeltecV3Templates.h`
   ```cpp
   #include "../platform/esp32/ESP32TsUnb.h"
   #include "../boards/esp32/heltec-v3/HeltecV3Board.h"
   #include "RegionalProfiles.h"
   
   namespace TsUnbLib {
   namespace HeltecV3 {
   
   using Board = Boards::HeltecV3::BoardConfig;
   
   using TsUnb_EU1 = TsUnb::SimpleNode<
       TsUnb::FixedUplinkMac,
       TsUnb::Phy<...>,
       Drivers::SX1262RadioLib<ESP32::ESP32TsUnb<Board::RADIO_CS,
                                                 Board::SYMBOL_RATE_MULT,
                                                 Board::TIMING_OFFSET_PPM,
                                                 Board::SPI_CLOCK_HZ,
                                                 Board::SPI_SCK,
                                                 Board::SPI_MISO,
                                                 Board::SPI_MOSI>,
                              Regional::EU1_Profile::MAX_POWER_DBM,
                              ...>,
       false>;
   
   } // namespace HeltecV3
   } // namespace TsUnbLib
   ```

4. **Add to main library header** (optional):
   ```cpp
   // In open-mioty-arduino.h
   #ifdef ARDUINO_HELTEC_WIFI_LORA_32_V3
   #include "regional/HeltecV3Templates.h"
   #endif
   ```

## Key Principles

1. **Separation of Concerns**:
   - Platform = MCU-specific (ESP32 vs RP2040)
   - Board = Pin assignments and hardware config
   - Regional = Radio frequency/power regulations

2. **Reusability**:
   - Same platform code for all ESP32 boards
   - Same driver code for all SX1262 radios
   - Mix and match as needed

3. **Maintainability**:
   - Board configs in one place
   - Easy to add new boards without changing platform code
   - Clear hierarchy: Platform → Board → Regional → Templates

4. **Flexibility**:
   - Use `-1` for SPI pins to use MCU defaults
   - Specify custom pins when needed (like Nesso N1)
   - Board configs can include any board-specific constants

## Current Boards

- **Arduino Nesso N1** (`boards/esp32/nesso-n1/`)
  - MCU: ESP32-C6
  - Radio: SX1262
  - Custom SPI pins (shared with display)
