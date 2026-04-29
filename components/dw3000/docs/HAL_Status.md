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
| AON sleep/wake | Usable foundation | AON_DIG_CFG/AON_CFG helpers, direct AON RAM access, sleep counter programming, sleep/deepsleep entry, SPIRDY wake completion with OTP LDO/BIAS reload and optional RX/PLL post-wake hooks. |
| GPIO | Usable foundation | GPIO clocks, mode/function packing, direction, pulls, output state, raw reads, IRQ sense/mode/both-edge/debounce setup, IRQ latch clear, LED and external PA/LNA helpers. |
| ACC/CIR | Usable foundation | Accumulator clock handling, dummy-octet discard, sample-indexed direct/indirect reads, raw and decoded 24-bit complex CIR samples, Ipatov/STS CIR spans, CIADONE check helper. |
| AES | Usable foundation | AES_CFG/DMA/IV/key helpers, key RAM/scratch access, CCM*/GCM IV helpers, status/event control, start/wait/run wrappers, and transfer bounds validation. |
| OTP-backed init orchestration | Usable foundation | Full initialize/configure-device wrappers that order status clear, PMSC, OTP-backed calibration, PHY, MAC, STS, CIA, GPIO, AES, AON, RX defaults, and deferred IDLE_PLL entry. |

## Remaining

| Area | Needed next | Why it matters |
| --- | --- | --- |
| PLL/RF/TX calibration HALs | Dedicated wrappers around PLL, RF, and TX_CAL register files. | Regulatory testing, TX power calibration, and temperature compensation need controlled workflows. |
| External sync HAL | EC_CTRL/RX_CAL flow and deterministic timebase reset support. | Multi-anchor sync and wired reference-clock systems need this. |
| ESP-IDF port/examples/tests | Concrete SPI/reset/IRQ port adapter, example app, mocks or host tests. | The HAL compiles, but it still needs integration proof and repeatable verification. |

## Current Priority

1. PLL/RF/TX calibration HALs.
2. External sync HAL.
3. ESP-IDF port/examples/tests.
