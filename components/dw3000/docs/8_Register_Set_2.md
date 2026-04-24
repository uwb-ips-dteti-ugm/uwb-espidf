# Chapter 8 — Register Set (Part 2)

> **Source:** DW3000 Family User Manual, Version 1.1 (© Decawave Ltd 2019, revised 28 May 2021)
> **Pages:** 122-165

---

### 8.2.3 Register File: 0x02 — STS Configuration and Status

| ID   | Length (octets) | Type | Mnemonic   | Description                                                     |
| ---- | --------------- | ---- | ---------- | --------------------------------------------------------------- |
| 0x02 | 55              | —    | STS_CONFIG | Scrambled Timestamp Sequence configuration and status registers |

Register file 0x02 is concerned with the DW3000 STS block.

**Table 23 — Register file: 0x02 overview**

| OFFSET | Mnemonic | Description       |
| ------ | -------- | ----------------- |
| 0x00   | STS_CFG  | STS configuration |
| 0x04   | STS_CTRL | STS control       |
| 0x08   | STS_STS  | STS status        |
| 0x0C   | STS_KEY  | STS 128-bit KEY   |
| 0x1C   | STS_IV   | STS 128-bit IV    |

---

#### 8.2.3.1 STS_CFG — STS Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x02:00` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field   | Description                                                                                                                                |
| ------ | ------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| [7:0]  | CPS_LEN | STS length in blocks of 8. Default `7` = ~64 μs (64 × 512 chips). Min value `3` = 32 chips. Max value `255` = ~2048 μs (2048 × 512 chips). |
| [15:8] | —       | Reserved. **Any change to these bits may cause the DW3000 to malfunction.**                                                                |

---

#### 8.2.3.2 STS_CTRL — STS Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x02:04` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits  | Field    | Description                                                                                                                                                   |
| ----- | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [0]   | LOAD_IV  | Load STS_IV into the AES-128 block for STS generation. Writing `1` loads the IV. `CP_SPC` configuration must be set before asserting this bit. Self-clearing. |
| [1]   | RST_LAST | Start STS generation from the last count used by the AES-128 block of the previous STS. Self-clearing.                                                        |
| [7:2] | —        | Reserved.                                                                                                                                                     |

---

#### 8.2.3.3 STS_STS — STS Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x02:08` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                                                                                                                                                                                            |
| ------- | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [11:0]  | ACC_QUAL | 12-bit STS accumulation quality measure. Assess this value before accepting a receive timestamp. A low value indicates the STS_TOA timestamp may be corrupted. See §6 — Secure ranging / timestamping. |
| [15:12] | —        | Reserved.                                                                                                                                                                                              |

---

#### 8.2.3.4 STS_KEY — STS 128-bit Key

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x02:0C` |
| Size      | 16 bytes  |
| Access    | RW        |

128-bit key (four 32-bit words) used by the AES-128 block for Scrambled Timestamp Sequence generation. Lower order octets are at lower offset addresses.

**Default:** `0x738123B B5E5A4ED 8DF43A20 C9A375FA`

---

#### 8.2.3.5 STS_IV — STS 128-bit IV

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x02:1C` |
| Size      | 16 bytes  |
| Access    | RW        |

128-bit initialisation vector (four 32-bit words) used by the AES-128 block for STS generation. Lower order octets are at lower offset addresses.

**Default:** `0x1`

> **Note:** Sub-register `0x0F:48` — Counter debug contains the current value of the low 32-bits of the STS_IV.

---

### 8.2.4 Register File: 0x03 — Receiver Tuning Parameters

| ID   | Length (octets) | Type | Mnemonic | Description                |
| ---- | --------------- | ---- | -------- | -------------------------- |
| 0x03 | —               | RW   | RX_TUNE  | Receiver tuning parameters |

Register file 0x03 controls and configures the DW3000 receiver. The tuning parameters below must be configured based on the channel used to optimise receiver performance when 64 MHz PRF is configured. Other sub-registers in this file are reserved and must not be modified.

**Table 24 — Receiver tuning parameters**

| OFFSET | Mnemonic  | Channel 5 value | Channel 9 value |
| ------ | --------- | --------------- | --------------- |
| 0x1C   | DGC_CFG0  | 0x10000240      | 0x10000240      |
| 0x20   | DGC_CFG1  | 0x1b6da489      | 0x1b6da489      |
| 0x38   | DGC_LUT_0 | 0x0001C0FD      | 0x0002A8FE      |
| 0x3C   | DGC_LUT_1 | 0x0001C43E      | 0x0002AC36      |
| 0x40   | DGC_LUT_2 | 0x0001C6BE      | 0x0002A5FE      |
| 0x44   | DGC_LUT_3 | 0x0001C77E      | 0x0002AF3E      |
| 0x48   | DGC_LUT_4 | 0x0001CF36      | 0x0002AF7d      |
| 0x4C   | DGC_LUT_5 | 0x0001CFB5      | 0x0002AFB5      |
| 0x50   | DGC_LUT_6 | 0x0001CFF5      | 0x0002AFB5      |

