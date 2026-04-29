# DW3000 HAL Status

This file tracks the component HAL surface so unfinished areas stay explicit.

## Implemented

| Area | Status | Notes |
| --- | --- | --- |
| Register transport | Usable | Direct and indirect SPI register access, typed 8/16/32-bit helpers, register descriptors. |
| Core bring-up | Usable foundation | Context init, hard reset, SPI-ready wait, device ID probe, capability detection, optional IDLE_PLL entry. |
| Fast commands | Usable foundation | Fast command encoding and basic host-side state tracking. |
| Status/interrupts | Usable foundation | SYS_STATUS read/clear and SYS_ENABLE masking/cache. |
| PHY/RX tuning | Usable foundation | Channel, PRF, data rate, preamble/SFD/PHR, RF/PLL channel constants, RX tuning registers. |
| MAC helpers | Usable foundation | EUI/PANADR and frame-filter owned bits. |
| TX | Usable foundation | TX buffer/frame control, delayed/reference time, antenna delay, timestamp read, TX start commands. |
| RX | Usable foundation | RX SYS_CFG bits, frame wait timeout, RX buffer reads, RX metadata, sniff mode, RX start commands. |
| CIA | Usable foundation | Antenna delay, CIA path timestamp/diagnostic reads, PDoA/TDoA helpers, TOAST decoding. |
| STS | Usable foundation | STS length/key/IV, STS SYS_CFG bits, ACC_QUAL checks, LOAD_IV/RST_LAST helpers. |
| PMSC/power state | Usable foundation | Clock control, sequencer flags, CPLOCK wait, IDLE_PLL entry, FORCE2INIT to IDLE_RC, SOFT_RST pulse, TXFSEQ, LED, BIAS_CTRL access. |
| OTP | Usable foundation | OTP word reads, SRDATA read, DGC/LDO/BIAS/OPS kick helpers, guarded word programming API. |
| Calibration | Usable foundation | TX power, XTAL trim, antenna-delay wrappers, SAR measurement/conversion, RX calibration, PGC helpers, PLL recalibration, OTP factory-kick helper. |

## Remaining

| Area | Needed next | Why it matters |
| --- | --- | --- |
| AON sleep/wake HAL | AON config/save/restore, sleep/deepsleep entry, wake-source setup, post-wake recalibration hooks. | Low-power operation is not safe without explicit retained config and wake sequencing. |
| AES HAL | AES_CFG, IV/key, DMA setup, start/status, CCM*/GCM workflow wrappers. | Data security and hardware AES offload are exposed only as register types today. |
| GPIO HAL | GPIO mode, direction, pulls, IRQ/debounce, LED and external PA/LNA modes. | Board integration, interrupts, activity LEDs, and RF front-end control need a stable API. |
| OTP-backed init orchestration | Top-level ordered init that applies OTP/calibration, PMSC, PHY, MAC, STS, CIA, TX/RX defaults. | The current pieces are individually callable; a product-ready component needs a safe default sequence. |
| ACC/CIR HAL | Safe accumulator reads, dummy-byte handling, indirect reads above offset 127, CIR sample decoding. | Diagnostics, NLOS analysis, and advanced timestamp quality checks need CIR data. |
| PLL/RF/TX calibration HALs | Dedicated wrappers around PLL, RF, and TX_CAL register files. | Regulatory testing, TX power calibration, and temperature compensation need controlled workflows. |
| External sync HAL | EC_CTRL/RX_CAL flow and deterministic timebase reset support. | Multi-anchor sync and wired reference-clock systems need this. |
| ESP-IDF port/examples/tests | Concrete SPI/reset/IRQ port adapter, example app, mocks or host tests. | The HAL compiles, but it still needs integration proof and repeatable verification. |

## Current Priority

1. AON sleep/wake HAL.
2. GPIO and ACC/CIR HALs.
3. AES and advanced sync/RF/TX calibration workflows.
4. OTP-backed init orchestration.
5. ESP-IDF port/examples/tests.
