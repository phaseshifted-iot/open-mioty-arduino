# Skill: Arduino Nesso N1 Platform

**Scope:** Hardware and software knowledge specific to the Arduino Nesso N1 + the `open-mioty-arduino` library configuration for that board.

**Use this skill when:**
- Writing or debugging code that targets the Arduino Nesso N1
- Configuring SPI, radio pins, or the display
- Porting or modifying `src/platform/esp32/nesso-n1/`
- Troubleshooting LoRa / SX1262 initialization failures

---

## 1. Hardware Summary

| Component | Part | Notes |
|-----------|------|-------|
| SoC | ESP32-C6 | Single-core RISC-V, up to 160 MHz, 512 kB SRAM, 16 MB Flash |
| LoRa radio | Semtech SX1262 | 850–960 MHz FSK/LoRa, controlled via SPI |
| Display | 1.14" IPS (135×240 px) | ST7789 controller, SPI; M5GFX-compatible |
| Touch | FT6336U | I2C (on primary `Wire` bus) |
| IMU | Bosch BMI270 | 6-axis accel + gyro, I2C |
| Battery | 250 mAh LiPo | AW32001 power path + BQ27220 fuel gauge |
| Power management chips | AW32001, BQ27220 | Requires `Arduino_Nesso_N1` library ≥ 1.0.0 |
| I/O expanders | 2× PI4IOE5V6408 | I2C addresses 0x43 (E0) and 0x44 (E1) |
| RF switch | FM8625 | Controlled via `LORA_ANTENNA_SWITCH` (E0.P6) |
| Receive LNA | SGM1300 | Controlled via `LORA_LNA_ENABLE` (E0.P5) |
| User buttons | KEY1 (E0.P0), KEY2 (E0.P1) | Via I/O expander — requires BSP library |
| Onboard LED | LED_BUILTIN (E1.P7) | Green, **inverted logic** (LOW = ON) |
| Buzzer | GPIO11 (BEEP_PIN) | Passive buzzer, use `tone()` |
| IR TX | GPIO9 (IR_TX_PIN) | Timer conflict with `tone()` — cannot use both simultaneously |

---

## 2. SPI Pin Mapping

The Nesso N1 uses **non-default SPI pins**. These are shared between the SX1262 radio and the ST7789 display.

| Signal | GPIO | Notes |
|--------|------|-------|
| SCK | GPIO 20 | Non-default (ESP32-C6 default is GPIO 6) |
| MISO | GPIO 22 | Non-default |
| MOSI | GPIO 21 | Non-default |
| Radio CS | GPIO 23 | `LORA_CS` — direct GPIO |
| Display CS | GPIO 17 | `LCD_CS` — direct GPIO |

**Critical:** This is a **shared SPI bus**. Both devices share SCK/MISO/MOSI. You must deselect one CS before activating the other. The OpenMioty board config handles this for the radio; the display must call `display.ensureDeselected()` or equivalent after drawing.

These pins are set in [src/platform/esp32/nesso-n1/BoardConfig.h](../../src/platform/esp32/nesso-n1/BoardConfig.h):

```cpp
SPI_SCK   = 20
SPI_MISO  = 22
SPI_MOSI  = 21
RADIO_CS  = 23
SPI_CLOCK_HZ = 4000000  // 4 MHz — conservative for shared bus
```

---

## 3. SX1262 Radio Pins

| Signal | GPIO / Expander | Type | Notes |
|--------|----------------|------|-------|
| `LORA_CS` | GPIO 23 | Direct GPIO | SPI chip select |
| `LORA_IRQ` / DIO1 | GPIO 15 | Direct GPIO | Interrupt request |
| `LORA_BUSY` | GPIO 19 | Direct GPIO | Module busy indicator |
| `LORA_ENABLE` (RESET) | E0.P7 | I/O expander | HIGH = enable, LOW = reset module |
| `LORA_ANTENNA_SWITCH` | E0.P6 | I/O expander | HIGH for normal operation (powers FM8625 RF switch VDD) |
| `LORA_LNA_ENABLE` | E0.P5 | I/O expander | HIGH to enable SGM1300 LNA on RX path |

### Initialization Sequence (Arduino BSP pattern)

