# Skill: phaseshifted Open-Source Demokit (RP2040 + RFM69HW)

**Scope:** Hardware and software knowledge specific to the phaseshifted open-source mioty demokit — a custom breakout PCB for the Raspberry Pi Pico with a HopeRF RFM69(H)CW radio module.

**Use this skill when:**
- Porting OpenMioty to the demokit / RP2040 platform
- Implementing `src/platform/rp2040/` or `src/drivers/Rfm69hw.h`
- Ordering or assembling the demokit hardware
- Debugging hardware connections on the demokit

---

## 1. Hardware Summary

| Component | Part | Notes |
|-----------|------|-------|
| MCU | Raspberry Pi Pico | RP2040, dual ARM Cortex-M0+, 133 MHz, 264 kB SRAM, 2 MB Flash |
| Radio | HopeRF RFM69(H)CW | SPI-controlled FSK sub-GHz radio; RFM69HW = high-power (+20 dBm) variant |
| Form factor | Custom KiCAD breakout PCB | Pin-headers for Pico, footprint for RFM69(H)CW |
| Buttons | 2× SMD momentary (5.2×5.2 mm) | SW1 on GP2, SW2 on GP3 |
| Expansion | 2× Grove connectors | One 3.3 V I2C (GP4, GP5), one 5 V / analog (GP26, GP27) |
| Power keep-alive | 100 Ω resistor to GND | On GP15 — keeps power banks from shutting off |
| Antenna | SMA edge-mount or coil antenna | Connector: `Connector_Coaxial:SMA_Molex_73251-2120_EdgeMount_Horizontal` |