> **Note:** When 16 MHz PRF is used, the `RX_TUNE_EN` bit (bit 0 in `DGC_CFG`) must be cleared.

---

#### 8.2.4.1 DGC_CFG — RX Tune Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x03:18` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field      | Description                                                                                                  |
| ------ | ---------- | ------------------------------------------------------------------------------------------------------------ |
| [0]    | RX_TUNE_EN | RX tuning enable. Set to `1` when 64 MHz PRF is used; set to `0` otherwise.                                  |
| [14:9] | THR_64     | RX tuning threshold for 64 MHz PRF. Change from default `0x38` to `0x32` for optimised receiver performance. |
| —      | —          | Other bits are reserved.                                                                                     |

---

#### 8.2.4.2 DGC_DBG — DGC Report

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x03:60` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits    | Field        | Description              |
| ------- | ------------ | ------------------------ |
| [30:28] | DGC_DECISION | DGC decision index.      |
| —       | —            | Other bits are reserved. |

---

### 8.2.5 Register File: 0x04 — External Sync Control and RX Calibration

| ID   | Length (octets) | Type | Mnemonic | Description                                         |
| ---- | --------------- | ---- | -------- | --------------------------------------------------- |
| 0x04 | 12              | RW   | EXT_SYNC | External synchronisation control and RX calibration |

Register file 0x04 controls the DW3000 synchronisation hardware and RX calibration function.

**Table overview — Register file: 0x04**

| OFFSET | Mnemonic    | Description                                          |
| ------ | ----------- | ---------------------------------------------------- |
| 0x00   | EC_CTRL     | External clock synchronisation counter configuration |
| 0x0C   | RX_CAL      | RX calibration block configuration                   |
| 0x14   | RX_CAL_RESI | RX calibration block result I                        |
| 0x1C   | RX_CAL_RESQ | RX calibration block result Q                        |
| 0x20   | RX_CAL_STS  | RX calibration block status                          |

---

#### 8.2.5.1 EC_CTRL — External Clock Sync Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x04:00` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits   | Field     | Description                                                                                 |
| ------ | --------- | ------------------------------------------------------------------------------------------- |
| [10:3] | OSTS_WAIT | Wait counter for external timebase reset. See §7.1.1 — One Shot Timebase Reset (OSTR) Mode. |
| [11]   | OSTR_MODE | External timebase reset mode enable. See §7.1.1.                                            |
| —      | —         | Other bits are reserved.                                                                    |

---

#### 8.2.5.2 RX_CAL — RX Calibration Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x04:0C` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                                                                                                                            |
| ------- | -------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| [1:0]   | CAL_MODE | RX calibration mode: `0`=Normal (default), `1`=Calibration mode, `2`/`3`=Reserved.                                                     |
| [4]     | CAL_EN   | RX calibration enable. Write `1` to start calibration. Self-clears when calibration is complete. RX_CAL_STS will be set on completion. |
| [19:16] | COMP_DLY | RX calibration tuning value. Set to `0x2` for optimal performance. Other values must not be used.                                      |
| —       | —        | Other bits are reserved.                                                                                                               |

---

#### 8.2.5.3 RX_CAL_RESI — RX Calibration Result I

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x04:14` |
| Size      | 4 bytes   |
| Access    | RW        |

29-bit register. Reports the I-channel calibration result once calibration is complete. If the value reads back as `0x1FFFFFFF`, calibration has failed and the receiver must not be used. Repeat the RX calibration procedure.

---

#### 8.2.5.4 RX_CAL_RESQ — RX Calibration Result Q

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x04:1C` |
| Size      | 4 bytes   |
| Access    | RW        |

29-bit register. Reports the Q-channel calibration result once calibration is complete. If the value reads back as `0x1FFFFFFF`, calibration has failed. If either `RX_CAL_RESI` or `RX_CAL_RESQ` reports failure, the RX calibration must be repeated.

---

#### 8.2.5.5 RX_CAL_STS — RX Calibration Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x04:20` |
| Size      | 1 byte    |
| Access    | RW        |

Single-bit register. Bit `0` is set when RX calibration is complete. Write `1` to clear.

---

### 8.2.6 Register File: 0x05 — GPIO Control and Status

| ID   | Length (octets) | Type | Mnemonic  | Description                                    |
| ---- | --------------- | ---- | --------- | ---------------------------------------------- |
| 0x05 | 47              | —    | GPIO_CTRL | General Purpose Input-Output control registers |

> **Note:** GPIO clocks must be enabled before enabling or disabling GPIO mode or value. Enable them by setting `GPIO_CLK_EN`, `GPIO_DCLK_EN`, and `GPIO_DRST_N` in Sub-register `0x11:04` — Clock control.

**Table 25 — Register file: 0x05 overview**

