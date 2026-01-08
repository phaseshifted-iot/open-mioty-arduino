# NOTICE - Third-Party Modified Software

---

## ⚠️ THIS IS NOT OFFICIAL FRAUNHOFER SOFTWARE

**open-mioty-arduino** is a third-party, community-driven modification of the Fraunhofer TS-UNB-Lib reference implementation. This project is:

- **NOT** affiliated with Fraunhofer Institute for Integrated Circuits IIS
- **NOT** affiliated with Sisvel International S.A.
- **NOT** endorsed or supported by Fraunhofer or Sisvel
- **NOT** the same quality as commercially-licensed MIOTY software
- **NOT** production-ready or certified for commercial deployment

## Status and Intended Use

### ✅ This Software IS:
- An **educational adaptation** of Fraunhofer's reference implementation
- **Open-source** for academic and non-commercial use
- Suitable for **testing, demonstration, and learning purposes**
- A **development tool** for proof-of-concept projects
- **For hobby and experimental use** only

### ❌ This Software IS NOT:
- **NOT production-ready** or suitable for commercial deployment
- **NOT certified** or validated for regulatory compliance
- **NOT commercially licensed** (patent licensing required for production)
- **NOT equivalent** to Fraunhofer's commercial MIOTY software
- **NOT supported** by Fraunhofer or Sisvel

## Legal Requirements

### Patent Licensing Required for Commercial Use
Commercial use of MIOTY technology requires patent licenses:
- **Contact**: Sisvel International S.A.
- **Website**: https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms

### Commercial-Grade Software Available
For production-ready, commercially-licensed MIOTY software:
- **Contact**: Fraunhofer Institute for Integrated Circuits IIS
- **Website**: https://www.iis.fraunhofer.de/en/ff/lv/lok/tech/mioty.html

## Disclaimer

**THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED. USE AT YOUR OWN RISK.**

The authors, contributors, and copyright holders shall not be liable for any claim, damages, or other liability arising from the use of this software.

---

# Third-Party Notices and Attributions

This library ("open-mioty-arduino") contains or depends on third-party software components. The following is a list of these components along with their respective licenses and copyright information.

---

## 1. Fraunhofer TS-UNB-Lib

**Copyright:** (c) 2019 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V.

**License:** Fraunhofer TS-UNB-Lib License (see LICENSE.md)

**Used Components:**
- `src/core/TsUnb/` - Physical Layer (PHY), MAC Layer, SimpleNode, RadioBurst
- `src/core/Encryption/` - AES-128 encryption
- `src/core/Utils/` - Bit access utilities

**Source:** Based on reference implementation for AVR microcontrollers

**Contact:** 
- Fraunhofer Institute for Integrated Circuits IIS
- Division Communication Systems
- Am Wolfsmantel 33, 91058 Erlangen, Germany
- ks-contracts@iis.fraunhofer.de

**Important Notes:**
- This is a **Third-Party Modified Version** adapted for Arduino Nesso N1 (ESP32-C6)
- For academic and non-commercial use only
- Commercial use requires patent licenses from Sisvel International S.A.
- See: https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms

---

## 2. RadioLib

**Copyright:** (c) 2018 Jan Gromeš

**License:** MIT License (see LICENSE-RADIOLIB.md)

**Repository:** https://github.com/jgromes/RadioLib

**Used For:** SX1262 radio transceiver driver functionality

**Integration:** This library uses RadioLib as a dependency for low-level radio communication with the SX1262 chip.

---

## 3. ESP32 Arduino Core

**Copyright:** Espressif Systems

**License:** LGPL 2.1

**Repository:** https://github.com/espressif/arduino-esp32

**Used For:** ESP32-C6 platform support, SPI, GPIO, Timer functionality

---

## 4. MIOTY Standard

**Standard:** ETSI TS 103 357 - TS-UNB (Telegram Splitting Ultra Narrowband)

**Trademark:** "MIOTY" is a registered trademark

**Patent Information:** 
- The MIOTY/TS-UNB protocol is covered by patents
- Patent licenses available through Sisvel International S.A.
- https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms

**Standards Body:** ETSI (European Telecommunications Standards Institute)

---

## License Compatibility

This library uses a combination of licenses:

1. **Fraunhofer TS-UNB-Lib License** - Core protocol implementation (restrictive, academic/non-commercial)
2. **MIT License** - RadioLib dependency (permissive)
3. **LGPL 2.1** - ESP32 Arduino Core (permissive with copyleft for modifications)

**Overall Effect:** The most restrictive license (Fraunhofer TS-UNB-Lib) applies to this combined work. This means:
- ✅ Free for academic and non-commercial use
- ❌ Commercial use requires separate patent licenses
- ✅ Source code must remain available
- ✅ Modifications must be clearly marked
- ⚠️ Patent licenses needed for commercial MIOTY implementations

---

## Acknowledgments

Special thanks to:
- **Fraunhofer IIS** for developing and open-sourcing the TS-UNB reference implementation
- **Jan Gromeš** for creating and maintaining RadioLib
- **Espressif Systems** for ESP32 platform support
- **ETSI** for standardizing the MIOTY/TS-UNB protocol
- The open-source community for tools and support

---

## Disclaimer

This library is provided "AS IS" without warranty of any kind. Users are responsible for:
- Complying with all applicable licenses
- Obtaining necessary patent licenses for commercial use
- Following regional radio regulations (FCC, ETSI, etc.)
- Ensuring proper use of radio spectrum

For questions about licensing or commercial use, contact the respective copyright holders directly.
