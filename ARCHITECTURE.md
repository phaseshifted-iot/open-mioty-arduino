# open-mioty-arduino Library Architecture

## Overview

This library implements a clean separation of concerns across four architectural layers:

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                        │
│  (User code: sketch.ino)                                    │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   REGIONAL LAYER                            │
│  (RegionalProfiles.h)                                       │
│  - Frequency bands (868/915 MHz)                            │
│  - TX power limits (14/22 dBm)                              │
│  - Carrier spacing                                          │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   RADIO DRIVER LAYER                        │
│  (SX1262RadioLib.h, RFM69HW.h, etc.)                        │
│  - Radio chip implementation                                │
│  - Receives pins from Board Layer (template params)         │
│  - Receives power from Regional Layer                       │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                    BOARD LAYER                              │
│  (BoardConfig.h)                                            │
│  - Pin mappings (CS, DIO1, BUSY, RESET)                     │
│  - SPI configuration (SCK, MISO, MOSI, clock)               │
│  - Board-specific hardware details                          │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   PLATFORM LAYER                            │
│  (ESP32TsUnb.h, RP2040TsUnb.h, etc.)                        │
│  - MCU-specific implementation                              │
│  - SPI driver, timers, interrupts                           │
│  - Clock configuration                                      │
└─────────────────────────────────────────────────────────────┘
```

## Detailed Layer Descriptions

### 1. Platform Layer

**Location:** `src/platform/<mcu-family>/`
**Examples:** ESP32, RP2040, ATmega328p

**Responsibilities:**
- MCU-specific SPI implementation
- Hardware timer configuration for symbol timing
- Interrupt handling
- Low-level peripheral access

**Template Parameters Received:**
- SPI pins from Board Layer (SCK, MISO, MOSI, CS)
- SPI clock frequency from Board Layer
- Symbol rate and timing configuration

**Example (ESP32-C6):**
```cpp
ESP32TsUnb<
    Board::RADIO_CS,        // CS pin for this board
    Board::SYMBOL_RATE_MULT,// Symbol rate multiplier
    Board::TIMING_OFFSET_PPM,// Crystal tolerance correction
    Board::SPI_CLOCK_HZ,    // SPI clock frequency
    Board::SPI_SCK,         // SPI clock pin
    Board::SPI_MISO,        // SPI MISO pin
    Board::SPI_MOSI         // SPI MOSI pin
>
```

### 2. Board Layer

**Location:** `src/platform/<mcu-family>/<board-name>/BoardConfig.h`
**Examples:** Nesso N1, HopeRF Demo Kit, Custom boards

**Responsibilities:**
- Define GPIO pin connections to radio module
- Define SPI bus configuration (important for shared SPI buses)
- Specify radio chip type selection
- Board-specific hardware notes (e.g., I/O expander control)

**Configuration Structure:**
```cpp
struct BoardConfig {
    // SPI Configuration (platform-specific pins)
    static constexpr int8_t SPI_SCK = 20;
    static constexpr int8_t SPI_MISO = 22;
    static constexpr int8_t SPI_MOSI = 21;
    static constexpr uint32_t SPI_CLOCK_HZ = 4000000;
    
    // Radio Control Pins (board-specific connections)
    static constexpr int8_t RADIO_CS = 23;
    static constexpr int8_t RADIO_DIO1 = 15;
    static constexpr int8_t RADIO_RESET = -2;  // RADIOLIB_NC
    static constexpr int8_t RADIO_BUSY = 19;
    
    // Platform-specific settings
    static constexpr uint16_t SYMBOL_RATE_MULT = 48;
    static constexpr int16_t TIMING_OFFSET_PPM = 0;
};
```

**Key Point:** Different boards on the same platform (e.g., two ESP32-C6 boards) will have different pin mappings but use the same platform implementation.

### 3. Radio Driver Layer

**Location:** `src/drivers/`
**Examples:** SX1262RadioLib, RFM69HW

**Responsibilities:**
- Radio chip-specific communication protocol
- FSK/GFSK modulation configuration
- Burst transmission implementation
- Power management

**Template Parameters:**
```cpp
template <
    class Cpu_T,            // Platform implementation
    int8_t NSS_PIN,         // From Board Layer
    int8_t DIO1_PIN,        // From Board Layer
    int8_t RESET_PIN,       // From Board Layer
    int8_t BUSY_PIN,        // From Board Layer
    int8_t TX_POWER_DBM,    // From Regional Layer
    uint32_t FREQ_DEV_HZ,   // mioty standard
    class RadioBurst_T      // Burst configuration