```cpp
#include <Arduino_Nesso_N1.h>
#include <RadioLib.h>

// LORA_ENABLE is on the I/O expander — pass RADIOLIB_NC to RadioLib constructor
SX1262 radio = new Module(LORA_CS, LORA_IRQ, RADIOLIB_NC, LORA_BUSY);

void setup() {
    // IMPORTANT: Use direct I2C atomic writes for the radio reset sequence.
    // The BSP's digitalWrite(LORA_ENABLE, ...) uses read-modify-write (RMW).
    // When LORA_ANTENNA_SWITCH is set up later, that RMW reads stale expander
    // state and silently clears LORA_ENABLE's output-enable bit, making NRESET
    // permanently float HIGH (SX1262 never gets a clean reset → BUSY stuck HIGH).
    //
    // PI4IOE5V6408 E0 (0x43) registers: 0x03=output-enable, 0x05=value, 0x07=high-Z
    Wire.begin();
    auto writeE0 = [](uint8_t reg, uint8_t val) {
        Wire.beginTransmission(0x43); Wire.write(reg); Wire.write(val); Wire.endTransmission();
    };
    // One BSP call to trigger expander init and set _initialized[0]=true
    pinMode(LORA_ENABLE, OUTPUT);
    // Atomic one-shot: P7=LORA_ENABLE, P6=ANTENNA_SWITCH, P5=LNA_ENABLE
    constexpr uint8_t RP = (1<<7)|(1<<6)|(1<<5);
    writeE0(0x03, RP);         // output-enable
    writeE0(0x07, ~RP & 0xFF); // push-pull
    writeE0(0x05, 0x00);       // NRESET asserted (LOW)
    delay(100);
    writeE0(0x05, RP);         // NRESET released + LNA + antenna switch HIGH

    // Initialize radio — RadioLib handles BUSY polling internally
    radio.begin(868.0, 125.0, 9, 7, 0x12, 10, 8, 3.0, true);  // tcxoVoltage=3.0 for DIO3 TCXO
    radio.setDio2AsRfSwitch(true);  // MANDATORY — controls Tx/Rx path automatically
}
```

**Why `RADIOLIB_NC` for reset?** `LORA_ENABLE` is an I/O expander pin. RadioLib's reset logic requires a direct GPIO it can toggle. Since the expander pin is accessed via the `Arduino_Nesso_N1` BSP (not raw `pinMode`/`digitalWrite` on a GPIO number), it cannot be passed to the RadioLib `Module(...)` constructor. Control the reset manually before calling `radio.begin()`.

### OpenMioty Board Config Pattern

Inside `OpenMioty`, the board config does this automatically. The values are in [BoardConfig.h](../../src/platform/esp32/nesso-n1/BoardConfig.h):

```cpp
RADIO_CS      = 23
RADIO_DIO1    = 15
RADIO_RESET   = -1   // RADIOLIB_NC: int8_t(-1) sign-extends to uint32_t(0xFFFFFFFF)
RADIO_BUSY    = 19
ANTENNA_LOSS_DBM = 6  // +6 dBm compensation (Nesso antenna is lossy)
TIMING_OFFSET_PPM = 0
```

---

## 4. BSP Library Requirements

The `Arduino_Nesso_N1` library (≥ 1.0.0) **must** be installed when:
- Using any I/O expander pin (`KEY1`, `KEY2`, `LED_BUILTIN`, `LORA_ENABLE`, `LORA_ANTENNA_SWITCH`, `LORA_LNA_ENABLE`, `LCD_RESET`, `LCD_BACKLIGHT`)
- Using battery management (`NessoBattery`)
- Using the display (`NessoDisplay`) or touch (`NessoTouch`)

**Install:** Arduino IDE → Tools → Manage Libraries → search "Arduino_Nesso_N1"

---

## 5. I/O Expander Pinout Reference

### Expander E0 (address 0x43)

| Pin | Symbol | Direction | Function |
|-----|--------|-----------|----------|
| P0 | KEY1 | Input | Programmable user button 1 |
| P1 | KEY2 | Input | Programmable user button 2 |
| P5 | LORA_LNA_ENABLE | Output | SGM1300 LNA enable (HIGH = on) |
| P6 | LORA_ANTENNA_SWITCH | Output | FM8625 RF switch VDD power (HIGH = on) |
| P7 | LORA_ENABLE | Output | SX1262 reset/enable (HIGH = running) |

### Expander E1 (address 0x44)

| Pin | Symbol | Direction | Function |
|-----|--------|-----------|----------|
| P0 | POWEROFF | Output | System power-off control |
| P1 | LCD_RESET | Output | Display reset |
| P2 | GROVE_POWER_EN | Output | Grove connector power enable |
| P5 | VIN_DETECT | Input | External VIN power detection |
| P6 | LCD_BACKLIGHT | Output | Display backlight control |
| P7 | LED_BUILTIN | Output | Onboard green LED (inverted: LOW = ON) |

**⚠ I2C address conflict:** Expander E1 occupies 0x44. The **Modulino Thermo** module also uses 0x44 — it **cannot** be connected via the Qwiic port on this board.

---

## 6. Other GPIO Pins (Direct ESP32-C6)

| Signal | GPIO | Notes |
|--------|------|-------|
| SDA (primary I2C) | GPIO 10 | Shared with Qwiic connector |
| SCL (primary I2C) | GPIO 8 | Shared with Qwiic connector |
| SYS_IRQ | GPIO 3 | Combined interrupt from IMU + I/O expander |
| BEEP_PIN | GPIO 11 | Passive buzzer |
| IR_TX_PIN | GPIO 9 | IR transmitter |
| GROVE_IO_0 | GPIO 5 | Grove connector, also ADC |
| GROVE_IO_1 | GPIO 4 | Grove connector, also ADC |
| D1 | GPIO 7 | 8-pin expansion header (PWM) |
| D2 | GPIO 2 | 8-pin expansion header (PWM) |
| D3 | GPIO 6 | 8-pin expansion header (PWM) |

