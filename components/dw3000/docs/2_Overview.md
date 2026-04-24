# Chapter 2 — Overview of the DW3000

> **Source:** DW3000 User Manual, Chapter 2 (pages 10–32)

---

## 2.1 Introduction

The DW3000 consists of an **analog front-end** (RF and baseband — receiver and transmitter) and a **digital back-end**. The digital back-end interfaces to a host processor via a standard SPI interface, controls the analog front-end, accepts data from the host for transmission, and provides received data back to the host.

The DW3000 implements HRP UWB PHY as specified by IEEE 802.15.4 [1] and the BPRF mode of the IEEE 802.15.4z amendment [2]. Two device variants exist:

| IC Variant | Package  | PDoA Support |
| ---------- | -------- | ------------ |
| DW3110     | WLCSP52  | No           |
| DW3120     | WLCSP52  | Yes          |
| DW3210     | QFN40    | No           |
| DW3220     | QFN40    | Yes          |

Key capabilities:
- UWB channels 5 and 9 (6489.6 MHz and 7987.2 MHz)
- 2-way ranging, TDoA, and optionally PDoA location schemes
- Data rates of 850 kb/s and 6.8 Mb/s
- Integrated AES CCM\* and AES GCM 128/192/256
- Enhanced Time-of-Flight security modes using STS
- SPI interface up to 38 MHz
- Asset location accuracy of ~10 cm
- Low power consumption (coin cell battery suitable)

---

## 2.2 Comparison and Compatibility with DW1000

The DW3000 is **not software-compatible** with the DW1000. Key differences:

| Feature | DW3000 vs DW1000 |
| ------- | ---------------- |
| Power consumption | Reduced peak and mean |
| Channels | Added channel 9 (8 GHz); removed channels 1, 2, 3, 4, 7 |
| BOM | Reduced — integrated balun and filters on-die |
| Software interface | Simplified; different register map |
| PDoA | Family variant supports single-chip PDoA measurement |
| Security | Enhanced ToF security modes using STS |
| AES | Integrated hardware AES (GCM and CCM\*) 128/192/256 |
| 110 kb/s data rate | **Not supported** |
| Smart TX | **Not supported** — must be implemented on host |
| Compatibility | Backwards compatible on channel 5 at 850 kb/s and 6.81 Mb/s with modified software |

> **Migration note:** Only channel 5 (6.5 GHz) is common between DW1000 and DW3000. Both 16 and 64 MHz PRFs and both data rates (850 kb/s, 6.81 Mb/s) are supported on channel 5. The DW3000 control register interface is different — modified software is required.

---

## 2.3 Interfacing to the DW3000

### 2.3.1 The SPI Interface

The DW3000 host communications interface is a **slave-only SPI** compliant with industry protocol. The host must include a SPI master controller to communicate with the DW3000.

The host controls the DW3000 via SPI by reading/writing configuration and status registers, data buffers, and issuing commands. The SPI-accessible registers are detailed in Chapter 8; fast commands in Chapter 9.

The SPI operating mode (polarity and phase) is determined at reset or wake-up: **GPIO5 and GPIO6 are sampled** to select SPI polarity and phase respectively. SPI mode can also be stored in OTP to avoid needing external components.

> **Note:** Octets are presented on SPI with **MSB first**. The first bit in the transaction sequence determines direction: `0` = SPI read, `1` = SPI write.

#### SPI Transaction Types

SPI transactions are enveloped by asserting (low) the **SPICSn** (chip select) line. Each transaction starts with a **1 or 2-octet header** followed by the transaction data.

The 32 register files are organised by a **5-bit base address** (register file ID). Sub-addressing within a register file uses a **7-bit sub-address offset**.