>
class SX1262RadioLib {
    // Radio-specific implementation
};
```

**Design Principle:** Radio pins are passed as template parameters (compile-time), ensuring zero runtime overhead and type safety.

### 4. Regional Layer

**Location:** `src/regional/RegionalProfiles.h`

**Responsibilities:**
- Define frequency bands for each region
- Specify maximum transmit power limits
- Configure carrier spacing
- Regional regulatory compliance

**Examples:**
```cpp
// Europe: 868 MHz, 14 dBm
struct EU1_Profile {
    static constexpr uint32_t FREQ_CH_A = 86818;  // 868.18 MHz
    static constexpr uint32_t FREQ_CH_B = 86808;  // 868.08 MHz
    static constexpr int8_t MAX_POWER_DBM = 14;
};

// USA: 915 MHz, 22 dBm
struct US0_Profile {
    static constexpr uint32_t FREQ_CH_A = 91560;  // 915.6 MHz
    static constexpr uint32_t FREQ_CH_B = 91560;
    static constexpr int8_t MAX_POWER_DBM = 22;
};
```

## Complete Data Flow Example

### Arduino Nesso N1 with EU1 Configuration

**User Code:**
```cpp
#include <open-mioty-arduino.h>
TsUnb::EU1 miotyNode;  // Simple!
```

**Behind the Scenes (Template Expansion):**

```cpp
// Step 1: Board Layer defines pins
// File: src/platform/esp32/nesso-n1/BoardConfig.h
struct BoardConfig {
    static constexpr int8_t SPI_SCK = 20;
    static constexpr int8_t SPI_MISO = 22;
    static constexpr int8_t SPI_MOSI = 21;
    static constexpr int8_t RADIO_CS = 23;
    static constexpr int8_t RADIO_DIO1 = 15;
    static constexpr int8_t RADIO_BUSY = 19;
    static constexpr int8_t RADIO_RESET = -2;
};

// Step 2: Regional Layer defines frequency and power
// File: src/regional/RegionalProfiles.h
struct EU1_Profile {
    static constexpr uint32_t FREQ_CH_A = 86818;  // 868.18 MHz
    static constexpr int8_t MAX_POWER_DBM = 14;
};

// Step 3: Template combines all layers
// File: src/platform/esp32/nesso-n1/Templates.h
using TsUnb_EU1 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<...>,
    Drivers::SX1262RadioLib<
        // Platform Layer
        ESP32::ESP32TsUnb<
            Board::RADIO_CS,      // 23 - from Board
            Board::SYMBOL_RATE_MULT,
            Board::TIMING_OFFSET_PPM,
            Board::SPI_CLOCK_HZ,  // 4MHz - from Board
            Board::SPI_SCK,       // 20 - from Board
            Board::SPI_MISO,      // 22 - from Board
            Board::SPI_MOSI       // 21 - from Board
        >,
        // Radio pins from Board Layer
        Board::RADIO_CS,          // 23
        Board::RADIO_DIO1,        // 15
        Board::RADIO_RESET,       // -2 (RADIOLIB_NC)
        Board::RADIO_BUSY,        // 19
        // Regional settings
        Regional::EU1_Profile::MAX_POWER_DBM,  // 14
        Regional::MiotyParams::FREQ_DEVIATION, // 595
        TsUnb::RadioBurst<2, 2>
    >,
    false
