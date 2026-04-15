# AGENTS.md — OpenMioty Arduino Library

## Mandatory Reading Order

Before working on this codebase, read these files in order:

| # | File | When to read |
|---|------|--------------|
| 1 | This file | Always |
| 2 | [ARCHITECTURE.md](ARCHITECTURE.md) | Always — deep-dive on the 4-layer design |
| 3 | [CONTRIBUTING.md](CONTRIBUTING.md) | When adding features or platforms |
| 4 | [.github/skills/nesso-n1-platform.md](.github/skills/nesso-n1-platform.md) | When working on the Arduino Nesso N1 |
| 5 | [.github/skills/demokit-rp2040-platform.md](.github/skills/demokit-rp2040-platform.md) | When working on the open-source demokit (RP2040) |

---

## Project Identity

**OpenMioty** is an Arduino library that implements mioty® (ETSI TS 103 357 / TS-UNB) — a long-range, low-power LPWAN protocol based on Telegram Splitting Ultra-Narrowband (TS-UNB). It is a third-party modified version of Fraunhofer's TS-UNB-Lib.

| Attribute | Value |
|-----------|-------|
| Version | 0.1.1 (experimental) |
| Protocol | mioty® / ETSI TS 103 357 / TS-UNB |
| Direction | **Uplink only** (no downlink/RX windows yet) |
| Payload | 20–255 bytes |
| Range | Up to 15 km (line of sight) |
| Regions | EU (868 MHz), US (915 MHz), India (865 MHz) |
| License | Academic / non-commercial only |
| Primary dependency | [RadioLib](https://github.com/jgromes/RadioLib) ≥ 6.0.0 |

---

## Architecture Overview

The library uses a **4-layer template hierarchy** resolved entirely at compile time — no virtual functions, no runtime overhead.

```
User Sketch
    │
    ▼
SimpleNode<MAC, PHY, TX>          ← High-level API: init(), send()
    │
    ├─ FixedUplinkMac              ← AES-128 CTR encryption + CMAC MIC
    ├─ Phy<profile params>         ← Convolutional encoding, TSMA hopping, burst assembly
    └─ SX1262RadioLib / Rfm69hw   ← Radio chip driver (board pins injected via templates)
           │
           └─ ESP32TsUnb / RP2040TsUnb    ← MCU platform: SPI, 20 MHz hardware timer
```

The entry point for user code is `OpenMioty.h`, which exposes pre-built type aliases:

```cpp
#include <OpenMioty.h>

TsUnb::EU1 node;   // Arduino Nesso N1, 868 MHz EU dual-channel
TsUnb::US0 node;   // Arduino Nesso N1, 915 MHz US dual-channel
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for the full layer description, template instantiation details, and porting guide.

---

## Directory Map

```
open-mioty-arduino/
│
├── AGENTS.md                        ← This file (reading order + project structure)
├── ARCHITECTURE.md                  ← Layer design, porting guide, template API
├── CONTRIBUTING.md                  ← Contribution workflow, coding standards
├── CHANGELOG.md                     ← Version history
├── README.md                        ← User-facing quickstart
├── library.properties               ← Arduino Library Manager metadata
├── keywords.txt                     ← Arduino IDE syntax highlighting
│
├── src/
│   ├── OpenMioty.h                  ← Single include for users; exposes TsUnb namespace
│   │
│   ├── core/
│   │   ├── Encryption/
│   │   │   └── Aes128.h             ← AES-128 block cipher + CMAC (FIPS 197, PROGMEM-safe)
│   │   └── TsUnb/
│   │       ├── SimpleNode.h         ← Top-level API wrapper (init, send, address setters)
│   │       ├── FixedMac.h           ← MAC layer: header, address, counter, AES-CTR, CMAC MIC
│   │       ├── Phy.h                ← PHY layer: CC encoding, whitening, burst assembly, TSMA
│   │       └── RadioBurst.h         ← Single 36-symbol burst container (head + payload + tail)
│   │
│   ├── drivers/
│   │   ├── SX1262RadioLib.h         ← SX1262 driver via RadioLib (functional, FSK mode)
│   │   └── Rfm69hw.h                ← RFM69HW driver (SKELETON — burst TX not yet implemented)
│   │
│   ├── platform/
│   │   ├── PlatformInterface.h      ← Abstract interface all platforms must implement
│   │   ├── esp32/
│   │   │   ├── ESP32TsUnb.h         ← ESP32 platform: 20 MHz XTAL timer, SPI, initTimer/waitTimer
│   │   │   ├── ESP32TsUnb.cpp       ← ESP32 platform impl
│   │   │   ├── ESP32Utils.h         ← EUI64, radio enable, DeferredTaskQueue (ISR-safe task queue)
│   │   │   └── nesso-n1/
│   │   │       ├── BoardConfig.h    ← Nesso N1 pin map, SPI config, ANTENNA_LOSS_DBM, timing PPM
│   │   │       └── Templates.h      ← Pre-built type aliases per region (TsUnb_EU1, TsUnb_US0, …)
│   │   └── rp2040/
│   │       └── opensource-demokit-phaseshifted/
│   │           └── (empty)          ← NOT YET IMPLEMENTED — target for demokit board config
│   │
│   └── regional/
│       └── RegionalProfiles.h       ← EU0, EU1, EU2, US0, IN0: center freq, power, B_c, duty cycle
│
└── examples/
    ├── basicTransmitNesso/
    │   └── basicTransmitNesso.ino   ← Minimal transmit example (Serial output only)
    └── basicTransmitNessoDisplay/
        ├── basicTransmitNessoDisplay.ino  ← Transmit + display countdown
        ├── Display.h                ← M5GFX display wrapper
        └── DisplayConfig.h          ← Screen dimensions, brand colors
```

---

## Supported Platforms

| Platform | Board | Radio | Status | Skills |
|----------|-------|-------|--------|--------|
| ESP32-C6 | Arduino Nesso N1 | SX1262 | **Functional (experimental)** | [nesso-n1-platform.md](.github/skills/nesso-n1-platform.md) |
| RP2040 | phaseshifted open-source demokit | RFM69(H)CW | **Skeleton only** | [demokit-rp2040-platform.md](.github/skills/demokit-rp2040-platform.md) |

---

## Key Implementation Constraints

These constraints are **non-negotiable** — violating them will either break timing or corrupt transmissions:

### 1. Transmission is blocking (~1.7 s)
`node.send(data, len)` does not return until the entire telegram is transmitted. All display updates, ISR actions, and sensor reads must happen **before** or **after** the call — never during.

### 2. Timing precision < 20 ppm
The ESP32 platform uses a 40 MHz external crystal divided to 20 MHz (50 ns resolution) for symbol timing. Do not use `Serial.print()`, `Wire.*`, or `delay()` inside ISRs during transmission. Use `DeferredTaskQueue` (in `ESP32Utils.h`) to defer work.

### 3. Configuration is compile-time only
All parameters (SPI pins, TX power, frequencies, symbol rate) are resolved via C++ templates. There is no runtime configuration API. Changing the regional profile or board requires a recompile.

### 4. Shared SPI bus on Nesso N1
The SX1262 radio and the ST7789 display share the same SPI bus (SCK=GPIO20, MISO=GPIO22, MOSI=GPIO21). They are separated by chip select (radio CS=GPIO23, display CS=GPIO5). The code must always deselect one before activating the other.

### 5. LoRa control pins are on an I/O expander (Nesso N1)
`LORA_ENABLE`, `LORA_ANTENNA_SWITCH`, and `LORA_LNA_ENABLE` are not direct GPIO — they are on a PI4IOE5V6408 I/O expander and require the `Arduino_Nesso_N1` BSP library. See the platform skill for correct initialization.

---

## Build Requirements

| Requirement | Version | Notes |
|-------------|---------|-------|
| Arduino IDE | ≥ 2.0 | or PlatformIO |
| `esp32` board package by Espressif | ≥ 3.3.5 | Required for Nesso N1 I/O expander support |
| `RadioLib` | ≥ 6.0.0 | SX1262 driver backend |
| `Arduino_Nesso_N1` | ≥ 1.0.0 | Required for I/O expander pins and battery management |
| `M5GFX` | ≥ 0.2.17 | Required for display examples only |

---

## Quick Usage Reference

```cpp
#include <OpenMioty.h>

// 1. Pick your region (TsUnb::EU0, EU1, EU2, US0, IN0)
TsUnb::EU1 miotyNode;

const uint8_t networkKey[16] = { 0x01, 0x02, ... };
const uint8_t eui64[8]       = { 0xDE, 0xAD, ... };

void setup() {
    miotyNode.init();
    miotyNode.setNetworkKey(networkKey);
    miotyNode.setEUI64(eui64);
    miotyNode.setShortAddress(0x1234);
}

void loop() {
    uint8_t data[] = { 0xAA, 0xBB, 0xCC };
    miotyNode.send(data, sizeof(data));  // Blocks ~1.7 s
    delay(60000);
}
```

See [examples/basicTransmitNesso/](examples/basicTransmitNesso/) for the complete minimal example, and [examples/basicTransmitNessoDisplay/](examples/basicTransmitNessoDisplay/) for the display + cooperative scheduling pattern.

---

## What Agents Should NOT Do

- Do **not** add runtime config APIs (breaks compile-time template design).
- Do **not** call `Serial.print()` or I2C operations inside ISRs or during `send()`.
- Do **not** assume the RP2040/demokit platform works — it is a skeleton.
- Do **not** use `analogWrite()` / `tone()` during transmission (timer conflicts on Nesso N1).
- Do **not** read `LORA_ENABLE`, `LORA_ANTENNA_SWITCH`, or `LORA_LNA_ENABLE` as direct GPIO — they are on the I/O expander.
