# Changelog

All notable changes to the OpenMioty Arduino library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

[0.1.1]: https://github.com/phaseshifted-iot/open-mioty-arduino/releases/tag/v0.1.1
[0.1.0]: https://github.com/phaseshifted-iot/open-mioty-arduino/releases/tag/v0.1.0