| Transaction Type | Description |
| ---------------- | ----------- |
| **Fast command** | Single octet: 5-bit fast command code encapsulated by control bits. No data phase. |
| **Short addressed** | 1-octet header: 5-bit base address, RD/WR bit. Access register files 0x00–0x1F. |
| **Full addressed** | 2-octet header: 5-bit base address + 7-bit sub-address. Access any sub-address 0x00–0x7F. |
| **Masked write** | 2-octet header + AND mask + OR mask. Modifies sub-fields without a read-modify-write cycle. **Do not use on write-1-to-clear event status bits.** |

**SPI header format summary (Figure 2 in manual):**

```
Fast command (1 octet):
 bit7  bit6   bits5:1   bit0
  1     0    Fast Cmd    1

Short addressed (1 octet + N data bytes):
 bit7  bit6  bits6:1  bit0
 RD/WR  0   Base Addr   0    [X octet data]

Full addressed (2 octets + N data bytes):
 byte0: RD/WR 1 [5-bit base addr]
 byte1: [7-bit sub-addr] M1 M0    [X octet data]
   (M1:M0 = 00 for plain write/read)

Masked write (2 octets + masks):
 byte0: 1 1 [5-bit base addr]
 byte1: [7-bit sub-addr] M1 M0
   M1:M0 = 01 → 1-octet AND + 1-octet OR masks
   M1:M0 = 10 → 2-octet AND + 2-octet OR masks
   M1:M0 = 11 → 4-octet AND + 4-octet OR masks
```

#### SPI CRC Mode

When enabled (via `SPI_CRCEN` bit in `SYS_CFG`), each SPI write from host must be followed by an 8-bit CRC (CRC-8-ATM polynomial: G(x) = x⁸ + x² + x + 1) computed over all preceding bytes written during the SPI transaction. A mismatch sets the `SPICRCE` event bit in `SYS_STATUS`.

For SPI reads, the DW3000 appends the CRC to the response. The host reads it from `SPI_RD_CRC` register and verifies independently.

> **Note:** SPI CRC mode is disabled by default. It is generally unnecessary for a local digital interface, but may be useful when SPI lines are long or during debug.

> **Note:** Maximum SPI rate when SPI CRC mode is enabled is **20 MHz**.

### 2.3.2 Interrupts

The DW3000 asserts its **IRQ pin** when one or more enabled status events occur. The host interrupt handler reads `SYS_STATUS` to identify the event.

- **IRQ polarity**: configured by `HIRQ_POL` bit in `DIAG_TMC` register. Default = **active high**.
- IRQ pin **floats in SLEEP and DEEPSLEEP** states — pull low externally to prevent spurious interrupts.
- Event status bits in `SYS_STATUS` are **write-1-to-clear**.
- Each event bit has a corresponding enable bit in `SYS_ENABLE`. Setting an enable bit allows that event to assert IRQ.
- On power-up, `SPIRDY_EN` is set to 1 by default so the SPI-ready interrupt is enabled.

### 2.3.3 General Purpose I/O

The DW3000 provides **9 GPIO pins** (GPIO0–GPIO8). By default on power-up, all GPIOs are configured as **inputs** (GPIO_DIR = 0xFF, direction = input).

GPIO configuration is controlled via the `GPIO_CTRL` register (register file `0x05`). Some GPIO lines have alternative functions:

| GPIO Pin | Alternative Function |
| -------- | -------------------- |
| GPIO0/RXOKLED | RXOK LED output |
| GPIO1/SFDLED | SFD LED output |
| GPIO2/RXLED | RX LED output |
| GPIO3/TXLED | TX LED output |
| GPIO4/EXTPA | External Power Amplifier enable |
| GPIO5/EXTTXE/SPIPOL | External TX Enable / SPI polarity select |
| GPIO6/EXTRXE/SPIPHA | External RX Enable / SPI phase select |
| SYNC/GPIO7 | External clock synchronisation input |
| IRQ/GPIO8 | Interrupt request output (default) |

### 2.3.4 The SYNC Pin

