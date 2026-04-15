# Changelog

All notable changes to the OpenMioty Arduino library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.2] - 2026-04-15

### Added
- `NessoN1Helper.h` — high-level board initialization helper for the Arduino Nesso N1; encapsulates correct BSP lazy-init ordering, I2C, radio power-on, and battery management so user sketches can call a single `NessoN1Helper::init()` before `miotyNode.init()`
- `advancedTransmitNessoDisplay` example — demonstrates cooperative scheduling with display updates around a `node.send()` call
- `.github/skills/nesso-n1-platform.md` — agent/developer reference for Nesso N1 hardware quirks and initialization sequence
- `.github/skills/demokit-rp2040-platform.md` — agent/developer reference for the open-source RP2040 demokit platform (skeleton)
- `AGENTS.md` — mandatory reading order and project identity document for AI coding agents

### Fixed
- **Timer reset race condition** (`ESP32TsUnb.h`): `startTimer()` now resets both the accumulator (`preciseTsUnbTimer`) and the hardware counter (`gptimer_set_raw_count`) before arming the timer. Previously, stale values from the preceding TX could corrupt all alarm times in the next transmission.
- **TX timing order** (`SX1262RadioLib.h`): `startTimer()` is now called *before* `addTimerDelay(4)`. The previous order (delay then start) left the accumulator in a non-zero state at the start of each burst.
- **SPI CS glitch on init** (`ESP32TsUnb.h`): CS pin is driven HIGH before `SPI.begin()` to prevent it from floating low during bus initialization and inadvertently clocking the radio. `spiDeinit()` now holds CS HIGH (was floating INPUT) to keep the radio deselected between transmissions.
- **SX1262 TCXO voltage** (`SX1262RadioLib.h`): `radio.begin()` and `beginFSK()` now pass explicit TCXO voltage (3.0 V) and LDO regulator flag, matching the Nesso N1 hardware. The previous 0.0 V default caused unreliable startup.
- **DIO2 RF-switch** (`SX1262RadioLib.h`): Added `radio.setDio2AsRfSwitch(true)` after `beginFSK()`; required for the Nesso N1 antenna path (missing call caused no RF output).
- **I/O expander antenna switch** (`ESP32Utils.h`): `enableRadioModule()` now triggers the BSP lazy-init of the PI4IOE5V6408 expander (E0, 0x43) via a dummy `pinMode(LORA_ENABLE, ...)` call *before* writing registers directly over I2C. Without this ordering, a later BSP call would software-reset E0 and deassert `LORA_ANTENNA_SWITCH`, causing ~40 dB attenuation.
- **`RADIO_RESET` value** (`BoardConfig.h`): Corrected from `-2` to `-1` (`RADIOLIB_NC`); the wrong value was passed to RadioLib's `Module` constructor.
- **Antenna loss compensation** (`BoardConfig.h`): Increased `ANTENNA_LOSS_DBM` from 6 to 8 dBm based on measured path loss on the production Nesso N1 PCB.

## [0.1.1] - 2026-01-14

### Fixed
- library.properties update

## [0.1.0] - 2026-01-08

### Added
- Initial public release of OpenMioty library
- Full uplink transmission support for ETSI TS 103 357 TS-UNB (mioty®)
- ESP32-C6 platform support with hardware timer integration
- SX1262 radio driver via RadioLib
- Regional profiles: EU0, EU1, EU2 (868 MHz), US0 (915 MHz), IN0 (865 MHz)
- Arduino Nesso N1 board configuration
- AES-128 encryption for network security
- Simple one-liner API: `TsUnb::EU1 miotyNode;`
- basicTransmit example sketch for the Arduino Nesso
- Comprehensive documentation (README, ARCHITECTURE, LICENSE, NOTICE)
- Arduino Library Manager compatible structure

### Technical Details
- Based on Fraunhofer TS-UNB-Lib reference implementation
- Clean layered architecture: Application → Regional → Driver → Board → Platform
- Hardware-timed symbol transmission (< 20 ppm accuracy)
- Telegram splitting with frequency hopping
- Dependencies: RadioLib >= 6.0.0, ESP32 Arduino Core

### Known Limitations
- Uplink only (no downlink support)
- No duty cycle enforcement (user responsible for regulatory compliance)
- Academic/non-commercial use only (patent licensing required for commercial use)
- Limited platform support (ESP32 only in v0.1.0)

### License
Third-Party Modified Version of the Fraunhofer TS-UNB-Lib
- Academic and non-commercial use only
- Commercial use requires patent licenses from Sisvel International S.A.
- See LICENSE.md and NOTICE.md for complete terms

---

## Release Notes

For detailed release information, see [GitHub Releases](https://github.com/phaseshifted-iot/open-mioty-arduino/releases).

[0.1.2]: https://github.com/phaseshifted-iot/open-mioty-arduino/releases/tag/v0.1.2
[0.1.1]: https://github.com/phaseshifted-iot/open-mioty-arduino/releases/tag/v0.1.1
[0.1.0]: https://github.com/phaseshifted-iot/open-mioty-arduino/releases/tag/v0.1.0