>;
```

## Adding Support for a New Board

### Example: Adding HopeRF Demo Kit on RP2040

1. **Create board directory:**
   ```
   src/platform/rp2040/hoperfdevkit/
   ├── BoardConfig.h
   └── Templates.h
   ```

2. **Define pins in BoardConfig.h:**
   ```cpp
   namespace TsUnbLib {
   namespace HopeRFDevKit {
   
   struct BoardConfig {
       // SPI Configuration (RP2040 default SPI pins)
       static constexpr int8_t SPI_SCK = 2;
       static constexpr int8_t SPI_MISO = 4;
       static constexpr int8_t SPI_MOSI = 3;
       static constexpr uint32_t SPI_CLOCK_HZ = 4000000;
       
       // RFM69HW Radio Pins (example)
       static constexpr int8_t RADIO_CS = 5;
       static constexpr int8_t RADIO_DIO0 = 6;
       static constexpr int8_t RADIO_DIO2 = 7;
       static constexpr int8_t RADIO_RESET = 8;
       
       // Platform settings
       static constexpr uint16_t SYMBOL_RATE_MULT = 48;
       static constexpr int16_t TIMING_OFFSET_PPM = 0;
   };
   
   } // namespace HopeRFDevKit
   } // namespace TsUnbLib
   ```

3. **Create Templates.h:**
   ```cpp
   #include "../RP2040TsUnb.h"
   #include "BoardConfig.h"
   #include "../../../drivers/RFM69HW.h"  // Different radio!
   
   namespace TsUnbLib {
   namespace HopeRFDevKit {
   
   using Board = BoardConfig;
   
   using TsUnb_EU1 = TsUnb::SimpleNode<
       TsUnb::FixedUplinkMac,
       TsUnb::Phy<...>,
       Drivers::RFM69HW<               // Different radio driver
           RP2040::RP2040TsUnb<...>,   // Different platform
           Board::RADIO_CS,
           Board::RADIO_DIO0,
           Board::RADIO_DIO2,
           Board::RADIO_RESET,
           Regional::EU1_Profile::MAX_POWER_DBM,
           ...
       >,
       false
   >;
   
   } // namespace HopeRFDevKit
   } // namespace TsUnbLib
   ```

4. **Add convenience alias in main header:**
   ```cpp
   // In OpenMioty.h
   namespace TsUnb {
       #ifdef ARDUINO_HOPERFDEVKIT
           using EU1 = TsUnbLib::HopeRFDevKit::TsUnb_EU1;
       #endif
   }
   ```

## Key Design Benefits

### 1. **Separation of Concerns**
- Platform developers work on MCU-specific code
- Board designers define pin mappings
- Radio driver developers implement chip protocols
- Regional settings are centralized

### 2. **Zero Runtime Overhead**
- All configuration via template parameters (compile-time)
- No virtual functions, no runtime polymorphism
- Compiler can optimize away abstraction layers

### 3. **Type Safety**
- Incorrect pin assignments caught at compile time
- Regional power limits enforced by type system
- No magic numbers in user code

### 4. **Easy Porting**
- Add new platform: Implement PlatformTsUnb interface
- Add new board: Create BoardConfig + Templates
- Add new radio: Implement RadioDriver interface
- All without modifying existing code

### 5. **User Simplicity**
Despite complex internal architecture, user code is simple:
```cpp
TsUnb::EU1 node;
node.init();
node.send(data, len);
```

## File Organization

```
open-mioty-arduino/
├── src/
│   ├── OpenMioty.h                    # Main header with convenience aliases
│   ├── boards/
│   │   └── esp32/
│   │       └── nesso-n1/
│   │           └── NessoN1Board.h  # Duplicate of BoardConfig (legacy)
│   ├── core/                       # mioty protocol implementation
│   │   ├── TsUnb/
│   │   │   ├── SimpleNode.h
│   │   │   ├── FixedMac.h
│   │   │   ├── Phy.h
│   │   │   └── RadioBurst.h
│   │   ├── Encryption/
│   │   │   └── Aes128.h
│   │   └── Utils/
│   │       └── BitAccess.h
│   ├── drivers/                    # Radio Driver Layer
│   │   ├── SX1262RadioLib.h       # For SX1262/SX1261 (via RadioLib)
│   │   └── RFM69HW.h              # For RFM69HW (future)
│   ├── platform/                   # Platform + Board Layer
│   │   ├── PlatformInterface.h
│   │   ├── esp32/
│   │   │   ├── ESP32TsUnb.h       # Platform Layer
│   │   │   ├── ESP32TsUnb.cpp
│   │   │   ├── ESP32Utils.h
│   │   │   └── nesso-n1/          # Board Layer
│   │   │       ├── BoardConfig.h  # Pin definitions
│   │   │       └── Templates.h    # Combines all layers
│   │   └── rp2040/
│   │       ├── RP2040TsUnb.h      # Platform Layer
│   │       └── opensource-demokit-phaseshifted/  # Board Layer
│   │           ├── BoardConfig.h
│   │           └── Templates.h
│   └── regional/                   # Regional Layer
│       ├── RegionalProfiles.h
│       └── NessoN1Templates.h     # Legacy convenience file
└── examples/
    ├── 01_BasicTransmit_EU/
    ├── 02_BasicTransmit_US/
    └── ...
```

## Compile-Time Configuration Flow

```cpp
// User selects region
TsUnb::EU1 node;

// Expands to:
TsUnbLib::NessoN1::TsUnb_EU1

// Which is defined as:
SimpleNode<
    MAC,
    PHY,
    SX1262RadioLib<
        ESP32TsUnb<23, 48, 0, 4000000, 20, 22, 21>,  // Platform + Board pins
        23,    // NSS from Board
        15,    // DIO1 from Board
        -2,    // RESET from Board
        19,    // BUSY from Board
        14,    // TX_POWER from Regional
        595,   // FREQ_DEV (mioty standard)
        RadioBurst<2, 2>
    >
>

// At compile time, this generates specialized code with:
// - Correct SPI pins (20, 22, 21)
// - Correct radio pins (23, 15, 19)
// - Correct power limit (14 dBm)
// - Correct frequency (868 MHz)
// All constants, no runtime lookup!
```

## Summary

This architecture provides:
- **Clean separation**: Platform ← Board ← Radio ← Regional
- **Compile-time safety**: Wrong pins = compile error
- **Zero overhead**: Templates optimize to direct hardware access
- **Easy maintenance**: Change board without touching platform
- **Simple user API**: `TsUnb::EU1 node;` - that's it!