The SYNC pin (shared with GPIO7) is used for **external clock synchronisation**. It allows multiple DW3000 devices to synchronise their system clocks to a common external reference, enabling TDoA (Time Difference of Arrival) location systems. See Section 7.1 of the manual for full details.

---

## 2.4 DW3000 Operational States

### 2.4.1 State Diagram

The DW3000 has the following operational states (Figure 7 in manual):

```
         OFF
          │  AON releases digital reset
          ▼
        WAKEUP ◄──────────────────── SLEEP (sleep count done)
          │  AON configuration download     ▲ Auto-to-sleep
          ▼                                 │
        INIT_RC (~30 MHz)          DEEP SLEEP
          │  AON download complete     ▲ Auto-to-sleep / I/O wakeup
          ▼                            │
        IDLE_RC (~120 MHz) ───────────►┘
          │  PLL locked
          ▼
┌──── IDLE_PLL (125 MHz) ────────────────────────┐
│TX_EN│                                    │RX_EN│
▼     │                                    │     ▼
TX_WAIT                                    RX_WAIT
│                                                │
▼                                                ▼
TX ──────────────────────────────────────────► RX
```

### 2.4.2 Operational States Description

| State | Description |
| ----- | ----------- |
| **OFF** | Completely powered off. No voltages applied. 0 µA consumption. |
| **WAKE_UP** | AON sequencer starts primary power and clock blocks. Transitions automatically to INIT_RC. |
| **INIT_RC** | Lowest power state with SPI access (limited to 7 MHz). System clocked from FAST_RC ÷ 4 (~30 MHz). |
| **IDLE_RC** | Lowest power with full-speed SPI. System clocked from FAST_RC (~120 MHz). |
| **IDLE_PLL** | PLL running at 125 MHz nominal. SPI up to 38 MHz. Analog RX/TX circuits are powered down. Host initiates TX or RX from this state. |
| **TX_WAIT** | Delayed TX in progress. DW3000 counting down to programmed TX time. TX analog blocks not yet on. |
| **TX** | Actively transmitting: preamble → SFD → (STS) → PHR → PHY Payload. Returns to IDLE_PLL on completion (or SLEEP if ATX2SLP set). |
| **RX_WAIT** | Delayed RX in progress. DW3000 counting down to programmed RX time. RX analog blocks not yet on. |
| **RX** | Actively receiving: hunting for preamble, then SFD, then receiving packet. All RX blocks active. Half-duplex — cannot TX and RX simultaneously. Returns to IDLE_PLL on completion or error (or SLEEP if ARX2SLP set). |
| **SLEEP** | < 1 µA consumption. Internal LDOs off. Low-power oscillator (~20 kHz) runs sleep counter. Wake via WAKE_UP pin, SPICSn, or sleep timer expiry. SPI not accessible. |
| **DEEPSLEEP** | < 250 nA consumption. Lowest power state. Only AON memory active. Wake via WAKE_UP pin or SPICSn assertion (≥500 µs). SPI not accessible. RSTn pin also wakes and fully resets. |

> **Note:** The DW3000 is a **half-duplex** transceiver — it cannot be in TX and RX states simultaneously.

### 2.4.3 Clock Periods and Frequencies

The chipping rate specified by IEEE 802.15.4 for HRP UWB PHY is **499.2 MHz**. All IC system clocks are referenced to this frequency:

| Clock Reference | Nominal Value | Actual Value |
| --------------- | ------------- | ------------ |
| System clock | 125 MHz | 124.8 MHz (crystal 38.4 MHz × 13 ÷ 4) |
| System clock period | 8 ns | 1/(124.8×10⁶) s |
| 1 GHz PLL clock | 1 GHz | 998.4 MHz |
| Ranging sampling clock | 63.8976 GHz | 63.8976 GHz (15.65 ps period) |

### 2.4.4 Pulse Repetition Frequency (PRF)