**All GPIO are 3.3 V only — not 5 V tolerant.**

---

## 7. Display Usage with OpenMioty

The display (ST7789, controlled via `NessoDisplay` which extends M5GFX) shares the SPI bus with the radio. The key rule for safe co-operation with mioty transmission:

```
BEFORE send():  update display freely
DURING send():  do NOT touch SPI (send() is blocking, ~1.7 s)
AFTER send():   update display freely
```

Always call `display.ensureDeselected()` (or `endWrite()`) after drawing so the display CS is deasserted before the radio driver takes control of the bus.

```cpp
display.showSending(counter);     // Last display op before TX
miotyNode.send(data, len);        // ~1.7 s blocking TX
display.showIdle(newCounter);     // First display op after TX
```

---

## 8. ESP32 Platform Layer (Timing)

The mioty PHY requires symbol timing accuracy < 20 ppm (symbol period ≈ 420 µs). The ESP32 platform achieves this via:

- **Clock source:** 40 MHz external crystal (`TIMER_XTAL_CLK`), not the APB clock (which has jitter)
- **Timer divider:** 2 → 20 MHz effective clock → 50 ns per tick
- **Compile-time symbol duration:** `1 000 000 / 2380.371 Hz × (1 + PPM × 1e-6)` µs

**Never call these during or between radio bursts:**
- `Serial.print()` / `Serial.println()` (100 µs–10 ms per call)
- `Wire.beginTransmission()` / `Wire.endTransmission()` (I2C blocking)
- `delay()` or `delayMicroseconds()` (overrides timing)
- Any `analogRead()` / ADC access

Use `DeferredTaskQueue` from [ESP32Utils.h](../../src/platform/esp32/ESP32Utils.h) to queue work from ISRs for execution after the current `send()` call returns.

---

## 9. Power Supply Options

| Source | How | Notes |
|--------|-----|-------|
| USB-C | 5 V via USB-C port | Also charges battery |
| LiPo battery | Onboard 250 mAh | Autonomous operation |
| VIN pin | 5 V on 8-pin expansion header pin 8 | External supply, also charges battery |
| BATTERY OUT pin | Raw LiPo voltage on pin 6 | Unregulated (3.0–4.2 V), use with caution |

---

## 10. OpenMioty Template Aliases (Nesso N1)

These are defined in [Templates.h](../../src/platform/esp32/nesso-n1/Templates.h) and re-exported from [OpenMioty.h](../../src/OpenMioty.h):

| Alias | Region | Frequencies | TX Power |
|-------|--------|-------------|----------|
| `TsUnb::EU0` | Europe | 868.180 MHz (single channel) | 14 dBm |
| `TsUnb::EU1` | Europe (recommended) | 868.180 / 868.080 MHz | 14 dBm |
| `TsUnb::EU2` | Europe (wide) | 867.625 / 866.825 MHz | 14 dBm |
| `TsUnb::US0` | North America | 916.400 / 915.600 MHz | 22 dBm |
| `TsUnb::IN0` | India | 866.875 / 866.775 MHz | 22 dBm |

---

## 11. Common Gotchas

| Issue | Root Cause | Fix |
|-------|-----------|-----|
| Radio init fails after reset | `LORA_ENABLE` not driven HIGH before `radio.begin()` | Set `LORA_ENABLE HIGH` with 100 ms delay before init |
| No receive even when close | `LORA_LNA_ENABLE` or `LORA_ANTENNA_SWITCH` not set HIGH | Both must be HIGH for RX path to work |
| Display freezes after TX | SPI CS not released before radio takes bus | Call `ensureDeselected()` before `send()` |
| `LED_BUILTIN` backwards | Inverted logic on expander | `LOW` = on, `HIGH` = off |
| `tone()` and IR conflict | Timer sharing | Release pin with `pinMode(pin, INPUT)` and re-call `IrSender.begin()` before each IR send |
| I2C device not found at 0x44 | Using Modulino Thermo via Qwiic | Address conflict with expander E1 — not compatible |
| Compilation error for expander pins | Missing `Arduino_Nesso_N1` library | Install via Library Manager |

---

## 12. External References

- [Arduino Nesso N1 User Manual](https://docs.arduino.cc/tutorials/nesso-n1/user-manual/)
- [M5Stack LoRa Tutorial (Nesso N1)](https://docs.m5stack.com/en/arduino/arduino_nesso_n1/lora)
- [RadioLib SX1262 Docs](https://jgromes.github.io/RadioLib/class_s_x126x.html)
- [Arduino_Nesso_N1 Library (Library Manager)](https://www.arduino.cc/reference/en/libraries/)
