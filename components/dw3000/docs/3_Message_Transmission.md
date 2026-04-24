# Chapter 3 — Message Transmission

> **Source:** DW3000 User Manual, Chapter 3 
> **Pages:** 33–37

---

## 3.1 Basic Transmission

The DW3000 can transmit packets with or without data payload. It supports **four packet formats** defined by the IEEE 802.15.4z amendment, determined by the **STS (Scrambled Timestamp Sequence)** configuration.

### 3.1.1 Packet Formats (SP0–SP3)

The arrow in each diagram marks the **RMARKER** — the reference timestamp position (see Section 3.2).

```
SP0 — Standard IEEE 802.15.4 UWB (no STS):
┌─────────────────┬─────┬─────┬─────────────┐
│  Ipatov Preamble│ SFD │ PHR │ PHY Payload │
└─────────────────┴─────┴─────┴─────────────┘
                          ↑ RMARKER

SP1 — STS follows SFD:
┌─────────────────┬─────┬─────┬─────┬─────────────┐
│  Ipatov Preamble│ SFD │ STS │ PHR │ PHY Payload │
└─────────────────┴─────┴─────┴─────┴─────────────┘
                          ↑ RMARKER

SP2 — STS after PHY Payload:
┌─────────────────┬─────┬─────┬─────────────┬─────┐
│  Ipatov Preamble│ SFD │ PHR │ PHY Payload │ STS │
└─────────────────┴─────┴─────┴─────────────┴─────┘
                    ↑ RMARKER

SP3 — STS only (no PHR or PHY Payload):
┌─────────────────┬─────┬─────┐
│  Ipatov Preamble│ SFD │ STS │
└─────────────────┴─────┴─────┘
                    ↑ RMARKER
```

| Format  | STS Position          | PHR + PHY Payload | Use Case                         |
| ------- | --------------------- | ----------------- | -------------------------------- |
| **SP0** | None                  | Yes               | Standard UWB, no security        |
| **SP1** | Between SFD and PHR   | Yes               | Secure ranging with data         |
| **SP2** | After PHY Payload     | Yes               | Secure ranging, post-payload STS |
| **SP3** | Immediately after SFD | No                | Secure ranging, STS only         |

> **Note:** In SP3, the PHR and PHY Payload are omitted entirely. STS must be enabled and configured via the STS configuration registers before using SP1/SP2/SP3.

### 3.1.2 Basic TX Sequence

The DW3000 begins in **IDLE_PLL** state awaiting host instructions. The basic transmit sequence:

```
IDLE_PLL
    │
    ▼
[Host writes frame data to TX_BUFFER]
    │
    ▼
[Host configures TX_FCTRL + CHAN_CTRL]
  (preamble length, frame length, data rate, PRF)
    │
    ▼
[Host issues TX command, e.g. CMD_TX]
    │
    ▼
TX state — DW3000 transmits: Preamble → SFD → (STS) → PHR → PHY Payload → (STS)
    │
    ▼
TX complete — TXFRS event bit set in SYS_STATUS
    │
    ├─── Auto-Sleep enabled? ──Yes──► SLEEP / DEEPSLEEP
    │
    └─── No ──► IDLE_PLL
```

### 3.1.3 Host Configuration Steps

Before issuing a TX command, the host must:

1. **Write frame data** to `TX_BUFFER` (register `0x14`) — the MAC frame bytes (FCF, Sequence Number, Addressing, Payload).
2. **Set TX_FCTRL** (register `0x00:24`) — configure:
   - `TXFLEN`: Frame length in bytes (including 2-byte FCS, which is auto-appended)
   - `TXBR`: Data rate (`0` = 850 kbps, `1` = 6.8 Mbps)
   - `TXPSR`: Preamble symbol repetitions
   - `FINE_PLEN`: Fine preamble length (optional)
   - `PE`: Preamble extension mode
3. **Set CHAN_CTRL** (register `0x00:14`) — configure channel (5 or 9), PRF (16 or 64 MHz), TX/RX preamble codes (`TX_PCODE`, `RX_PCODE`).
4. **Issue TX command** — one of the fast commands (see Section 3.5).