The PRF values quoted in documentation are **nominal approximations**:
- **16 MHz PRF** — mean PRF slightly higher for SHR vs PHR/data portions
- **64 MHz PRF** — mean PRF slightly higher for SHR vs PHR/data portions

### 2.4.5 Data Rate

The **6.8 Mb/s** data rate is equivalent to the 6.81 Mb/s data rate in the standard. Quoted rates are rounded nominal values based on the data symbol rate multiplied by the Reed-Solomon (RS) coding rate of 0.87 (= 330/378, because RS adds 48 parity bits per 330 bits data).

---

## 2.5 Power On Reset (POR)

### 2.5.1 Cold Start Sequence

When power (VDD1) is first applied:

1. POR circuit compares supply voltage to internal threshold (~1.5 V). Once threshold passed, AON block is released from reset and `EXTON` is asserted.
2. VDD2a and VDD3 supplies are monitored. Once above required levels, FAST_RC oscillator (~500 µs) and XTAL oscillator (~1 ms) start.
   > **Warning:** If VDD2a or VDD3 takes more than **10 ms** to stabilise, the device must be reset once those supplies are up.
3. Digital core held in reset until crystal oscillator is stable.
4. Once digital reset de-asserts, device enters **INIT_RC** state.
5. AON and OTP configurations are restored into configuration registers.
6. Device enters **IDLE_RC** state. Host sets `AINIT2IDLE` bit in `SEQ_CTRL` to enable PLL and enter **IDLE_PLL**.

GPIO5 and GPIO6 are sampled at reset to set the SPI mode.

**Cold start timing (Figure 8 in manual):**
```
VDD1 rises → EXTON asserted
   → VDD2a/VDD3 rise (<10 ms) → LP Osc + Fast Osc start (~500 µs) → XTAL starts
   → ~20 LP oscillator cycles from VDDx OK → WAKEUP state
   → OTP boot (~70 µs) → INIT_RC → IDLE_RC
   → Host interrupt (SPI @ 38 MHz allowed) → IDLE_PLL
```

### 2.5.2 SLEEP and DEEPSLEEP

**DEEPSLEEP** (`< 250 nA`): IC almost completely powered down. Only "always-on" (AON) memory retains configuration. Wake requires:
- Asserting **WAKE_UP** input for ≥500 µs, or
- Asserting **SPICSn** (chip select) low for ≥500 µs
  > **Note:** When using SPICSn to wake, hold SPIMOSI **low** during SPICSn assertion to prevent a spurious write.

**SLEEP** (`~1 µA`): Low-power oscillator (~20 kHz) runs sleep timer. Wake via WAKE_UP, SPICSn, or internal **sleep timer expiry** (configured via `SLEEP_TIM` in `AON_CFG` with `WAKE_CNT` bit set).

After wake from either state, device progresses: WAKEUP → INIT_RC → IDLE_RC. If `AINIT2IDLE` was set before sleep, it transitions further to **IDLE_PLL**. Otherwise host must set it manually.

> **Important:** Prior to entering SLEEP/DEEPSLEEP, clear the `AINIT2IDLE` bit in `SEQ_CTRL`. This ensures the device stays in **IDLE_RC** after wake-up so the host can load `LDOTUNE_CAL` values from OTP before enabling the PLL.

### 2.5.3 Configuration Register Preservation (AON Memory)

Before entering SLEEP or DEEPSLEEP, main DW3000 configuration registers are **saved** (copied) into the Always-On (AON) memory. Upon waking, registers are **restored** from AON memory prior to exiting INIT_RC.

AON memory is powered at all times, including during SLEEP and DEEPSLEEP. The save/restore and OTP boot takes **~85 µs** (warm start from SLEEP/DEEPSLEEP, see Figure 9).

> **Important:** The host should wait for the `SPIRDY` interrupt before using SPI after wake-up to avoid corrupting configuration values.