| OFFSET | Mnemonic     | Description                          |
| ------ | ------------ | ------------------------------------ |
| 0x00   | GPIO_MODE    | GPIO Mode Control Register           |
| 0x04   | GPIO_PULL_EN | GPIO Drive Strength and Pull Control |
| 0x08   | GPIO_DIR     | GPIO Direction Control Register      |
| 0x0C   | GPIO_OUT     | GPIO Data Output Register            |
| 0x10   | GPIO_IRQE    | GPIO Interrupt Enable                |
| 0x14   | GPIO_ISTS    | GPIO Interrupt Status                |
| 0x18   | GPIO_ISEN    | GPIO Interrupt Sense Selection       |
| 0x1C   | GPIO_IMODE   | GPIO Interrupt Mode (Level / Edge)   |
| 0x20   | GPIO_IBES    | GPIO Interrupt "Both Edge" Select    |
| 0x24   | GPIO_ICLR    | GPIO Interrupt Latch Clear           |
| 0x28   | GPIO_IDBE    | GPIO Interrupt De-bounce Enable      |
| 0x2C   | GPIO_RAW     | GPIO Raw State                       |

---

#### 8.2.6.1 GPIO_MODE — GPIO Mode Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:00` |
| Size      | 4 bytes   |
| Access    | RW        |

Selects whether each GPIO pin operates as a standard GPIO or as a special-function pin.

| Bits    | Field | GPIO pin  | Description                                                              |
| ------- | ----- | --------- | ------------------------------------------------------------------------ |
| [2:0]   | MSGP0 | GPIO0     | `000`=GPIO0 (default); `001`=RXOKLED; `010`=PDOA_SW_0                    |
| [5:3]   | MSGP1 | GPIO1     | `000`=GPIO1 (default); `001`=SFDLED; `010`=PDOA_SW_1                     |
| [8:6]   | MSGP2 | GPIO2     | `000`=GPIO2 (default); `001`=RXLED; `010`=PDOA_SW_2                      |
| [11:9]  | MSGP3 | GPIO3     | `000`=GPIO3 (default); `001`=TXLED; `010`=PDOA_SW_3                      |
| [14:12] | MSGP4 | GPIO4     | `000`=GPIO4 (default); `001`=EXTPA (External Power Amplifier); `010`=IRQ |
| [17:15] | MSGP5 | GPIO5     | `000`=GPIO5 (default); `001`=EXTTXE (External Transmit Enable)           |
| [20:18] | MSGP6 | GPIO6     | `000`=GPIO6 (default); `001`=EXTRXE (External Receiver Enable)           |
| [23:21] | MSGP7 | GPIO7     | `000`=SYNC input (reserved test function); `001`=GPIO7                   |
| [26:24] | MSGP8 | IRQ/GPIO8 | `000`=IRQ output (default); `001`=GPIO8                                  |
| [31:27] | —     | —         | Reserved.                                                                |

> **Note:** LED outputs (RXOKLED, SFDLED, RXLED, TXLED) drain power in battery-powered applications. Blink time is configured in Sub-register `0x11:16` — LED control.

---

#### 8.2.6.2 GPIO_PULL_EN — GPIO Drive Strength and Pull Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:04` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field       | Description                                                                                                      |
| ------ | ----------- | ---------------------------------------------------------------------------------------------------------------- |
| [8:0]  | GPEN0–GPEN8 | Pull enable for GPIO0–GPIO8. Setting a bit to `0` lowers the drive strength for that GPIO. Default: all enabled. |
| [15:9] | —           | Reserved.                                                                                                        |

---

#### 8.2.6.3 GPIO_DIR — GPIO Direction Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:08` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field     | Description                                                                      |
| ------ | --------- | -------------------------------------------------------------------------------- |
| [8:0]  | GDP0–GDP8 | Direction for GPIO0–GPIO8. `1`=input, `0`=output. Default: all inputs (`0x1FF`). |
| [15:9] | —         | Reserved.                                                                        |

Applies only when the GPIO pin is selected to operate as a GPIO via `GPIO_MODE`.

---

#### 8.2.6.4 GPIO_OUT — GPIO Data Output

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:0C` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field     | Description                                                  |
| ------ | --------- | ------------------------------------------------------------ |
| [8:0]  | GOP0–GOP8 | Output state for GPIO0–GPIO8. `1`=logic high, `0`=logic low. |
| [15:9] | —         | Reserved.                                                    |

Reading from this register returns the current output setting, not the actual pin state (which depends on `GPIO_MODE` and `GPIO_DIR`).

---

#### 8.2.6.5 GPIO_IRQE — GPIO Interrupt Enable

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:10` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                  |
| ------ | ------------- | ------------------------------------------------------------ |
| [8:0]  | GIRQE0–GIRQE8 | Interrupt enable for GPIO0–GPIO8. `1`=enabled, `0`=disabled. |
| [15:9] | —             | Reserved. Write `0`.                                         |

When a GPIO interrupt is triggered it is signalled via the `GPIOIRQ` event status bit in Sub-register `0x00:44` — System event status.

---

