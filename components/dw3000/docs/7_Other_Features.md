# Chapter 7 — Other Features of the IC

> **Source:** DW3000 Family User Manual, Version 1.1 (© Decawave Ltd 2019, revised 28 May 2021)
> **Pages:** 66–70

---

## 7.1 External Synchronisation

Used to synchronise the DW3000 with external clocks, events, or with other DW3000 devices. Primary use case: **TDoA RTLS systems** with wired clock synchronisation of anchor nodes.

### Functions Provided

a) Reset the internal system timebase counter at a **deterministic time** relative to assertion of the SYNC input pin + external 38.4 MHz clock on EXTCLK pin

b) Initiate a packet transmission **deterministically** relative to SYNC pin assertion + EXTCLK

c) Synchronise **RX timestamping** to an external counter

### Hardware Interface

```
EXTCLK/XTI pin ─── 38.4 MHz external reference clock ──► DW3000
SYNC pin        ─── synchronisation pulse ────────────────► DW3000
```

- SYNC pin is **sampled on the rising edge of EXTCLK** — it must be source-synchronous with EXTCLK
- Refer to the DW3000 Datasheet for SYNC pin setup and hold times

When several DW3000 anchors share the same reference clock and SYNC signal, their internal timebases can be synchronised with **sub-nanosecond accuracy** (accounting for deterministic cable delay offsets via calibration).

### 7.1.1 One Shot Timebase Reset (OSTR) Mode

OSTR mode resets the DW3000 timebase counter at a precise, repeatable time relative to a synchronisation event.

**Accuracy:** reset repeatable to within **300 ps** (typically < 100 ps). The remaining deterministic error from process variation can be calibrated out to compensate for cable transmission delays in a wired sync system.

**Configuration:**
1. Set `OSTR_M` bit in `EC_CTRL` register (Register file `0x04` – External sync control)
2. Set `OSTS_WAIT` to the desired delay value
3. A counter running on the 38.4 MHz external clock counts up; when the count equals `OSTS_WAIT` on the rising edge of SYNC → DW3000 timebase counter is reset

At the moment SYNC is asserted, the **PLL dividers are also reset** to give a deterministic phase relationship between the 124.8 MHz system clock and the 38.4 MHz external clock.

**OSTS_WAIT value selection:**
- The value dictates the phase relationship between system clock and external clock
- Rule: `OSTS_WAIT modulo 4` must equal **1**
- **Recommended value: 33 decimal**
- Other valid values: 29, 37, 41, … (any value where modulo 4 = 1)

---

## 7.2 External Power Amplification

In some geographic regions, regulations permit transmitting **+20 dB above normal UWB regulation levels** (e.g. ETSI UWB rules for emergency first responders in the EU).

The DW3000 supports an external power amplifier via GPIO lines in a special mode:

- **GPIO signals used:** `EXTPA` (PA enable), `EXTTXE` (TX path enable), `EXTRXE` (RX path enable)
- Configure via the `GPIO_MODE` register
- Controls: external PA turn-on, and analog switching of TX/RX signal paths

> **Note:** Ensuring regulatory compliance is the customer's responsibility. A separate Decawave application note provides the complete circuit diagram and design considerations. Contact Decawave applications support for details.

---

## 7.3 Using the On-Chip OTP Memory

The DW3000 contains a small **OTP (One-Time Programmable)** memory for device-specific configuration and calibration data.

- Some areas are programmed by **Decawave during production test** (calibration, chip ID, wafer data, antenna delay, DGC tuning, PLL codes, temperature/voltage reference values)
- Other areas are available for **customer use** (EUI-64, AES keys, XTAL trim, user data)
- Each OTP location is **32 bits wide**; addresses are word addresses
- Access via: Register file `0x0B` – OTP memory interface

An OTP area is reserved for customers to program the **EUI-64** that is loaded into the `EUI_64` register on power-up.

### 7.3.1 OTP Memory Map (Table 17)

Each entry is 32 bits wide. Size column shows used bytes.

