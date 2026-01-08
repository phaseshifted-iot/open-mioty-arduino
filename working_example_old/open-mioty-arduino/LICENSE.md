# Software License for the Third-Party Modified Version of the Fraunhofer TS-UNB-Lib

---

## ⚠️ IMPORTANT NOTICES - READ FIRST

**THIS IS A THIRD-PARTY MODIFIED VERSION** of the Fraunhofer TS-UNB-Lib, created independently for educational and testing purposes.

### Status and Intended Use

- ✅ **Open-Source**: Available for academic research and non-commercial use
- ✅ **Testing & Demonstration**: Suitable for protocol evaluation and development
- ❌ **NOT Production-Ready**: Not intended for commercial deployment
- ❌ **NO Commercial License**: Patent licensing required for commercial use
- ❌ **NO Quality Guarantees**: Not equivalent to Fraunhofer's commercial MIOTY software

### What This Library IS:
- An educational Arduino adaptation of Fraunhofer's TS-UNB reference implementation
- A platform for learning and experimenting with the MIOTY protocol
- A development tool for proof-of-concept projects
- Open-source software for academic research

### What This Library IS NOT:
- NOT a production-ready commercial product
- NOT certified or validated for regulatory compliance
- NOT endorsed or supported by Fraunhofer or Sisvel
- NOT suitable for commercial deployment without proper licensing

---

(c) Copyright 2019 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V. All rights reserved.

## 1. INTRODUCTION

The Fraunhofer Telegram Splitting - Ultra Narrowband Library ("TS-UNB-Lib") is software that implements only the uplink of the ETSI TS 103 357 TS-UNB standard ("MIOTY") for wireless data transmission in the field of IoT. Patent licenses for any patent claim regarding the ETSI TS 103 357 TS-UNB standard implementation (including those of Fraunhofer) may be obtained through Sisvel International S.A. (https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms) or through the respective patent owners individually. 

**The purpose of this TS-UNB-Lib is academic and non-commercial use.** Therefore, Fraunhofer does not offer any support for the TS-UNB-Lib. Furthermore, the TS-UNB-Lib is NOT identical and on the same quality level as the commercially-licensed MIOTY software also available from Fraunhofer. Users are encouraged to check the Fraunhofer website for additional applications information and documentation.

## 2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification, are permitted without payment of copyright license fees provided that you satisfy the following conditions: You must retain the complete text of this software license in redistributions of the TS-UNB-Lib software or your modifications thereto in source code form. You must retain the complete text of this software license in the documentation and/or other materials provided with redistributions of the TS-UNB-Lib software or your modifications thereto in binary form. You must make available free of charge copies of the complete source code of the TS-UNB-Lib software and your modifications thereto to recipients of copies in binary form. The name of Fraunhofer may not be used to endorse or promote products derived from this software without prior written permission. You may not charge copyright license fees for anyone to use, copy or distribute the TS-UNB-Lib software or your modifications thereto. Your modified versions of the TS-UNB-Lib software must carry prominent notices stating that you changed the software and the date of any change. For modified versions of the TS-UNB-Lib software, the term "Fraunhofer TS-UNB-Lib" must be replaced by the term "Third-Party Modified Version of the Fraunhofer TS-UNB-Lib."

## 3. NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without limitation the patents of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE. Fraunhofer provides no warranty of patent non-infringement with respect to this software. You may use this TS-UNB-Lib software or modifications thereto only for purposes that are authorized by appropriate patent licenses.

## 4. DISCLAIMER

This TS-UNB-Lib software is provided by Fraunhofer on behalf of the copyright holders and contributors "AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, including but not limited to the implied warranties of merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary, or consequential damages, including but not limited to procurement of substitute goods or services; loss of use, data, or profits, or business interruption, however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence), arising in any way out of the use of this software, even if advised of the possibility of such damage.

## 5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS  
Attention: Division Communication Systems  
Am Wolfsmantel 33  
91058 Erlangen, Germany  
ks-contracts@iis.fraunhofer.de

---

## MODIFICATIONS NOTICE

### This is a Third-Party Modified Version

**THIS SOFTWARE IS AN INDEPENDENT, THIRD-PARTY MODIFICATION** of the Fraunhofer TS-UNB-Lib reference implementation. It is **NOT** the original Fraunhofer software and is **NOT** affiliated with, endorsed by, or supported by Fraunhofer IIS or Sisvel International S.A.

**Modified by:** Independent contributors to the open-mioty-arduino project  
**Modification Date:** January 2026  
**Purpose:** Educational, testing, and demonstration use only

### Changes Made to Original Fraunhofer Code:
- Adapted for Arduino Nesso N1 (ESP32-C6 + SX1262 radio)
- Added ESP32 platform abstraction layer with hardware timer support
- Created RadioLib-based SX1262 driver wrapper
- Added regional profile templates for EU/US/IN frequency bands
- Reorganized into modular Arduino library structure
- Added Arduino-style API for ease of use

**Original Source:** [Fraunhofer TS-UNB-Lib-AVR](https://github.com/Fraunhofer-IIS-Wireless-Comms/TS-UNB-Lib-AVR) (reference implementation)

### Legal and Usage Status:

**This modified version:**
- ✅ Retains all original Fraunhofer copyright notices and license terms
- ✅ Is intended **ONLY** for academic, educational, and non-commercial use
- ✅ Is suitable for testing, demonstration, and protocol evaluation
- ❌ Is **NOT production-ready** or commercially licensed
- ❌ Is **NOT validated** for regulatory compliance
- ❌ Is **NOT certified** for use in commercial products
- ⚠️  Requires appropriate patent licenses from Sisvel for any MIOTY implementation
- ⚠️  Is provided **AS-IS without any warranty** of any kind

### Important Disclaimers:

**NO PRODUCTION USE**: This library is for testing and educational purposes only. It is NOT suitable for production deployment, commercial products, or mission-critical applications without proper validation, certification, and patent licensing.

**NO WARRANTIES**: This software is provided "AS IS" without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, and non-infringement.

**PATENT LICENSING REQUIRED**: Commercial use or production deployment of MIOTY technology requires patent licenses. Contact [Sisvel International S.A.](https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms) for licensing information.

**NOT FRAUNHOFER QUALITY**: This modified library is NOT equivalent in quality, validation, or certification to the commercially-licensed MIOTY software available from Fraunhofer IIS.

### For Commercial Use or Production Deployment:

- **Patent Licensing:** Sisvel International S.A. - https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms
- **Commercial MIOTY Software:** Fraunhofer IIS - https://www.iis.fraunhofer.de/en/ff/lv/lok/tech/mioty.html
- **Technical Support:** Not available for this third-party modification

---

**Use this software at your own risk. This is a community-driven educational project, not a commercial product.**