#### 8.2.6.6 GPIO_ISTS — GPIO Interrupt Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:14` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                                                  |
| ------ | ------------- | -------------------------------------------------------------------------------------------- |
| [8:0]  | GISTS0–GISTS8 | Interrupt status for GPIO0–GPIO8. `1`=this GPIO gave rise to the `GPIOIRQ` SYS_STATUS event. |
| [15:9] | —             | Reserved. Write `0`.                                                                         |

---

#### 8.2.6.7 GPIO_ISEN — GPIO Interrupt Sense Selection

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:18` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                                                                        |
| ------ | ------------- | ------------------------------------------------------------------------------------------------------------------ |
| [8:0]  | GISEN0–GISEN8 | Sense selection for GPIO0–GPIO8. `0`=Active high / rising-edge triggered. `1`=Active low / falling-edge triggered. |
| [31:9] | —             | Reserved. Write `0`.                                                                                               |

Used together with `GPIO_IMODE` to fully configure interrupt sensitivity.

---

#### 8.2.6.8 GPIO_IMODE — GPIO Interrupt Mode Selection

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:1C` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                              |
| ------ | ------------- | ------------------------------------------------------------------------ |
| [8:0]  | GIMOD0–GIMOD8 | Interrupt mode for GPIO0–GPIO8. `0`=Level sensitive, `1`=Edge triggered. |
| [15:9] | —             | Reserved. Write `0`.                                                     |

---

#### 8.2.6.9 GPIO_IBES — GPIO Interrupt Both-Edge Selection

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:20` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                                                                                 |
| ------ | ------------- | --------------------------------------------------------------------------------------------------------------------------- |
| [8:0]  | GIBES0–GIBES8 | Both-edge selection for GPIO0–GPIO8. `0`=edge determined by `GPIO_IMODE`/`GPIO_ISEN`. `1`=both edges trigger the interrupt. |
| [15:9] | —             | Reserved. Write `0`.                                                                                                        |

Only applies when edge-sensitive interrupts are enabled in `GPIO_IMODE`.

---

#### 8.2.6.10 GPIO_ICLR — GPIO Interrupt Latch Clear

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:24` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                                                                        |
| ------ | ------------- | ------------------------------------------------------------------------------------------------------------------ |
| [8:0]  | GICLR0–GICLR8 | Write `1` to clear the interrupt latch for the corresponding GPIO. Writing `0` has no effect. Reading returns `0`. |
| [15:9] | —             | Reserved. Write `0`.                                                                                               |

> **Note:** There is no way to read the interrupt latch — only one GPIO interrupt can be enabled at a time unless the host has an external way to distinguish events. For level-sensitive interrupts, if the active level persists after the latch is cleared, the interrupt will immediately re-trigger.

---

#### 8.2.6.11 GPIO_IDBE — GPIO Interrupt De-bounce Enable

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:28` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits   | Field         | Description                                                            |
| ------ | ------------- | ---------------------------------------------------------------------- |
| [8:0]  | GIDBE0–GIDBE8 | De-bounce enable for GPIO0–GPIO8. `1`=de-bounce enabled, `0`=disabled. |
| [15:9] | —             | Reserved. Write `0`.                                                   |

The de-bounce filter removes short transients using the kilohertz clock (enabled via `LP_CLK_EN` in Sub-register `0x11:04`; clock rate set via `LP_CLK_DIV` in `0x11:08`). A state change must persist for two kilohertz clock cycles before it is presented to the interrupt logic.

---

#### 8.2.6.12 GPIO_RAW — GPIO Raw State

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x05:2C` |
| Size      | 2 bytes   |
| Access    | RO        |

| Bits   | Field         | Description                                     |
| ------ | ------------- | ----------------------------------------------- |
| [8:0]  | GRAWP0–GRAWP8 | Raw sampled state of GPIO0–GPIO8 pins (0 or 1). |
| [15:9] | —             | Reserved. Write `0`.                            |

---

### 8.2.7 Register File: 0x06 — Digital Receiver Configuration

| ID   | Length (octets) | Type | Mnemonic | Description                    |
| ---- | --------------- | ---- | -------- | ------------------------------ |
| 0x06 | —               | —    | DRX_CONF | Digital receiver configuration |

Register file 0x06 is concerned with the low-level digital receiver configuration.

**Table 26 — Register file: 0x06 overview**

| OFFSET | Mnemonic    | Description                          |
| ------ | ----------- | ------------------------------------ |
| 0x00   | DTUNE0      | PAC configuration                    |
| 0x02   | RX_SFD_TOC  | SFD timeout                          |
| 0x04   | PRE_TOC     | Preamble detection timeout           |
| 0x0C   | DTUNE3      | Receiver tuning register             |
| 0x14   | DTUNE_5     | Digital Tuning Reserved register     |
| 0x29   | DRX_CAR_INT | Carrier recovery integrator register |

---