**PCB source:** [https://github.com/phaseshifted-iot/mioty-demokit](https://github.com/phaseshifted-iot/mioty-demokit)

---

## 2. Pin Mapping

This is the complete pinout from the demokit hardware design:

| Signal | Pico Physical Pin | RP2040 GPIO | Direction | Notes |
|--------|------------------|-------------|-----------|-------|
| RFM69HW SCK | 24 | GP18 | Output | SPI0 clock |
| RFM69HW MOSI | 25 | GP19 | Output | SPI0 data to radio |
| RFM69HW MISO | 21 | GP16 | Input | SPI0 data from radio |
| RFM69HW NSS | 22 | GP17 | Output | SPI chip select (active low) |
| RFM69HW RESET | 29 | GP22 | Output | Radio reset (active high pulse) |
| RFM69HW GND | 23 | GND | Power | |
| RFM69HW 3.3V | 36 | 3V3 | Power | |
| SW1 (Button 1) | 4 | GP2 | Input | Pull-up suggested |
| SW2 (Button 2) | 5 | GP3 | Input | Pull-up suggested |
| Grove I2C SDA (3.3 V) | 6 | GP4 | I/O | 3.3 V I2C connector |
| Grove I2C SCL (3.3 V) | 7 | GP5 | I/O | 3.3 V I2C connector |
| Grove I2C / Analog (5 V) | 31 | GP26 | I/O | 5 V Grove or ADC input |
| Grove I2C / Analog (5 V) | 32 | GP27 | I/O | 5 V Grove or ADC input |
| Power bank keep-alive | 20 | GP15 | Output | 100 Ω resistor to GND; low current draw keeps USB power banks active |

---

## 3. RFM69HW Radio Module

The HopeRF RFM69HW (high-power variant) is an FSK/OOK sub-GHz transceiver. Key characteristics relevant to mioty:

| Parameter | Value |
|-----------|-------|
| Frequency step resolution | 32 MHz / 2^19 = **61.03515625 Hz/LSB** |
| TX power range | −18 dBm to +20 dBm (PA_BOOST) |
| Frequency range | 433 / 868 / 915 MHz (band-specific variants) |
| SPI clock max | 10 MHz |
| Modulation | FSK (required for mioty TS-UNB) |
| Symbol rate register step | Fxosc / BitRate register value |

### Frequency Register Calculation

```cpp
// RFM69HW frequency register (3 bytes, big-endian)
// RegFreq = freq_Hz / (32_000_000 / 2^19) = freq_Hz / 61.03515625
uint32_t reg = (uint32_t)(freq_Hz / 61.03515625);
radio.setRegister(RF_FRFMSB, (reg >> 16) & 0xFF);
radio.setRegister(RF_FRFMID, (reg >>  8) & 0xFF);
radio.setRegister(RF_FRFLSB, (reg >>  0) & 0xFF);
```

This matches the `RFM69_FREQ_STEP_HZ = 61.03515625f` constant already defined in [RegionalProfiles.h](../../src/regional/RegionalProfiles.h), and the `B_c_RFM69 = 39` carrier spacing constant (39 × 61.03515625 ≈ 2380.37 Hz).

---

## 4. Implementation Status in OpenMioty

**The RP2040/demokit platform is a skeleton — it is not functional yet.**

| Component | File | Status | What is missing |
|-----------|------|--------|----------------|
| RP2040 platform layer | `src/platform/rp2040/` | **Empty directory** | Everything (timer, SPI, clock source) |
| Demokit board config | `src/platform/rp2040/opensource-demokit-phaseshifted/` | **Empty directory** | `BoardConfig.h`, `Templates.h` |
| RFM69HW radio driver | `src/drivers/Rfm69hw.h` | **Skeleton (~10%)** | Frequency register writes, burst TX loop, frequency hopping |

Do **not** attempt to use `TsUnb::*` type aliases targeting the RP2040 — they do not exist yet.

---

## 5. Porting Guide: What Needs Implementing

Implement in this order:

### Step 1: Board Config (`BoardConfig.h`)

Create `src/platform/rp2040/opensource-demokit-phaseshifted/BoardConfig.h` mirroring the Nesso N1 pattern from [src/platform/esp32/nesso-n1/BoardConfig.h](../../src/platform/esp32/nesso-n1/BoardConfig.h):

```cpp
struct BoardConfig {
    static constexpr int SPI_SCK       = 18;   // GP18
    static constexpr int SPI_MISO      = 16;   // GP16
    static constexpr int SPI_MOSI      = 19;   // GP19
    static constexpr int RADIO_CS      = 17;   // GP17
    static constexpr int RADIO_RESET   = 22;   // GP22
    static constexpr uint32_t SPI_CLOCK_HZ = 4000000;
    static constexpr int TIMING_OFFSET_PPM = 0;
    static constexpr int ANTENNA_LOSS_DBM  = 0;  // Tune after hardware testing
    static constexpr const char* BOARD_NAME  = "phaseshifted demokit";
    static constexpr const char* MCU_TYPE    = "RP2040";
    static constexpr const char* RADIO_TYPE  = "RFM69HW";
};
```

### Step 2: RP2040 Platform Layer (`RP2040TsUnb.h`)

The platform layer must implement the same interface as [ESP32TsUnb.h](../../src/platform/esp32/ESP32TsUnb.h). Key requirements:

| Method | Purpose | RP2040 Approach |
|--------|---------|----------------|
| `initTimer()` | Configure hardware timer for symbol timing | RP2040 hardware alarm (alarm pool) or PIO state machine |
| `startTimer()` | Arm timer for next burst | Set alarm target from `alarm_pool_add_alarm_at()` |
| `stopTimer()` | Cancel pending alarm | `alarm_pool_cancel_alarm()` |
| `addTimerDelay(count)` | Schedule next burst N ticks ahead | Accumulate tick count; use Pico SDK `time_us_64()` |
| `waitTimer()` | Block until burst window | Spin on alarm flag or use semaphore |
| `spiInit()` | Configure SPI0 with board pins | `spi_init()`, `gpio_set_function()` for GP16-19 |
| `spiSend(byte)` | Write byte to radio | `spi_write_blocking()` |
| `spiSendReceive(byte)` | Half-duplex SPI exchange | `spi_write_read_blocking()` |

**Timing precision requirement:** Symbol period = 1 000 000 / 2380.371 ≈ 420.168 µs. The RP2040 hardware timer has 1 µs resolution via the Pico SDK `time_us_64()` which is sufficient. Use the on-chip crystal (12 MHz XOSC), not the ring oscillator, as the clock source.

### Step 3: RFM69HW Driver (`Rfm69hw.h`)

The driver must match the interface of [SX1262RadioLib.h](../../src/drivers/SX1262RadioLib.h). Key implementation tasks:

```cpp
// Incomplete methods in src/drivers/Rfm69hw.h that need implementing:

// 1. init() — Configure RFM69HW for mioty FSK
//    - Set bit rate: 2.380371 kbps (Fxosc / BitRate_reg)
//    - Set frequency deviation: 600 Hz (hardware minimum)
//    - Disable CRC, sync word, DC-free encoding
//    - Set data shaping to NONE
//    - Set to fixed packet length mode
//    - Configure PA_BOOST for HW variant

// 2. transmit(RadioBurst* bursts, uint16_t f0_reg) — Burst TX loop
//    - For each burst:
//      a. waitTimer() — wait for burst window
//      b. Set frequency: f0 + carrier_offset (in RFM69 register units)
//      c. Write burst bytes to FIFO via SPI
//      d. Set mode to TX
//      e. Wait for PacketSent IRQ or poll register
//      f. Set mode to STANDBY
//      g. Schedule next burst: addTimerDelay(SYMBOL_DURATION * symbols_per_burst)
```

### Step 4: Templates and Type Aliases

After the above are complete, create `src/platform/rp2040/opensource-demokit-phaseshifted/Templates.h` and add `TsUnb::EU1`, `TsUnb::US0`, etc. aliased to the demokit backend. Then expose them from `OpenMioty.h` under a separate namespace or conditional compile guard (e.g., `#ifdef ARDUINO_ARCH_RP2040`).

---

## 6. Reference Implementations

Before implementing from scratch, study these existing implementations that use the same hardware:

| Project | URL | What it demonstrates |
|---------|-----|---------------------|
| TS-UNB-Lib-Pico | https://github.com/mioty-iot/TS-UNB-Lib-Pico | Official Fraunhofer TS-UNB library for Pico — RP2040 timer, SPI, and RFM69HW burst TX loop |
| mioty-pico-endpoint | https://github.com/phaseshifted-iot/mioty-pico-endpoint | This demokit in action with the TS-UNB-Lib-Pico |

The TS-UNB-Lib-Pico is the primary reference. Study its timer implementation and RFM69 FSK configuration before writing the OpenMioty RP2040 platform layer.

---

## 7. Ordering and Assembling the PCB

### Order the PCB

1. Clone or download `https://github.com/phaseshifted-iot/mioty-demokit`
2. Compress the contents of `/export` into a ZIP file
3. Upload to a PCB manufacturer (e.g., PCBWay — upload Gerber files). A 2-layer board will be detected. Default settings are acceptable.
4. Cost: approximately a few dollars per board in small quantities

### Bill of Materials

| Reference | Component | Package |
|-----------|-----------|---------|
| U1 | Raspberry Pi Pico | 2×20 pin 2.54 mm headers |
| U2 | HopeRF RFM69(H)CW | SMD radio module footprint |
| J1 | SMA antenna connector | `Connector_Coaxial:SMA_Molex_73251-2120_EdgeMount_Horizontal` |
| J2, J3 | Grove connectors | `Connector_JST:JST_PH_B4B-PH-K_1x04_P2.00mm_Vertical` |
| SW1, SW2 | Momentary push buttons | SMD 5.2×5.2 mm (`Button_Switch_SMD:SW_SPST_TL3342`) |
| R1 | 100 Ω resistor | 0805 |
| C1 | 100 nF decoupling capacitor | 0805 |
| C2 | 1 µF decoupling capacitor | 0805 |

### Antenna Options
- **Coil antenna** (compact, fixed): Solder to SMA connector or use wire whip
- **SMA external antenna** (better range): Attach via SMA edge-mount connector

---

## 8. External References

- [phaseshifted demokit hardware (KiCAD, Gerbers, photos)](https://github.com/phaseshifted-iot/mioty-demokit)
- [TS-UNB-Lib-Pico (Fraunhofer, reference impl)](https://github.com/mioty-iot/TS-UNB-Lib-Pico)
- [mioty-pico-endpoint (example firmware for this demokit)](https://github.com/phaseshifted-iot/mioty-pico-endpoint)
- [RFM69HW datasheet (HopeRF)](https://www.hoperf.com/modules/rf_transceiver/RFM69HW.html)
- [Raspberry Pi Pico datasheet & SDK](https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html)
