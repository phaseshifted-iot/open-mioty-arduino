# OpenMioty

**Open-source mioty® (TS-UNB) protocol library for Arduino**

[![License](https://img.shields.io/badge/License-Academic%20%26%20Non--Commercial-blue.svg)](LICENSE.md)
[![Platform](https://img.shields.io/badge/Platform-ESP32-brightgreen.svg)](https://www.arduino.cc)
[![Status](https://img.shields.io/badge/Status-v0.1.0-orange.svg)](CHANGELOG.md)

> ⚠️ **IMPORTANT**: Academic and non-commercial use only. See [LICENSE.md](LICENSE.md) and [NOTICE.md](NOTICE.md) for legal disclaimers and patent licensing requirements.

## What is OpenMioty?

OpenMioty is an Arduino library for the **mioty®** LPWAN protocol (ETSI TS 103 357 TS-UNB) - a robust, low-power wireless technology for IoT. Based on Fraunhofer IIS's reference implementation, it provides:

- **Long Range**: Up to 15 km line-of-sight
- **Low Power**: Years of battery life
- **High Reliability**: Telegram splitting for interference resistance  
- **Standardized**: ETSI TS 103 357 compliant

**Repository**: [`open-mioty-arduino`](https://github.com/phaseshifted-iot/open-mioty-arduino)  
**Library Name**: `OpenMioty` (Arduino Library Manager)

> **Note**: Third-Party Modified Version of the Fraunhofer TS-UNB-Lib. For production-ready mioty® solutions, see [Fraunhofer IIS](https://www.iis.fraunhofer.de/en/ff/lv/lok/tech/mioty.html) or [StackForce](https://stackforce.com/en/products/protocol-stacks/mioty).

## Quick Start

### Installation

**Arduino Library Manager** (planned):
```
Sketch → Include Library → Manage Libraries → Search "OpenMioty"
```

**Manual Installation**:
1. Download [latest release](https://github.com/phaseshifted-iot/open-mioty-arduino/releases)
2. Arduino IDE: `Sketch → Include Library → Add .ZIP Library`

**PlatformIO**:
```ini
lib_deps = 
    https://github.com/phaseshifted-iot/open-mioty-arduino.git#v0.1.0
    jgromes/RadioLib @ ^6.0.0
```

### Arduino Nesso N1 Example

**Hardware Required**: [Arduino Nesso N1](https://store.arduino.cc/products/nesso-n1) (ESP32-C6 with SX1262 radio)

```cpp
#include <Arduino_Nesso_N1.h>
#include <OpenMioty.h>

// Network configuration (16-byte AES key)
const uint8_t NETWORK_KEY[16] = {
  0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
  0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

TsUnb::EU1 miotyNode;  // Europe 868 MHz (change to US0/IN0 for other regions)

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for serial monitor
  
  Serial.println("OpenMioty - Arduino Nesso N1");
  
  // Enable radio (Arduino Nesso N1 specific)
  TsUnbLib::ESP32::enableRadioModule();
  
  // Initialize mioty
  if (miotyNode.init() < 0) {
    Serial.println("ERROR: Init failed!");
    while (1) delay(1000);
  }
  
  // Configure network
  uint8_t eui64[8];
  TsUnbLib::ESP32::getEui64(eui64);
  miotyNode.setNetworkKey(NETWORK_KEY);
  miotyNode.setEUI64(eui64);
  miotyNode.setShortAddress(0x70FF);
  
  Serial.print("Device EUI: ");
  for (int i = 0; i < 8; i++) {
    Serial.printf("%02X", eui64[i]);
  }
  Serial.println();
  Serial.println("Ready to transmit!");
}

void loop() {
  // Create message
  uint8_t payload[] = "Hello mioty!";
  
  Serial.print("Transmitting... ");
  
  // Transmit (blocks ~1.7s for timing precision)
  if (miotyNode.send(payload, sizeof(payload)) == 0) {
    Serial.println("SUCCESS!");
  } else {
    Serial.println("FAILED!");
  }
  
  delay(60000); // Wait 60 seconds (respect duty cycle!)
}
```

**Next Steps**:
1. Install the Arduino Nesso N1 board support in Arduino IDE
2. Copy the example above or use `File → Examples → OpenMioty → basicTransmitNesso`
3. Upload and open Serial Monitor (115200 baud)
4. You should see successful transmissions every 60 seconds

**Troubleshooting**:
- Ensure antenna is connected to the Nesso N1
- Check that you're using the correct regional profile (EU1 for Europe, US0 for USA)
- Verify Serial Monitor is set to 115200 baud

## Supported Hardware

| Board | MCU | Radio | Status |
|-------|-----|-------|--------|
| [Arduino Nesso N1](https://store.arduino.cc/products/nesso-n1) | ESP32-C6 | SX1262 | ✅ Working |
| [Demo Kit](https://github.com/phaseshifted-iot/mioty-demokit) | RP2040 | RFM69 | Untested |

**Status**: Successfully transmitting on Arduino Nesso N1. RF parameters validated for basic operation.

**Regional Profiles**: EU (868 MHz), US (915 MHz), IN (865 MHz)

## Features

- ✅ ETSI TS 103 357 TS-UNB uplink transmission
- ✅ AES-128 encryption
- ✅ Hardware-timed symbol transmission (< 20 ppm)
- ✅ Multiple regional profiles (EU, US, IN)
- ✅ Clean, modular architecture for easy porting
- ✅ Arduino-style API

## Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)** - Technical design and porting guide
- **[CHANGELOG.md](CHANGELOG.md)** - Version history
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - How to contribute
- **[examples/](examples/)** - Example sketches
- **API Reference** - See header file documentation

## API Reference (Essential)

### Initialization
```cpp
int16_t init()                              // Initialize node
void setNetworkKey(const uint8_t* key)      // Set 128-bit AES key
void setEUI64(const uint8_t* eui)           // Set device ID (8 bytes)
void setShortAddress(uint16_t address)      // Set 16-bit address
```

### Transmission
```cpp
int16_t send(const uint8_t* payload, uint16_t length)  // Transmit packet
```

### Regional Profiles
```cpp
TsUnb::EU0  // Europe: 868.18 MHz, 14 dBm, single channel
TsUnb::EU1  // Europe: 868.18/868.08 MHz, 14 dBm, dual channel (recommended)
TsUnb::EU2  // Europe: 867.625/866.825 MHz, 14 dBm, dual channel, wide TSMA
TsUnb::US0  // USA: 916.4/915.6 MHz, 22 dBm, dual channel, wide TSMA
TsUnb::IN0  // India: 866.875/866.775 MHz, 14 dBm, dual channel
```

### Utility (ESP32)
```cpp
TsUnbLib::ESP32::enableRadioModule()  // Enable SX1262 on Nesso N1
TsUnbLib::ESP32::getEui64(uint8_t*)   // Get unique chip ID
```

### Advanced: Buttons/Sensors with mioty

mioty requires precise timing during transmission. If adding buttons or sensors, use deferred execution:

```cpp
#include <OpenMioty.h>

TsUnbLib::ESP32::DeferredTaskQueue taskQueue;  // Deferred task queue
TsUnb::EU1 miotyNode;

// Button ISR - defer work, don't execute immediately
void IRAM_ATTR buttonISR() {
  taskQueue.schedule(handleButton);  // Queue for later
}

void handleButton() {
  Serial.println("Button pressed!");  // Executes after transmission
}

void loop() {
  taskQueue.executePending();  // Execute deferred tasks
  
  miotyNode.send(payload, len);  // Critical timing - nothing interrupts this
  
  taskQueue.executePending();  // Execute tasks queued during transmission
  delay(60000);
}
```

See [BoardConfig.h](src/platform/esp32/nesso-n1/BoardConfig.h) for detailed patterns and examples.

## Regulatory Compliance

⚠️ **You are responsible for:**
- FCC, ETSI, and local radio regulations
- Duty cycle limits (typically 1% for EU 868 MHz)
- Frequency band restrictions
- Patent licensing for commercial use

**Duty Cycle Example** (EU): 1-second packet requires 99-second wait before next transmission.

## Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Areas for contribution:**
- Platform support (RP2040, STM32, nRF52)
- Radio drivers (RFM69HW, CC1101)
- Board configurations
- Examples and documentation

## License & Legal

**Academic and non-commercial use only**. Third-Party Modified Version of the Fraunhofer TS-UNB-Lib.

- See [LICENSE.md](LICENSE.md) for complete license terms
- See [NOTICE.md](NOTICE.md) for third-party notices
- **Commercial use**: Requires patent licensing via [Sisvel International S.A.](https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)
- **Production software**: Available from [Fraunhofer IIS](https://www.iis.fraunhofer.de/en/ff/lv/lok/tech/mioty.html) or [StackForce](https://stackforce.com/en/products/protocol-stacks/mioty)

## Acknowledgments

- **Fraunhofer IIS** - Original TS-UNB-Lib implementation
- **RadioLib** by Jan Gromeš - Radio driver foundation
- **StackForce** - mioty® protocol stack expertise
- **Arduino Community** - Testing and feedback

## Support

- **Issues**: [GitHub Issues](https://github.com/phaseshifted-iot/open-mioty-arduino/issues)
- **Discussions**: [GitHub Discussions](https://github.com/phaseshifted-iot/open-mioty-arduino/discussions)

> **Note**: Third-Party Modified Version of the Fraunhofer TS-UNB-Lib for educational use. For commercial deployments and professional support, contact [Fraunhofer IIS](https://www.iis.fraunhofer.de/en/ff/lv/lok/tech/mioty.html) or [StackForce](https://stackforce.com/en/products/protocol-stacks/mioty).

---

**Built with ❤️ for the IoT community**