#### 8.2.7.1 DTUNE0 — Digital RX Tuning Register 0

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x06:00` |
| Size      | 2 bytes   |
| Access    | RW        |

> **Caution:** Take care not to write other values to this register as doing so may cause the DW3000 to malfunction.

| Bits   | Field | Description                                                                |
| ------ | ----- | -------------------------------------------------------------------------- |
| [1:0]  | PAC   | Preamble Acquisition Chunk size. Select based on expected preamble length. |
| [4]    | DT0B4 | Tuning bit 4. Must be cleared to `0` for best performance.                 |
| [15:5] | —     | Reserved. Do not modify.                                                   |

**PAC values (see Table 12 for recommended PAC per preamble length):**

| PAC value | PAC size |
| --------- | -------- |
| `0x00`    | 8        |
| `0x01`    | 16       |
| `0x02`    | 32       |
| `0x03`    | 4        |

---

#### 8.2.7.2 RX_SFD_TOC — SFD Detection Timeout Count

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x06:02` |
| Size      | 2 bytes   |
| Access    | RW        |

16-bit SFD detection timeout counter period, in units of preamble symbols. The SFD detection timeout starts counting as soon as preamble is detected. If the SFD is not detected before the timeout, the current reception is aborted and the `RXSTO` event status bit is set.

**Default:** `65` (= 64 + 8 − 8 + 1, matching default preamble, SFD, and PAC).

**Recommended formula:** `RX_SFD_TOC = preamble_length + 1 − PAC_size + SFD_length`

> **WARNING:** Do **NOT** set `RX_SFD_TOC` to zero. Disabling the SFD timeout means a false preamble detection will leave the IC in receive mode indefinitely, significantly draining battery life.

---

#### 8.2.7.3 PRE_TOC — Preamble Detection Timeout Count

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x06:04` |
| Size      | 2 bytes   |
| Access    | RW        |

16-bit preamble detection timeout period, in units of PAC size symbols. A value of `0` disables the preamble detection timeout (default). If the timeout expires before preamble is detected, the `RXPTO` event bit is set.

Maximum timeout = 65535 × PAC size symbols (> 250 ms for the smallest PAC size).

**Example:** For a 1024-symbol preamble with PAC size = 32, set `PRE_TOC = 1024 / 32 = 32`.

---

#### 8.2.7.4 DTUNE3 — Digital RX Tuning Register 3

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x06:0C` |
| Size      | 4 bytes   |
| Access    | RW        |

32-bit tuning register. Change from the default value `0xAF5F584C` to `0xAF5F35CC` for optimal receiver performance.

---

#### 8.2.7.5 DTUNE_5 — Digital Tuning Reserved Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x06:14` |
| Size      | 4 bytes   |
| Access    | RO        |

Reserved register. Do not modify.

---

#### 8.2.7.6 DRX_CAR_INT — Carrier Recovery Integrator Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x06:29` |
| Size      | 3 bytes   |
| Access    | RO        |

21-bit signed read-only register. Provides an estimate of the remote transmitter's frequency offset, generated during reception as the receiver locks on and compensates for the frequency offset. Lower 17 bits are the fractional part; upper 4 bits are the integer portion.

**Frequency offset formula:**

```
F_offset = (C_int × 2^-17) / (2 × (N_samples / Fs))

where:
  C_int     = carrier integrator value (this register)
  N_samples = 8192 for 110 kb/s, 1024 otherwise
  Fs        = 998.4 × 10^6
```

**Convert to ppm clock offset:**

```
Offset_ppm = -10^6 × (F_offset / Fc)

where Fc = 6489.6 MHz for channel 5
```

**Table 27 — Constants for frequency offset calculation (simplified)**

| Data Rate          | Channel 5  | Channel 9  |
| ------------------ | ---------- | ---------- |
| 850 kb/s, 6.8 Mb/s | −0.5731e−3 | −0.1252e−3 |

Multiply the raw register value by the constant above to get F_offset in Hz directly.

> **Note:** The carrier recovery algorithm runs during the entire packet reception. The register reflects the value at the end of reception. An alternative value is available in the `CIA_DIAG_0` register.

---

### 8.2.8 Register File: 0x07 — Analog RF Configuration Block

| ID   | Length (octets) | Type | Mnemonic | Description             |
| ---- | --------------- | ---- | -------- | ----------------------- |
| 0x07 | —               | —    | RF_CONF  | Analog RF configuration |

Register file 0x07 is concerned with the low-level configuration of the IC analog blocks.

**Table 28 — Register file: 0x07 overview**

| OFFSET | Mnemonic     | Description                           |
| ------ | ------------ | ------------------------------------- |
| 0x00   | RF_ENABLE    | RF enable                             |
| 0x04   | RF_CTRL_MASK | RF enable mask                        |
| 0x14   | RF_SWITCH    | RF switch configuration               |
| 0x1A   | RF_TX_CTRL_1 | RF transmitter configuration          |
| 0x1C   | RF_TX_CTRL_2 | RF transmitter configuration          |
| 0x28   | TX_TEST      | Transmitter test configuration        |
| 0x34   | SAR_TEST     | SAR temperature sensor read enable    |
| 0x40   | LDO_TUNE     | Internal LDO voltage tuning parameter |
| 0x48   | LDO_CTRL     | LDO control                           |
| 0x51   | LDO_RLOAD    | LDO tuning register                   |