The DW3000 hardware **automatically appends the FCS (CRC-16/ITU-T)** based on `TXFLEN`. The host must include the 2 FCS bytes in the `TXFLEN` count but does **not** write FCS bytes to `TX_BUFFER`.

### 3.1.4 Key TX Registers

| Register    | Address   | Description                                              |
| ----------- | --------- | -------------------------------------------------------- |
| `TX_BUFFER` | `0x14:00` | TX frame data buffer (up to 1024 bytes)                  |
| `TX_FCTRL`  | `0x00:24` | TX frame control: frame length, data rate, PRF, preamble |
| `CHAN_CTRL` | `0x00:14` | Channel, PRF, TX/RX preamble codes                       |
| `TX_ANTD`   | `0x00:1C` | TX antenna delay (16-bit, added to raw TX timestamp)     |
| `TX_TIME`   | `0x00:74` | TX timestamp result (read after TXFRS event)             |
| `DX_TIME`   | `0x00:2C` | Delayed TX target time (for CMD_DTX)                     |
| `DREF_TIME` | `0x00:30` | Reference time for relative delayed TX (for CMD_DTX_REF) |

---

## 3.2 Transmission Timestamp

### 3.2.1 RMARKER Definition

The **RMARKER** is the IEEE 802.15.4z-defined transmit reference point: the time when the beginning of the first symbol following the SFD arrives at the local antenna (precisely: the peak pulse location associated with the first chip following the SFD).

### 3.2.2 TX Timestamp Capture

During packet transmission, the DW3000 internally:

1. Records the **raw TX timestamp** (`TX_RAWST`) — the system clock counter value at the moment it begins sending the first chip following the SFD.
2. Adds the **TX antenna delay** (programmed in `TX_ANTD`, a 16-bit value in system clock units) to account for the propagation delay through the RF frontend.
3. Writes the **antenna-adjusted TX timestamp** to the `TX_STAMP` field of register `TX_TIME`.

```
TX_STAMP = TX_RAWST + TX_ANTD
```

The resulting `TX_STAMP` is the best estimate of the actual time the RMARKER was at the antenna, and is the value used for ranging calculations.

### 3.2.3 Reading the TX Timestamp

After the `TXFRS` event bit is set in `SYS_STATUS`, the host reads `TX_TIME` (register `0x00:74`) to obtain the 40-bit TX timestamp. This value is in the DW3000 system time units (1 LSB ≈ 15.65 ps, equivalent to a 64 GHz counter).

> **Note:** The TX antenna delay (`TX_ANTD`) must be calibrated for accurate ranging. See the calibration section (Chapter 10 in the manual) for the calibration procedure.

---

## 3.3 Delayed Transmission

### 3.3.1 Purpose

Delayed transmission allows the host to schedule a packet to be transmitted at a precise future time. This is essential for:

- **Two-way ranging (TWR)**: embedding the known response delay into the transmitted ranging message.
- **Time-synchronized networks**: transmitting frames at TDMA-scheduled slots.
- **Response time optimization**: reducing ranging message count by pre-computing and embedding the TX timestamp into the frame payload before transmitting.

### 3.3.2 Absolute Delayed TX

To schedule transmission at an absolute future time:

1. Write the desired transmit time to **`DX_TIME`** (register `0x00:2C`).
2. Issue the **`CMD_DTX`** fast command.

The DW3000 enters **`TX_WAIT`** state, counts down to the programmed time, then transitions to **`TX`** state automatically.

### 3.3.3 Reference-Relative Delayed TX

To schedule transmission at a time relative to a reference (e.g., the RX timestamp of the preceding message):

1. Write the **reference time** to **`DREF_TIME`** (register `0x00:34`).
2. Write the **time offset** from the reference to **`DX_TIME`**.
3. Issue the **`CMD_DTX_REF`** fast command.

The effective transmit time is `DREF_TIME + DX_TIME`.

### 3.3.4 DX_TIME Resolution

The `DX_TIME` register uses the same time base as the system clock:

- **1 LSB** of `DX_TIME` = 2 ÷ (499.2 × 10⁶) seconds = **~4 ns**
- The **least significant bit is ignored**, giving an effective resolution of **~8 ns**
- To align the transmit time with the RMARKER at the antenna, the **low 9 bits should be zeroed** before adding the TX antenna delay to get the correct `DX_TIME` value