**Registers preserved in AON memory (Table 5):**

`AON_DIG_CFG`, `XTAL`, `PLL_CAL`, `PANADR`, `SYS_CFG`, `FF_CFG`, `TX_FCTRL`, `DREF_TIME`, `RX_FWTO`, `SYS_ENABLE`, `TX_ANTD`, `ACK_RESP_T`, `TX_POWER`, `CHAN_CTRL`, `AES_IV0–3`, `DMA_CFG`, `AES_KEY`, `STS_CFG`, `STS_KEY`, `STS_IV`, `GPIO_MODE`, `DTUNE0`, `RX_SFD_TOC`, `PRE_TOC`, `DTUNE3`, `PLL_CFG`, `CIA_CONF`, `FP_CONF`, `IP_CONF`, `STS_CONF_0/1`, `SEQ_CTRL`, `TXFSEQ`, `LED_CTRL`, `RX_SNIFF`, `DGC_CFG`, `RF_SWITCH`, `GPIO_PULL_EN`, `CIA_ADJUST`

### 2.5.4 Loading LDO Calibration from OTP

When waking from SLEEP or DEEPSLEEP, it is necessary to load the **`LDOTUNE_CAL`** value from OTP (programmed during IC production calibration) into the LDO tuning registers. This must be done while in IDLE_RC before enabling the PLL.

### 2.5.5 Default System Configuration

On power-up, the DW3000 default operational configuration is (Table 6):

| Parameter | Default Value |
| --------- | ------------- |
| Channel | **5** (Cf = 6489.6 MHz) |
| Data Rate | 6.8 Mb/s |
| PHR Rate | 850 kb/s |
| PRF | 64 MHz |
| Preamble Length | 64 symbols |
| Preamble Code | 9 |
| STS | Off |
| STS Sequence Length | n/a |
| SFD | IEEE802.15.4z [2] length 8 |

**Default GPIO functions (Table 7):**

| GPIO Pin | Default Function |
| -------- | ---------------- |
| GPIO0/RXOKLED | GPIO0 |
| GPIO1/SFDLED | GPIO1 |
| GPIO2/RXLED | GPIO2 |
| GPIO3/TXLED | GPIO3 |
| GPIO4/EXTPA | GPIO4 |
| GPIO5/EXTTXE/SPIPOL | GPIO5 |
| GPIO6/EXTRXE/SPIPHA | GPIO6 |
| SYNC/GPIO7 | SYNC |
| IRQ/GPIO8 | IRQ |

**Other defaults:**
- Frame wait timeout (`RXWTOE` in `SYS_CFG`): **off**
- Preamble detection timeout (`PRE_TOC`): **off**
- SFD detection timeout (`RX_SFD_TOC`): **on**
- RXAUTR (auto receiver re-enable), MAC features (FFEN, double buffering `DIS_DRXB`, `AUTO_ACK`): all **off**
- Auto CRC generation (`DIS_FCS_TX`): **on**
- CIA (Channel Impulse Analysis) algorithm (`CIARUNE`): **enabled** by default — calculates accurate ToA on every packet reception. Disable via `CIARUNE` in `SEQ_CTRL` for data-only applications.
- External synchronisation and external PA: **off** by default

**Default channel configuration** (`CHAN_CTRL`): Channel 5, preamble code 9, 64 MHz PRF.

**Default transmitter configuration**: TX data rate = 6.8 Mb/s (`TXBR` in `TX_FCTRL`). Preamble symbol repetition length = 64 symbols (`TXPSR` in `TX_FCTRL`). RF channel config for channel 5 set in `RF_TX_CTRL_2`.

**Default receiver configuration**: Digital RX tuning registers set for 64 MHz PRF, 6.8 Mb/s data rate, preamble symbol repetition of 64.

---

## 2.6 UWB Channels and Preamble Codes

The DW3000 supports **two UWB channels** from the IEEE 802.15.4 standard (Table 8):