---

#### 8.2.8.1 RF_ENABLE — RF Control Enable

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:00` |
| Size      | 4 bytes   |
| Access    | RW        |

32-bit configuration register for the transceiver. Used to force TX RF blocks on in test modes (e.g. Continuous Wave). The same value must also be written to `RF_CTRL_MASK`.

> **Caution:** Do not write other values to the reserved area of this register.

**Table 29 — RF_ENABLE and RF_CTRL_MASK values**

| TX Channel | 32-bit value |
| ---------- | ------------ |
| 5          | `0x02003C00` |
| 9          | `0x02001C00` |

---

#### 8.2.8.2 RF_CTRL_MASK — RF Control Mask

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:04` |
| Size      | 4 bytes   |
| Access    | RW        |

32-bit configuration register. Must match the value written to `RF_ENABLE`. See Table 29 above.

---

#### 8.2.8.3 RF_SWITCH — RF Switch Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:14` |
| Size      | 4 bytes   |
| Access    | RW        |

32-bit control register for the TX/RX antenna switch.

| Bits    | Field         | Description                                                                                         |
| ------- | ------------- | --------------------------------------------------------------------------------------------------- |
| [0]     | ANTSWNOTOGGLE | `1`=disable automatic antenna switch toggling in PDoA modes. `0`=auto-toggle (default).             |
| [1]     | ANTSWPDOAPORT | Starting port for PDoA: `0`=RF port 1 (default), `1`=RF port 2.                                     |
| [8]     | ANTSWEN       | `1`=manual control of antenna switch via ANTSWCTRL. `0`=automatic (default).                        |
| [14:12] | ANTSWCTRL     | Manual antenna switch control (when ANTSWEN=1): `0x0`=disconnect, `0x1`=RF port 1, `0x2`=RF port 2. |
| [16]    | TRXSWEN       | `1`=manual control of TX/RX switch via TRXSWCTRL. `0`=automatic (default).                          |
| [29:24] | TRXSWCTRL     | TX/RX switch control (when TRXSWEN=1): `0x01`=TX, `0x2a`=RX CH9, `0x1c`=RX CH5.                     |
| —       | —             | Other bits are reserved.                                                                            |

---

#### 8.2.8.4 RF_TX_CTRL_1 — RF TX Control Register 1

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:1A` |
| Size      | 1 byte    |
| Access    | RW        |

8-bit control register for the transmitter. Set to `0x0E` for optimal performance.

---

#### 8.2.8.5 RF_TX_CTRL_2 — RF TX Control Register 2

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:1C` |
| Size      | 4 bytes   |
| Access    | RW        |

32-bit control register for the transmitter. The value depends on the TX channel selected in `CHAN_CTRL`.

> **Caution:** Do not write other values to the reserved area; doing so may cause the DW3000 to malfunction.

| Bits  | Field    | Description                                                                                   |
| ----- | -------- | --------------------------------------------------------------------------------------------- |
| [5:0] | PG_DELAY | Pulse Generator delay. Sets transmitted pulse width / bandwidth. Value depends on TX channel. |
| —     | —        | Other bits reserved. Program only as directed in Table 30.                                    |

**Table 30 — RF_TX_CTRL_2 values**

| TX Channel | 32-bit value |
| ---------- | ------------ |
| 5          | `0x1C071134` |
| 9          | `0x1C010034` |

**Table 31 — PG_DELAY recommended values**

| TX Channel | PG_DELAY (6-bit) |
| ---------- | ---------------- |
| 5          | `0x34`           |
| 9          | `0x34`           |

---

#### 8.2.8.6 TX_TEST — Transmitter Test Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:28` |
| Size      | 1 byte    |
| Access    | RW        |

Configures the transmitter test modes (Continuous Wave).

| Bits  | Field     | Description                                                                                              |
| ----- | --------- | -------------------------------------------------------------------------------------------------------- |
| [3:0] | TX_ENTEST | Transmitter test enable bitmask: `0x01`=CW mode 4, `0x02`=CW mode 3, `0x04`=CW mode 2, `0x08`=CW mode 1. |
| [7:4] | —         | Reserved. Always write `0`.                                                                              |

---

#### 8.2.8.7 SAR_TEST — SAR Temperature Sensor Read Enable

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:34` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits | Field    | Description                                         |
| ---- | -------- | --------------------------------------------------- |
| [2]  | SAR_RDEN | Write `1` to enable SAR temperature sensor reading. |
| —    | —        | Other bits reserved. Always write `0`.              |

---

#### 8.2.8.8 LDO_TUNE — Internal LDO Voltage Tuning

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:40` |
| Size      | 8 bytes   |
| Access    | RW        |

| Bits   | Field    | Description                                                                                                                      |
| ------ | -------- | -------------------------------------------------------------------------------------------------------------------------------- |
| [60:0] | LDO_TUNE | Controls output voltage levels of the on-chip LDOs. On power-up, loaded from OTP (LDOTUNE_CAL, Table 17). Copy using `LDO_KICK`. |