### 3.3.5 Lateness Warning (HPDWARN)

If the host writes a `DX_TIME` that is in the very near future (so close that the DW3000 cannot begin the preamble in time to meet the target RMARKER time), the **`HPDWARN`** event status flag is set in `SYS_STATUS`.

| Scenario                              | HPDWARN | Behavior                                                        |
| ------------------------------------- | ------- | --------------------------------------------------------------- |
| Delay is in the far future            | Not set | Normal delayed TX proceeds                                      |
| Delay already passed                  | Set     | DW3000 waits nearly one full counter period before transmitting |
| Delay in the near future but too late | Set     | TX will still occur, but ~17.2 s late                           |

The host should monitor `HPDWARN` during development. To abort a late transmission, issue `CMD_TXRXOFF`.

> **Design guideline:** Choose delayed TX times that are at least several hundred microseconds in the future to avoid lateness. In deployed product, `HPDWARN` can be used as an error detection mechanism.

---

## 3.4 Extended Length Data Frames

### 3.4.1 Overview

Standard IEEE 802.15.4-2015 HRP UWB frames support payload up to **127 bytes**. The DW3000 also supports an extended mode with frame lengths up to **1023 bytes**, enabled by the `PHR_MODE` bits in the System Configuration register (`SYS_CFG`, sub-register `0x00:10`).

> **Warning:** Extended length frame mode is **not IEEE 802.15.4 compliant**. It is defined in the IEEE 802.15.8 standard. Both the transmitter and receiver must be configured to use the same mode, or communication will fail with PHR errors.

### 3.4.2 Extended PHR Format

In extended length mode, the PHR is redefined to carry 3 additional bits of frame length (10-bit frame length field instead of 7-bit):

```
Standard (SP0) PHR bit assignment (19 bits total):
 Bit: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
      R1 R0 L6 L5 L4 L3 L2 L1 L0  - P0 S5 S4 S3 S2 S1 S0  -  -
      └──┘ └────────────────────┘    └─┘ └──────────────────┘
    Data Rate     Frame Length     Preamble    SECDED Bits
                  (7-bit, std)     Duration

Extended PHR bit assignment (19 bits total):
 Bit: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
      R1 R0 L9 L8 L7 L6 L5 L4 L3 L2 L1 L0 P0 S5 S4 S3 S2 S1 S0
      └──┘ └──────────────────────────────┘ └─┘ └────────────────┘
    Data Rate     Frame Length (10-bit)    Preamble  SECDED Bits
```

| Field     | Size    | Description                                                         |
| --------- | ------- | ------------------------------------------------------------------- |
| **R1:R0** | 2 bits  | Data rate: `00` = 850 kbps, `10` = 6.8 Mbps                         |
| **L9:L0** | 10 bits | Frame length in bytes (0–1023), MSB first                           |
| **P0**    | 1 bit   | Preamble Duration: `0` = 64–1024 symbols, `1` = 1536–4096 symbols   |
| **S5:S0** | 6 bits  | SECDED check bits (inverted relative to standard: S*n* = NOT(C*n*)) |

### 3.4.3 Preamble Duration Field (P0)

| P0  | Preamble Length Range (BPM-BPSK mode) |
| --- | ------------------------------------- |
| `0` | 64 to 1024 symbols                    |
| `1` | 1536 to 4096 symbols                  |

The host may also use the `FINE_PLEN` field in `TX_FCTRL` to set any multiple of 8 symbols from 32 to 2048, overriding the standard table.

### 3.4.4 SECDED Error Check

The extended PHR SECDED bits (S5–S0) protect the PHR from channel impairments. They use the same calculation as the IEEE 802.15.4 standard, but with bits **inverted**:

```
S0 = NOT(C0),  S1 = NOT(C1),  S2 = NOT(C2),
S3 = NOT(C3),  S4 = NOT(C4),  S5 = NOT(C5)
```

> **Note:** The SECDED check sequence inversion means that in long frame mode the DW3000 will report PHR errors when receiving from any device using standard (non-extended) PHR mode. Both ends must agree on the frame mode setting.

### 3.4.5 Enabling Extended Frames