| Channel | Centre Frequency (MHz) | Bandwidth (MHz) | Preamble Codes (16 MHz PRF) | Preamble Codes (64 MHz PRF) |
| ------- | ---------------------- | --------------- | --------------------------- | --------------------------- |
| **5** | **6489.6** | 499.2 | 3, 4 | 9, 10, 11, 12 |
| **9** | **7987.2** | 499.2 | 3, 4 | 9, 10, 11, 12 |

The combination of channel number and preamble code is termed a **complex channel**. Preamble codes were chosen for low cross-correlation so that multiple networks can operate independently.

**Dynamic Preamble Select (DPS):** IEEE 802.15.4 feature where devices switch preamble codes during a ranging exchange to increase spoofing/eavesdropping difficulty. At 64 MHz PRF, additional DPS preamble codes are available: **13, 14, 15, 16, 21, 22, 23, 24**.

---

## 2.7 Data Modulation Scheme

UWB data transmission uses **BPM/BPSK** (Burst Position Modulation / Binary Phase-Shift Keying):

- Each data bit passes through a **convolutional encoder** generating a systematic bit (position) and a parity bit (phase).
- The **systematic (position) bit** determines which half of the symbol interval contains the burst.
- The **parity (sign) bit** determines whether the burst is positive or negative.
- Within each symbol quarter, the burst sub-interval is further divided into 2, 4, or 8 sub-intervals with a pseudo-random sequence determining actual burst placement — improving interference immunity and spectral whitening.

**Forward error correction (FEC):**
- PHR and data portions use a **Reed-Solomon (RS) code** applied to the data.
- RS coding rate = **0.87** (330/378) — adds 48 parity bits per 330 data bits.
- SECDED (Single Error Correct, Double Error Detect) is used for the PHR.
- Both SECDED and RS codes are *systematic* — data can be recovered without using the codes (e.g. by a non-coherent receiver), at reduced sensitivity.
- At 850 kb/s nominal rate, the PHR is actually transmitted at **975 kb/s** (no RS coding on PHR).

---

## 2.8 Synchronisation Header Modulation Scheme

The **SHR (Synchronisation Header)** consists of the **preamble sequence** and the **SFD** (Start of Frame Delimiter). It uses **single pulses** (not BPM/BPSK bursts).

The preamble symbol period is divided into approximately 500 "chip" time intervals (496 for 16 MHz PRF, 508 for 64 MHz PRF), each at the 499.2 MHz chip rate:

| Mean PRF (MHz) | Chips Per Symbol | Preamble Symbol Duration (ns) |
| -------------- | ---------------- | ----------------------------- |
| 16 nominal | 496 | 993.59 |
| 64 nominal | 508 | 1017.63 |

The preamble length is configured by the number of **Preamble Symbol Repetitions (PSR)**. Standard PSR values: **16, 64, 1024, 4096**. DW3000 additionally supports: **128, 256, 512, 1536, 2048**. (DW3000 cannot receive packets with preamble length below 32 symbols.)

The preamble sequence has **perfect periodic autocorrelation**, allowing the receiver to:
1. Make use of multipath energy (extending range)
2. Resolve the channel impulse response and determine the arrival time of the first (direct) path — enabling precise RTLS timestamps

**SFD sequences:** Mark the transition from preamble to PHR. Two SFD formats:
- **Standard length-8 SFD** (IEEE 802.15.4): `0, +1, 0, -1, +1, 0, 0, -1`
- **IEEE 802.15.4z length-8 SFD** (default, improved coherent receiver performance): `-1, -1, +1, -1, -1, -1, +1, -1`

The STS (if enabled, SP1–SP3 modes) provides a cryptographically secure timestamp sequence for improved ToA integrity — see Chapter 6.

---

## 2.9 PHY Header: Standard Data Frame Length