> **Note:** When programming OTP memory, `LDO_TUNE` bits [15:12] must be set to `0xF`. If OTP reads `0x0`, use the default value.

---

#### 8.2.8.9 LDO_CTRL — LDO Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:48` |
| Size      | 4 bytes   |
| Access    | RW        |

32-bit configuration register. Used to enable LDO blocks when exercising certain test modes (e.g. Continuous Wave). See DW3000 APIs for details.

---

#### 8.2.8.10 LDO_RLOAD — LDO Tuning Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x07:51` |
| Size      | 1 byte    |
| Access    | RW        |

8-bit tuning register for the transceiver. Set to `0x14` for optimal operation.

---

### 8.2.9 Register File: 0x08 — Transmitter Calibration Block

| ID   | Length (octets) | Type | Mnemonic | Description                   |
| ---- | --------------- | ---- | -------- | ----------------------------- |
| 0x08 | —               | —    | TX_CAL   | Transmitter calibration block |

Register file 0x08 is the transmit calibration block, ensuring the optimum configuration of the transmit signal.

**Table 32 — Register file: 0x08 overview**

| OFFSET | Mnemonic      | Description                        |
| ------ | ------------- | ---------------------------------- |
| 0x00   | SAR_CTRL      | SAR control                        |
| 0x04   | SAR_STATUS    | SAR status                         |
| 0x08   | SAR_READING   | Latest SAR readings                |
| 0x0C   | SAR_WAKE_RD   | SAR readings at last wake-up       |
| 0x10   | PGC_CTRL      | Pulse Generator control            |
| 0x14   | PGC_STATUS    | Pulse Generator status             |
| 0x18   | PG_TEST       | Pulse Generator test               |
| 0x1C   | PG_CAL_TARGET | Pulse Generator count target value |

---

#### 8.2.9.1 SAR_CTRL — SAR Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:00` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits   | Field     | Description                                                                                                       |
| ------ | --------- | ----------------------------------------------------------------------------------------------------------------- |
| [0]    | SAR_START | Write `1` to start SAR sampling; write `0` to clear enable. `SAR_STATUS` is set to `1` once sampling is complete. |
| [15:1] | —         | Reserved. Always write `0`.                                                                                       |

---

#### 8.2.9.2 SAR_STATUS — SAR Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:04` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits   | Field    | Description                                     |
| ------ | -------- | ----------------------------------------------- |
| [0]    | SAR_DONE | `0`=SAR gathering data, `1`=data ready to read. |
| [15:1] | —        | Reserved. Always write `0`.                     |

---

#### 8.2.9.3 SAR_READING — Latest SAR Readings

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:08` |
| Size      | 3 bytes   |
| Access    | RO        |

| Bits    | Field     | Description                                             |
| ------- | --------- | ------------------------------------------------------- |
| [7:0]   | SAR_LVBAT | Latest voltage reading. LSB ≈ 6 mV. Range: 2.25–3.76 V. |
| [15:8]  | SAR_LTEMP | Latest temperature reading. LSB ≈ 0.8 °C.               |
| [23:16] | —         | Reserved.                                               |

**Conversion formulas:**

```
Voltage (V)  = (SAR_LVBAT − OTP_READ(Vmeas @ 3.0 V)) × 0.0251 + 3.0
Temperature (°C) = (SAR_LTEMP − OTP_READ(Vtemp @ 22°C)) × 1.05 + 22
```

> **Note:** To read temperature, `SAR_RDEN` bit in `SAR_TEST` (`0x07:34`) must be set first.

---

#### 8.2.9.4 SAR_WAKE_RD — Wake-up SAR Readings

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:0C` |
| Size      | 2 bytes   |
| Access    | RO        |

| Bits   | Field     | Description                                                                                               |
| ------ | --------- | --------------------------------------------------------------------------------------------------------- |
| [7:0]  | SAR_WBAT  | Voltage reading at last wake-up event. Valid only if `ONW_RUN_SAR` bit was set in AON wake configuration. |
| [15:8] | SAR_WTEMP | RFU. Use `SAR_READING` register for temperature.                                                          |

---

#### 8.2.9.5 PGC_CTRL — PG Calibration Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:10` |
| Size      | 2 bytes   |
| Access    | RW        |

Two calibration modes:
1. **PGC_CNT:** Calculates `PG_COUNT` from current `PG_DELAY` value.
2. **PGC_DLY:** Auto-calibration — updates `PG_DELAY` based on a reference `PG_COUNT` (PG_TARGET) value. The new `PG_DELAY` is written to `TX_CTRL`.

| Bits  | Field        | Description                                                                                                                                 |
| ----- | ------------ | ------------------------------------------------------------------------------------------------------------------------------------------- |
| [0]   | PGC_START    | Write `1` to start pulse generator calibration (PGC_CNT mode). Self-clearing when complete.                                                 |
| [1]   | PGC_AUTO_CAL | Write `1` to start pulse generator auto-calibration (PGC_DLY mode). Self-clearing; new `PG_DELAY` written to `TX_CTRL`.                     |
| [5:2] | PGC_TMEAS    | 4 MSBs of the 10-bit calibration counter (clocked by system clock). Defines the number of cycles over which to run the calibration counter. |
| —     | —            | Other bits reserved.                                                                                                                        |