Set the `PHR_MODE` bits in `SYS_CFG` (sub-register `0x00:10`):

| PHR_MODE | Mode                                            |
| -------- | ----------------------------------------------- |
| `0b00`   | Standard IEEE 802.15.4 frames (up to 127 bytes) |
| `0b11`   | Extended length frames (up to 1023 bytes)       |

In extended mode, the high-order bit of `TXPSR` from `TX_FCTRL` is sent in the PHR `P0` field. On the receiver side, the `RXPSR` value in `RX_FINFO` reflects the received `P0`.

---

## 3.5 TX Fast Commands Summary

The DW3000 uses single-octet **fast commands** to initiate TX operations. These are written as a fast command SPI transaction (see Section 2 of the manual for SPI format).

| Command          | Hex Code | Description                                                  |
| ---------------- | -------- | ------------------------------------------------------------ |
| `CMD_TX`         | `0x01`   | Start immediate TX (no delay)                                |
| `CMD_DTX`        | `0x03`   | Start delayed TX at absolute time in `DX_TIME`               |
| `CMD_DTX_RS`     | `0x07`   | Delayed TX, then immediately switch to RX after TX           |
| `CMD_DTX_REF`    | `0x09`   | Delayed TX relative to `DREF_TIME + DX_TIME`                 |
| `CMD_DTX_TS`     | `0x05`   | Delayed TX, time specified relative to last TX timestamp     |
| `CMD_DTX_W4R`    | `0x0D`   | Delayed TX, then wait-for-response (RX) after TX             |
| `CMD_CCA_TX`     | `0x0B`   | Clear Channel Assessment TX (transmit only if channel clear) |
| `CMD_CCA_TX_W4R` | `0x11`   | CCA TX then wait-for-response                                |
| `CMD_TX_W4R`     | `0x0C`   | Immediate TX then wait-for-response (RX)                     |
| `CMD_TXRXOFF`    | `0x00`   | Cancel pending TX/RX and return to IDLE                      |

> **W4R commands:** The "wait-for-response" variants automatically switch the DW3000 from TX to RX mode after transmission completes, without host intervention. This is used for request-response ranging flows.

---

## 3.6 TX Status and Interrupt

After TX completes, the host is notified via:

| Status Bit | Register     | Description                                                     |
| ---------- | ------------ | --------------------------------------------------------------- |
| `TXFRS`    | `SYS_STATUS` | TX Frame Sent — set when the last symbol has been transmitted   |
| `HPDWARN`  | `SYS_STATUS` | Half Period Delay Warning — late delayed TX invocation detected |
| `TXBERR`   | `SYS_STATUS` | TX buffer error                                                 |

To enable the `TXFRS` interrupt on the IRQ pin, set the corresponding enable bit in `SYS_ENABLE` (register `0x00:3C`).

**Typical TX interrupt handler:**

```c
uint32_t status = read_reg(SYS_STATUS);

if (status & SYS_STATUS_TXFRS) {
    clear_bit(SYS_STATUS, SYS_STATUS_TXFRS);   // write-1-to-clear
    uint64_t tx_ts = read_tx_timestamp();        // read TX_TIME
    // process TX complete...
}

if (status & SYS_STATUS_HPDWARN) {
    clear_bit(SYS_STATUS, SYS_STATUS_HPDWARN);
    // handle late TX warning...
}
```

> **Note:** `SYS_STATUS` event bits are **write-1-to-clear**. Do not use a masked-write transaction to clear them (use a direct write of the bit mask).

---

## 3.7 TX Power Configuration

TX output power is controlled via the `TX_POWER` register (`0x00:28`). The DW3000 provides **coarse gain** and **fine gain** control:

| Field      | Description                                |
| ---------- | ------------------------------------------ |
| `TXPOWPHR` | TX power for PHR portion of the packet     |
| `TXPOWSD`  | TX power for SFD and data payload portions |

Both fields use an 8-bit encoding with a **coarse attenuator** (upper bits) and **fine attenuator** (lower bits). The actual output power depends on the channel and must be set to comply with regional regulatory limits.

> **Important:** TX power must be calibrated for the specific board design and regulatory jurisdiction. Decawave/Qorvo provides recommended TX power settings in application notes for common configurations.