The **PHR (PHY Header)** is a **19-bit field** transmitted immediately after the SFD using BPM/BPSK modulation (without Reed-Solomon coding). It uses a **6-bit SECDED** parity sequence.

```
Standard PHR bit assignment (Figure 11 in manual):
 Bit:  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
       R1  R0  L6  L5  L4  L3  L2  L1  L0 RNG EXT  P1  P0  C5  C4  C3  C2  C1  C0
       └──┘ └──────────────────────────┘  └─┘ └─┘  └──┘   └──────────────────────┘
      Rate     Frame Length (7-bit)     Rng  HdrExt  Preamble Duration  SECDED bits
```

| Field | Bits | Description |
| ----- | ---- | ----------- |
| **R1:R0** | 0–1 | Data rate: `00` = 850 kb/s, `10` = 6.8 Mb/s |
| **L6:L0** | 2–8 | Frame length in bytes (0–127, standard mode) |
| **RNG** | 9 | Ranging frame bit: `1` = this is a ranging frame |
| **EXT** | 10 | Header Extension bit — reserved by IEEE for future extensions; always `0` |
| **P1:P0** | 11–12 | Preamble Duration (reported in `RXPSR` of `RX_FINFO`) |
| **C5:C0** | 13–18 | SECDED check bits (hardware-generated on TX, verified on RX) |

The DW3000 fills in Data Rate, Frame Length, Ranging frame, and Preamble Duration from `TX_FCTRL` configuration and generates the SECDED sequence automatically.

---

## 2.10 Extended PHY Header: Extended Data Frame Length

Standard IEEE 802.15.4-2020 UWB packets carry up to **127 bytes** of payload. The DW3000 also supports an extended mode allowing frame lengths up to **1023 bytes**, enabled by the `PHR_MODE` bits in `SYS_CFG` (`Sub-register 0x00:10`).

> **Warning:** Extended PHR mode is **not IEEE 802.15.4 compliant** (defined in IEEE 802.15.8). **Both ends must use the same mode.** If only one end is in extended mode, communication will fail with PHR errors. The inverted SECDED sequence will cause PHR errors when receiving from any standard-mode device.

In extended mode, the PHR is redefined to carry a **10-bit frame length** (L9:L0):

```
Extended PHR bit assignment (Figure 12 in manual):
 Bit:  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
       R1  R0  L9  L8  L7  L6  L5  L4  L3  L2  L1  L0  P0  S5  S4  S3  S2  S1  S0
       └──┘ └────────────────────────────────────────┘  └─┘  └──────────────────────┘
      Rate          Frame Length (10-bit, 0–1023)      PreamDur   SECDED bits (inverted)
```

| Field | Description |
| ----- | ----------- |
| **R1:R0** | Data rate (same encoding as standard PHR) |
| **L9:L0** | Frame length in bytes (0–1023), MSB transmitted first |
| **P0** | Preamble Duration: `0` = 64–1024 symbols, `1` = 1536–4096 symbols |
| **S5:S0** | SECDED check bits — **inverted** relative to standard: S*n* = NOT(C*n*) |

**Preamble Duration field (P0):**

| P0 | Preamble Length (BPM-BPSK mode) |
| --- | ------------------------------- |
| `0` | 64 to 1024 symbols |
| `1` | 1536 to 4096 symbols |

The `FINE_PLEN` field in `TX_FCTRL` can alternatively set any **multiple of 8 symbols** from 32 to 2048.

The **SECDED inversion** (S*n* = NOT C*n*) is specified in IEEE 802.15.8 for frames up to 1023 octets.

In long frame mode, only the **high-order bit of `TXPSR`** from `TX_FCTRL` is sent in the PHR `P0` field. On the receiver side, `RXPSR` in `RX_FINFO` reflects the received `P0` value.

> **Note:** The probability of a frame error increases with frame length. Increasing frame length may or may not improve system throughput depending on the channel error rate and the need for retransmissions.