| Address   | Size  | Contents                                                          | Programmed By |
| --------- | ----- | ----------------------------------------------------------------- | ------------- |
| 0x00–0x01 | 4B ea | 64-bit EUID (primary)                                             | Customer      |
| 0x02–0x03 | 4B ea | Alternative 64-bit EUID (selected via reg/SR)                     | Customer      |
| 0x04–0x05 | 4B ea | LDOTUNE_CAL                                                       | Prod Test     |
| 0x06      | 4B    | Chip ID — 5 nibbles (20 bits)                                     | Prod Test     |
| 0x07      | 4B    | LOT ID — 7 nibbles (28 bits)                                      | Prod Test     |
| 0x08      | 4B    | Vbat @ 3.0V [23:16], Vbat @ 3.62V [15:8], Vbat @ 1.62V [7:0]      | Prod Test     |
| 0x09      | 2B    | Temperature @ 22°C ± 2°C [7:0]                                    | Prod Test     |
| 0x0A      | 0     | BIASTUNE_CAL                                                      | Prod Test     |
| 0x0B      | 4B    | Antenna Delay – RFLoop                                            | Prod Test     |
| 0x0C      | 4B    | AoA Isolation: CH9 RF2→RF1, CH9 RF1→RF2, CH5 RF2→RF1, CH5 RF1→RF2 | Prod Test     |
| 0x0D      | 0     | Wafer Sort Lot ID [3:0]                                           | Prod Test     |
| 0x0E      | 0     | Wafer Sort Lot ID [5:4]                                           | Prod Test     |
| 0x0F      | 0     | Wafer Sort: Wafer Number, Y Location, X Location                  | Prod Test     |
| 0x10–0x1D | 4B ea | Customer area (unspecified)                                       | Customer      |
| 0x1E      | 2B    | XTAL_Trim[6:0]                                                    | Customer      |
| 0x1F      | —     | OTP Revision                                                      | Customer      |
| 0x20–0x23 | 4B ea | RX_TUNE_CAL: DGC_CFG0–3                                           | Prod Test     |
| 0x24–0x26 | 4B ea | RX_TUNE_CAL: DGC_CFG4–6                                           | Prod Test     |
| 0x27–0x2D | 4B ea | RX_TUNE_CAL: DGC_LUT_0–6 (CH5)                                    | Prod Test     |
| 0x2E–0x34 | 4B ea | RX_TUNE_CAL: DGC_LUT_0–6 (CH9)                                    | Prod Test     |
| 0x35      | 4B    | PLL_LOCK_CODE                                                     | Prod Test     |
| 0x36–0x5F | —     | UNALLOCATED                                                       | Customer      |
| 0x60      | 1B    | QSR Register (Special function register)                          | Reserved      |
| 0x61      | 4B    | Q_RR Register [7:0]                                               | Reserved      |
| 0x62–0x77 | 4B ea | UNALLOCATED                                                       | Customer      |
| 0x78–0x7B | 4B ea | AES_KEY[127:96], [95:64], [63:32], [31:0] (big endian)            | Customer      |
| 0x7C–0x7F | 4B ea | AES_KEY[255:224], [223:192], [191:160], [159:128] (big endian)    | Customer      |

#### QSR (Special Function Register)

The QSR at address `0x60` is a **32-bit segment readable directly via the register interface** without going through the normal OTP read sequence. To program it: use the normal OTP programming procedure but set the OTP address to `0x60`. The new value takes effect in the QSR register after the **next power-up / boot sequence**.

#### Automatic Loading of Calibration Values

Tune values (LDOTUNE_CAL, BIASTUNE_CAL, DGC configuration, PLL_LOCK_CODE, etc.) can be **automatically loaded from OTP into their respective registers** on boot. The automatic loading ("kicking") process is described in section 8.2.12.3 — Sub-register `0x0B:08` (OTP configuration).

In Decawave evaluation kits, `XTAL_Trim` and `OTP Revision` are programmed into addresses `0x1E` and `0x1F`. During `dwt_initialise()`, the XTAL trim value is automatically copied to the `XTAL_TRIM` register.

### 7.3.2 Programming a Value into OTP Memory

OTP programming requires a specific multi-step sequence. Refer to the DW3000 API functions [3].

### 7.3.3 Reading a Value from OTP Memory

Use the API function:

```c
dwt_otpread(uint16_t address, uint32_t *array, uint8_t length);
```

---

## 7.4 Measuring IC Temperature and Voltage

The DW3000 includes an **8-bit SAR ADC** that can measure:
- Internal IC **temperature** (via on-chip temperature sensor)
- **Battery/supply voltage** (from VDD1 power supply input)

### Operating Modes

| Mode      | Trigger                          | Use case                                                          |
| --------- | -------------------------------- | ----------------------------------------------------------------- |
| Manual    | Host-controlled, any time        | On-demand measurement                                             |
| Automatic | Every time DW3000 enters WAKE_UP | Read temperature before TX heat-up; read unloaded battery voltage |

The automatic mode is particularly useful in battery-powered designs: measurements are taken while the device is in its low-power wake state, before the RF circuitry has heated the die, and before the battery is loaded by TX/RX current.

**Registers:**
- ADC configuration: Register file `0x08` – Transmitter calibration block
- Enable auto-run on wake: `ONW_RUN_SAR` bit in Sub-register `0x0A:00` – AON on wake configuration

---

## 7.5 The Brownout Detector

The DW3000 incorporates a brownout detection circuit that monitors the IC supply voltage and triggers a reset if the voltage drops below approximately **1.5 V**.

### Purpose

During high-current TX or RX operation, the supply voltage can droop — especially in battery-powered designs with:
- Insufficient remaining battery charge
- Insufficient decoupling capacitance on the supply

A brownout means the IC circuits lack sufficient voltage for correct operation, leading to degraded performance or undefined behaviour.

### Brownout Event Elements

| Element         | Register / Location                          | Description                                                           |
| --------------- | -------------------------------------------- | --------------------------------------------------------------------- |
| `VWARN` flag    | `SYS_STATUS`                                 | Set when brownout detected; sticky — stays set until cleared or reset |
| `VWARN_EN` mask | Sub-register `0x00:3C` (System event enable) | Enables `VWARN` to trigger a host interrupt                           |

To clear `VWARN`: write `1` to the `VWARN` bit in `SYS_STATUS`.

> **Important initialisation note:** When the voltage comparator is first enabled, it immediately sets the `VWARN` flag in `SYS_STATUS`. This **initial VWARN must be ignored and cleared**. Only subsequent `VWARN` events indicate a real supply voltage drop.
