# open-mioty-arduino

**Platform-independent Arduino library for mioty® (ETSI TS 103 357 TS-UNB) protocol**

[![License](https://img.shields.io/badge/License-Academic%20%26%20Non--Commercial-blue.svg)](LICENSE.md)
[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20RP2040-brightgreen.svg)](https://www.arduino.cc)
[![Status](https://img.shields.io/badge/Status-Test%20%26%20Demo%20Only-orange.svg)](NOTICE.md)

> ⚠️ **IMPORTANT**: This is a third-party educational project for testing and demonstration only. See [NOTICE.md](NOTICE.md) and [LICENSE.md](LICENSE.md) for legal disclaimers.

## Overview

open-mioty-arduino is an open-source, platform-independent Arduino library implementing the **mioty®** wireless protocol (ETSI TS 103 357 TS-UNB) for low-power, long-range IoT applications. Based on Fraunhofer IIS's reference implementation, this library provides a clean, modular architecture supporting multiple microcontroller platforms, radio chips, and regional configurations.

### What is mioty®?

**[mioty®](https://en.wikipedia.org/wiki/Mioty)** (formerly TS-UNB - Telegram Splitting Ultra Narrow Band) is a robust LPWAN technology designed for IoT:

- **Long Range**: Up to 15 km line-of-sight
- **Low Power**: Years of battery life
- **High Reliability**: Telegram splitting for interference resistance
- **Scalable**: Support for millions of devices per base station
- **Standardized**: ETSI TS 103 357

### Key Features

- ✅ **Platform Independent**: Support for ESP32, RP2040, and more
- ✅ **Multiple Radio Chips**: SX1262, RFM69HW (more coming soon)
- ✅ **Regional Profiles**: EU (868 MHz), US (915 MHz), IN (865 MHz)
- ✅ **Arduino-Style API**: Simple, intuitive interface
- ✅ **Clean Architecture**: Layered design for easy porting
- ✅ **Open Source**: MIT-compatible license for non-commercial use

## 📋 Table of Contents

- [Supported Hardware](#supported-hardware)
- [Architecture](#architecture)
- [Installation](#installation)
- [Getting Started](#getting-started)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Porting Guide](#porting-guide)
- [License](#license)

## Supported Hardware

### Platforms (Microcontrollers)

| Platform | Status | Notes |
|----------|--------|-------|
| **ESP32-C6** | ✅ Tested | Arduino Nesso N1, hardware timer support |
| **RP2040** | 🔶 Partial | Core code ready, needs board templates |

### Radio Chips

| Radio Chip | Status | Interface | Bi-Directional | Notes |
|------------|--------|-----------|----------------|-------|
| **SX1262** | ✅ Working | SPI (RadioLib) | ❌ No | Used on Arduino Nesso N1, uplink only |
| **RFM69HW** | 🔶 Stub | SPI (Direct) | 🔶 Unimplemented | Theoretically bi-directional, not yet implemented |

### Boards

| Board | Platform | Radio | Status |
|-------|----------|-------|--------|
| **Arduino Nesso N1** | ESP32-C6 | SX1262 | ✅ Fully Supported |
| HopeRF Demo Kit | RP2040 | RFM69HW | 🔶 Planned |

### Regional Profiles

| Profile | Region | Frequency | Power | Channels | Status |
|---------|--------|-----------|-------|----------|--------|
| **EU0** | Europe | 868.3 MHz | 14 dBm | Single | ✅ Tested |
| **EU1** | Europe | 868.18/868.08 MHz | 14 dBm | Dual | ✅ Tested |
| **EU2** | Europe | 868.95/869.05 MHz | 14 dBm | Dual | 🔶 Untested |
| **US0** | USA | 915.6 MHz | 22 dBm | Single | 🔶 Untested |
| **IN0** | India | 865.6/865.4 MHz | 14 dBm | Dual | 🔶 Untested |

**Legend**: ✅ Tested | 🔶 Untested (should work) | ❌ Not Started

## Architecture

open-mioty-arduino uses a clean layered architecture that separates concerns and enables easy porting:

```
┌─────────────────────────────────────────────────────────────┐
│                   APPLICATION LAYER                         │
│  (User sketch.ino)                                          │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   REGIONAL LAYER                            │
│  (RegionalProfiles.h)                                       │
│  • Frequency bands: EU1 (868 MHz), US0 (915 MHz), IN0      │
│  • TX power limits: 14 dBm (EU), 22 dBm (US)                │
│  • Carrier spacing and modulation parameters                │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   RADIO DRIVER LAYER                        │
│  (SX1262RadioLib.h, RFM69HW.h)                              │
│  • Radio chip-specific implementation                       │
│  • Receives pins from Board Layer                           │
│  • Receives parameters from Regional Layer                  │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   BOARD LAYER                               │
│  (BoardConfig.h)                                            │
│  • Pin mappings: CS, DIO1, BUSY, RESET                      │
│  • SPI configuration: SCK, MISO, MOSI, clock                │
│  • Board-specific hardware details                          │
└────────────────────────┬────────────────────────────────────┘
                         │
┌────────────────────────▼────────────────────────────────────┐
│                   PLATFORM LAYER                            │
│  (ESP32TsUnb.h, RP2040TsUnb.h)                              │
│  • MCU-specific SPI implementation                          │
│  • Hardware timers for precise symbol timing                │
│  • Interrupt handling and low-level peripherals             │
└─────────────────────────────────────────────────────────────┘
```

### Design Benefits

1. **Separation of Concerns**: Each layer has a single responsibility
2. **Easy Porting**: Add new board = create `BoardConfig.h` + `Templates.h`
3. **Zero Runtime Overhead**: All configuration via templates (compile-time)
4. **Type Safety**: Wrong pins = compile error, not runtime failure

### File Organization

```
open-mioty-arduino/
├── src/
│   ├── open-mioty-arduino.h           # Main header
│   ├── core/                       # mioty® protocol (Fraunhofer)
│   │   ├── TsUnb/
│   │   │   ├── SimpleNode.h        # Main API class
│   │   │   ├── FixedMac.h          # MAC layer
│   │   │   ├── Phy.h               # PHY layer
│   │   │   └── RadioBurst.h        # Burst configuration
│   │   ├── Encryption/
│   │   │   └── Aes128.h            # AES encryption
│   │   └── Utils/
│   │       └── BitAccess.h         # Bit manipulation
│   ├── drivers/                    # Radio Driver Layer
│   │   ├── SX1262RadioLib.h        # SX1262 via RadioLib
│   │   └── RFM69HW.h               # RFM69HW (future)
│   ├── platform/                   # Platform + Board Layer
│   │   ├── esp32/
│   │   │   ├── ESP32TsUnb.h        # ESP32 platform
│   │   │   ├── ESP32Utils.h        # Utility functions
│   │   │   └── nesso-n1/           # Board configuration
│   │   │       ├── BoardConfig.h   # Pin definitions
│   │   │       └── Templates.h     # Type aliases
│   │   └── rp2040/
│   │       └── ...                 # RP2040 support
│   └── regional/                   # Regional Layer
│       └── RegionalProfiles.h      # EU/US/IN configurations
└── examples/
    └── basicTransmit/
        └── basicTransmit.ino       # Getting started example
```

## Installation

### Arduino IDE

1. Download this repository as ZIP
2. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library**
3. Select the downloaded ZIP file
4. Restart Arduino IDE

### PlatformIO

Add to your `platformio.ini`:

```ini
[env:nesso_n1]
platform = espressif32
board = esp32-c6-devkitc-1
framework = arduino
lib_deps = 
    https://github.com/yourusername/TS-UNB-Arduino.git
    jgromes/RadioLib @ ^6.0.0
```

### Dependencies

- **RadioLib** (for SX1262): `https://github.com/jgromes/RadioLib`
- **Arduino Nesso N1 Board Support** (for Nesso N1): Follow official installation guide

## Getting Started

### Basic Transmission Example

This example demonstrates the simplest possible mioty transmission:

```cpp
#include <Arduino_Nesso_N1.h>
#include <open-mioty-arduino.h>

// Network configuration
const uint8_t NETWORK_KEY[16] = {
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};
const uint16_t SHORT_ADDRESS = 0x70FF;

// Transmit configuration
uint32_t packetCounter = 0;
const unsigned long TX_INTERVAL = 60000;  // 60 seconds

// Select your region:
TsUnb::EU1 miotyNode;  // Europe: 868 MHz, 14 dBm
// TsUnb::US0 miotyNode;  // USA: 915 MHz, 22 dBm
// TsUnb::IN0 miotyNode;  // India: 865 MHz, 14 dBm

void setup() {
  Serial.begin(115200);
  
  // Enable radio module (Arduino Nesso N1 specific)
  TsUnbLib::ESP32::enableRadioModule();
  
  // Initialize mioty node
  if (miotyNode.init() < 0) {
    Serial.println("Init failed!");
    while (1) delay(1000);
  }
  
  // Configure network
  uint8_t eui64[8];
  TsUnbLib::ESP32::getEui64(eui64);
  
  miotyNode.setNetworkKey(NETWORK_KEY);
  miotyNode.setEUI64(eui64);
  miotyNode.setShortAddress(SHORT_ADDRESS);
  miotyNode.setPacketCounter(packetCounter);
  
  // Display configuration
  char euiStr[24];
  miotyNode.getEUI64String(euiStr);
  Serial.print("EUI-64: ");
  Serial.println(euiStr);
  Serial.println("Ready!");
}

void loop() {
  // Create packet
  String message = "Hello mioty! #" + String(packetCounter);
  uint8_t payload[32];
  message.toCharArray((char*)payload, sizeof(payload));
  
  // Transmit
  Serial.print("Transmitting... ");
  if (miotyNode.send(payload, message.length() + 1) == 0) {
    Serial.println("success!");
    packetCounter++;
    miotyNode.setPacketCounter(packetCounter);
  } else {
    Serial.println("failed!");
  }
  
  // Wait (respect duty cycle)
  delay(TX_INTERVAL);
}
```

### What's Happening?

1. **Regional Selection**: `TsUnb::EU1` automatically configures:
   - Frequency: 868.18 MHz / 868.08 MHz (dual channel)
   - TX Power: 14 dBm (25 mW)
   - Modulation: FSK with mioty® parameters

2. **Network Configuration**:
   - `setNetworkKey()`: 128-bit AES key (must match the configuration on your mioty Application Center)
   - `setEUI64()`: Unique device identifier (uses ESP32's hardware EUI64 if not set)
   - `setShortAddress()`: 16-bit address for uplink

3. **Transmission**:
   - `send()` handles MAC encoding, PHY modulation, and burst transmission
   - Automatic telegram splitting across multiple frequency carriers
   - Precise timing via hardware timer (< 20 ppm accuracy)

### Understanding Duty Cycle

European regulations (ETSI EN 300 220) require **1% duty cycle** on 868 MHz:
- Max 36 seconds TX per hour
- Example: 1-second packet → wait 99 seconds before next transmission

**⚠️ Important**: This library does **not enforce** duty cycle limits. You are responsible for implementing appropriate delays between transmissions to comply with regional regulations.

Our example uses 60-second intervals for margin. Adjust `TX_INTERVAL` based on your packet size.

## API Reference

### SimpleNode Class

Main interface for mioty® communication.

#### Initialization

```cpp
int16_t init()
```
Initialize the mioty node. Call once in `setup()`.
- **Returns**: `0` on success, negative on error

#### Configuration Methods

```cpp
void setNetworkKey(const uint8_t* key)
```
Set 128-bit network encryption key (16 bytes).

```cpp
void setEUI64(const uint8_t* eui)
```
Set device EUI-64 identifier (8 bytes). If not called, the library will use the ESP32's hardware EUI64 automatically.

```cpp
void setShortAddress(uint16_t address)
```
Set 16-bit short address.

```cpp
void setPacketCounter(uint32_t counter)
```
Set/reset packet counter value.

```cpp
uint32_t getPacketCounter()
```
Get current packet counter.

#### Display Helpers

```cpp
void getEUI64String(char* buffer)
```
Get formatted EUI-64 string (e.g., `"A4:CF:12:34:56:78:9A:BC"`).
- **buffer**: Must be at least 24 bytes

```cpp
void getNetworkKeyString(char* buffer)
```
Get formatted network key string.
- **buffer**: Must be at least 48 bytes

```cpp
uint16_t getShortAddress()
```
Get short address as 16-bit value.

#### Transmission

```cpp
int16_t send(const uint8_t* payload, uint16_t payloadLength)
```
Transmit mioty® packet.
- **payload**: Data to transmit (max ~240 bytes)
- **payloadLength**: Length in bytes
- **Returns**: `0` on success, negative on error

### Regional Profiles

Available in `TsUnb::` namespace:

| Profile | Region | Frequency | Power | Channels |
|---------|--------|-----------|-------|----------|
| `EU0` | Europe | 868.3 MHz | 14 dBm | Single |
| `EU1` | Europe | 868.18/868.08 MHz | 14 dBm | Dual |
| `EU2` | Europe | 868.95/869.05 MHz | 14 dBm | Dual |
| `US0` | USA | 915.6 MHz | 22 dBm | Single |
| `IN0` | India | 865.6/865.4 MHz | 14 dBm | Dual |

### Utility Functions (ESP32)

```cpp
void TsUnbLib::ESP32::enableRadioModule()
```
Enable SX1262 power on Arduino Nesso N1. Call before `init()`.

```cpp
void TsUnbLib::ESP32::getEui64(uint8_t* eui64)
```
Read unique ESP32 MAC address as EUI-64.

## Examples

### basicTransmit

Simple periodic transmission with packet counter.

**Location**: `examples/basicTransmit/basicTransmit.ino`

**Features**:
- Region selection (EU/US/IN)
- Automatic EUI-64 from chip
- Packet counter management
- Serial output for debugging

## Porting Guide

### Adding a New Board

Example: Custom ESP32 board with different pin mappings

**1. Create board directory:**
```
src/platform/esp32/my-board/
├── BoardConfig.h
└── Templates.h
```

**2. Define pins in `BoardConfig.h`:**
```cpp
namespace TsUnbLib {
namespace MyBoard {

struct BoardConfig {
    // SPI Configuration
    static constexpr int8_t SPI_SCK = 18;
    static constexpr int8_t SPI_MISO = 19;
    static constexpr int8_t SPI_MOSI = 23;
    static constexpr uint32_t SPI_CLOCK_HZ = 4000000;
    
    // Radio Pins
    static constexpr int8_t RADIO_CS = 5;
    static constexpr int8_t RADIO_DIO1 = 26;
    static constexpr int8_t RADIO_RESET = 14;
    static constexpr int8_t RADIO_BUSY = 32;
    
    // Platform settings
    static constexpr uint16_t SYMBOL_RATE_MULT = 48;
    static constexpr int16_t TIMING_OFFSET_PPM = 0;
};

} // namespace MyBoard
} // namespace TsUnbLib
```

**3. Create type aliases in `Templates.h`:**
```cpp
#include "../ESP32TsUnb.h"
#include "BoardConfig.h"
#include "../../../drivers/SX1262RadioLib.h"
#include "../../../regional/RegionalProfiles.h"

namespace TsUnbLib {
namespace MyBoard {

using Board = BoardConfig;

// EU1 configuration
using TsUnb_EU1 = TsUnb::SimpleNode<
    TsUnb::FixedUplinkMac,
    TsUnb::Phy<...>,
    Drivers::SX1262RadioLib<
        ESP32::ESP32TsUnb<Regional::EU1_Profile, Board::RADIO_CS, ...>,
        Regional::EU1_Profile,
        Board::RADIO_CS,
        Board::RADIO_DIO1,
        Board::RADIO_RESET,
        Board::RADIO_BUSY,
        TsUnb::RadioBurst<2, 2>
    >,
    false
>;

// Add US0, IN0, etc. similarly...

} // namespace MyBoard
} // namespace TsUnbLib
```

**4. Add convenience aliases in main header:**
```cpp
// In open-mioty-arduino.h
namespace TsUnb {
    #ifdef ARDUINO_MY_BOARD
        using EU1 = TsUnbLib::MyBoard::TsUnb_EU1;
        using US0 = TsUnbLib::MyBoard::TsUnb_US0;
    #endif
}
```

### Adding a New Platform

For a completely new MCU family (e.g., STM32):

**1. Implement platform interface** in `src/platform/stm32/STM32TsUnb.h`:
- `spiInit()`, `spiDeinit()`
- `spiSend()`, `spiSendReceive()`
- `initTimer()`, `startTimer()`, `stopTimer()`
- `addTimerDelay()`, `waitTimer()`

**2. Create board configurations** following the structure above.

See `ARCHITECTURE.md` for detailed porting instructions.

## Regulatory & Legal

⚠️ **For complete legal disclaimers and usage restrictions, see [NOTICE.md](NOTICE.md) and [LICENSE.md](LICENSE.md).**

### Important Notes

- **Non-Commercial Use Only**: This library is for academic, educational, and personal use
- **Patent Licensing**: Commercial use of mioty® requires licenses from [Sisvel International S.A.](https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)
- **Regulatory Compliance**: You are responsible for FCC, ETSI, and local radio regulations
- **Duty Cycle**: Respect regional duty cycle limits (typically 1% for Europe)
- **No Warranties**: Provided "AS IS" without any warranty

For production deployment, contact Fraunhofer IIS for commercially-licensed mioty® software.

## Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Follow existing code style
4. Add tests/examples for new features
5. Submit pull request

### Areas for Contribution

- Additional platform support (STM32, nRF52, etc.)
- Radio driver implementations (RFM69HW, CC1101, etc.)
- Example applications
- Documentation improvements
- Testing and validation

## License

**Third-Party Modified Version of the Fraunhofer TS-UNB-Lib**

- **Original**: © 2019-2023 Fraunhofer-Gesellschaft
- **License**: Academic and non-commercial use only
- **Modified by**: Independent contributors (open-mioty-arduino project)

See [LICENSE.md](LICENSE.md) and [NOTICE.md](NOTICE.md) for complete terms and disclaimers.

## Acknowledgments

- **Fraunhofer IIS**: Original TS-UNB-Lib implementation
- **RadioLib**: Excellent Arduino radio library by Jan Gromeš
- **Arduino Community**: Testing and feedback

## Support

### Documentation

- **Architecture Guide**: See `ARCHITECTURE.md`
- **Examples**: Browse `examples/` directory
- **API Reference**: See header file documentation

### Community

- **GitHub Issues**: Bug reports and feature requests
- **Discussions**: Q&A and community support

### Commercial Support

For commercial licensing and professional support:
- **Sisvel**: [mioty® licensing](https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)
- **Fraunhofer IIS**: [Contact](https://www.iis.fraunhofer.de/en/ff/lv/lok/tech/mioty.html)

---

**Built with ❤️ for the IoT community**

*This is an independent open-source project. Not affiliated with or endorsed by Fraunhofer IIS or Sisvel International S.A.*