---

#### 8.2.9.6 PGC_STATUS — PG Calibration Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:14` |
| Size      | 2 bytes   |
| Access    | RO        |

| Bits   | Field        | Description                                                                                                                                           |
| ------ | ------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| [11:0] | PG_DELAY_CNT | Pulse generator count value (`PG_COUNT`) calculated by the calibration routine. Gives a consistent reflection of bandwidth regardless of temperature. |
| [12]   | AUTOCAL_DONE | `1` when auto-calibration is complete and new `PG_DELAY` has been written to `TX_CTRL`.                                                               |
| —      | —            | Other bits reserved.                                                                                                                                  |

---

#### 8.2.9.7 PG_TEST — PG Test Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:18` |
| Size      | 2 bytes   |
| Access    | RW        |

Used for crystal trimming (Continuous Wave mode). Leave at default for normal operation.

**Table 33 — PG_TEST values**

| Mode                 | 16-bit value |
| -------------------- | ------------ |
| Normal operation     | `0x0000`     |
| Continuous Wave (CW) | `0x000F`     |

---

#### 8.2.9.8 PG_CAL_TARGET — PG Count Target Value

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x08:1C` |
| Size      | 2 bytes   |
| Access    | RO        |

| Bits   | Field     | Description                                                                                                                        |
| ------ | --------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| [11:0] | PG_TARGET | Target value of `PG_COUNT` at which PG auto-calibration completes. The device returns a count as close as possible to this target. |
| —      | —         | Other bits reserved.                                                                                                               |

---

### 8.2.10 Register File: 0x09 — Frequency Synthesiser Control Block

| ID   | Length (octets) | Type | Mnemonic | Description                         |
| ---- | --------------- | ---- | -------- | ----------------------------------- |
| 0x09 | —               | —    | FS_CTRL  | Frequency synthesiser control block |

Register file 0x09 is the frequency synthesiser control block. Its main function is the generation of the carrier frequency for the operating channel.

**Table 34 — Register file: 0x09 overview**

| OFFSET | Mnemonic | Description                   |
| ------ | -------- | ----------------------------- |
| 0x00   | PLL_CFG  | PLL configuration             |
| 0x04   | PLL_CC   | PLL coarse code               |
| 0x08   | PLL_CAL  | PLL calibration configuration |
| 0x14   | XTAL     | Crystal trim                  |

---

#### 8.2.10.1 PLL_CFG — PLL Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x09:00` |
| Size      | 2 bytes   |
| Access    | RW        |

Per-channel PLL configuration register.

**Table 35 — PLL_CFG reference values**

| TX Channel | Value    |
| ---------- | -------- |
| 5          | `0x1F3C` |
| 9          | `0x0F3C` |

---

#### 8.2.10.2 PLL_CC — PLL Coarse Code

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x09:04` |
| Size      | 1 byte    |
| Access    | RW        |

Sets the starting code for PLL calibration.

| Bits    | Field    | Description                                                                      |
| ------- | -------- | -------------------------------------------------------------------------------- |
| [6:0]   | CH9_CODE | PLL coarse code for channel 9. Default `0x0B`. May be updated after calibration. |
| [13:8]  | CH5_CODE | PLL coarse code for channel 5. Default `0x0F`. May be updated after calibration. |
| [31:22] | —        | Reserved.                                                                        |

> **Note:** If OTP address `0x35` (`PLL_LOCK_CODE`) has a non-zero value, copy it to this register prior to PLL calibration and also set the `USE_OLD` bit in `PLL_CAL`.

---

#### 8.2.10.3 PLL_CAL — PLL Calibration Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x09:08` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field      | Description                                                                                                                                               |
| ------ | ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [0]    | USE_OLD    | When set to `1`, use the coarse code value from `PLL_CC` as the starting point for PLL calibration.                                                       |
| [7:2]  | PLL_CFG_LD | PLL calibration configuration value. Default `0x31`; use `0x81` for more optimal performance.                                                             |
| [8]    | CAL_EN     | PLL calibration enable. Set to `1` to recalibrate PLL (e.g. on channel 9 after a temperature change of 20 °C, or when switching channels). Self-clearing. |
| [15:9] | —          | Reserved.                                                                                                                                                 |

---

#### 8.2.10.4 XTAL — Crystal Trim Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x09:14` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits  | Field     | Description                                                                                                                   |
| ----- | --------- | ----------------------------------------------------------------------------------------------------------------------------- |
| [5:0] | XTAL_TRIM | Crystal trim value. Used to fine-tune the crystal oscillator frequency. See §10.1 — IC calibration — crystal oscillator trim. |
| [7:6] | —         | Reserved.                                                                                                                     |
