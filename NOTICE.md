# Third-Party Notices

This project includes or depends on third-party software components.

---

## Fraunhofer TS-UNB-Lib

**Copyright:** (c) 2019 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V.

**License:** See [LICENSE.md](LICENSE.md)

**Source:** https://github.com/mioty-iot

**Modified:** This is a third-party modified version adapted for ESP32-C6 platform (January 2026).

**Used in:**
- Core MIOTY protocol implementation (`src/core/TsUnb/`)
- AES-128 encryption (`src/core/Encryption/`)
- Utility functions (`src/core/Utils/`)

---

## 2. RadioLib

**Copyright:** (c) 2018 Jan Gromeš

**License:** MIT License (see LICENSE-RADIOLIB.md)

**Repository:** https://github.com/jgromes/RadioLib

**Used For:** SX1262 radio transceiver driver functionality

**IRadioLib

**Copyright:** (c) 2018 Jan Gromeš

**License:** MIT License  
See [extras/licenses/LICENSE-RADIOLIB.md](extras/licenses/LICENSE-RADIOLIB.md)

**Repository:** https://github.com/jgromes/RadioLib

**Used for:** SX1262 radio transceiver driver

**Used For:** ESP32-C6 platform support, SPI, GPIO, Timer functionality

---
ESP32 Arduino Core

**Copyright:** Espressif Systems

**License:** LGPL 2.1

**Repository:** https://github.com/espressif/arduino-esp32

**Used for:** ESP32-C6 platform support (SPI, GPIO, timers)

---

## Acknowledgments

Thanks to:
- **Fraunhofer IIS** for the TS-UNB reference implementation
- **Jan Gromeš** for RadioLib
- **Espressif Systems** for ESP32 platform support
- The open-source community