# DW3000 HAL Development Status

This file tracks the component HAL surface so unfinished areas stay explicit.
The higher-level convenience API is tracked separately in `Dev_Status_API.md`.

## Implemented

| Area                          | Status            | Notes                                                                                                                                                                                                   |
| ----------------------------- | ----------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Register transport            | Usable            | Direct and indirect SPI register access, typed 8/16/32-bit helpers, register descriptors.                                                                                                               |
| Core bring-up                 | Usable foundation | Context init, hard reset, SPI-ready wait, device ID probe, capability detection, optional IDLE_PLL entry.                                                                                               |
| Fast commands                 | Usable foundation | Fast command encoding and basic host-side state tracking.                                                                                                                                               |
| Status/interrupts             | Usable foundation | SYS_STATUS read/clear and SYS_ENABLE masking/cache.                                                                                                                                                     |
| PHY/RX tuning                 | Usable foundation | Channel, PRF, data rate, preamble/SFD/PHR, RF/PLL channel constants, RX tuning registers.                                                                                                               |
| MAC helpers                   | Usable foundation | EUI/PANADR and frame-filter owned bits.                                                                                                                                                                 |
| TX                            | Usable foundation | TX buffer/frame control, delayed/reference time, antenna delay, timestamp read, TX start commands.                                                                                                      |
| RX                            | Usable foundation | RX SYS_CFG bits, frame wait timeout, RX buffer reads, RX metadata, sniff mode, RX start commands.                                                                                                       |
| CIA                           | Usable foundation | Antenna delay, CIA path timestamp/diagnostic reads, PDoA/TDoA helpers, TOAST decoding.                                                                                                                  |
| STS                           | Usable foundation | STS length/key/IV, STS SYS_CFG bits, ACC_QUAL checks, LOAD_IV/RST_LAST helpers.                                                                                                                         |
| PMSC/power state              | Usable foundation | Clock control, sequencer flags, CPLOCK wait, IDLE_PLL entry, FORCE2INIT to IDLE_RC, SOFT_RST pulse, TXFSEQ, LED, BIAS_CTRL access.                                                                      |
| OTP                           | Usable foundation | OTP word reads, SRDATA read, DGC/LDO/BIAS/OPS kick helpers, guarded word programming API.                                                                                                               |
| Calibration                   | Usable foundation | TX power, XTAL trim, antenna-delay wrappers, SAR measurement/conversion, RX calibration, PGC helpers, PLL recalibration, OTP factory-kick helper.                                                       |
| AON sleep/wake                | Usable foundation | AON_DIG_CFG/AON_CFG helpers, direct AON RAM access, sleep counter programming, sleep/deepsleep entry, SPIRDY wake completion with OTP LDO/BIAS reload and optional RX/PLL post-wake hooks.              |
| GPIO                          | Usable foundation | GPIO clocks, mode/function packing, direction, pulls, output state, raw reads, IRQ sense/mode/both-edge/debounce setup, IRQ latch clear, LED and external PA/LNA helpers.                               |
| ACC/CIR                       | Usable foundation | Accumulator clock handling, dummy-octet discard, sample-indexed direct/indirect reads, raw and decoded 24-bit complex CIR samples, Ipatov/STS CIR spans, CIADONE check helper.                          |
| AES                           | Usable foundation | AES_CFG/DMA/IV/key helpers, key RAM/scratch access, CCM*/GCM IV helpers, status/event control, start/wait/run wrappers, and transfer bounds validation.                                                 |
| OTP-backed init orchestration | Usable foundation | Full initialize/configure-device wrappers that order status clear, PMSC, OTP-backed calibration, PHY, MAC, STS, CIA, GPIO, AES, AON, RX defaults, and deferred IDLE_PLL entry.                          |
| PLL/RF/TX_CAL                 | Usable foundation | Dedicated register-file HALs for RF path forcing, TX test modes, LDO/SAR test access, PLL CFG/CC/CAL/XTAL access, SAR readings, PGC control/status, and pulse-generator calibration target/test access. |
| External sync                 | Usable foundation | EC_CTRL OSTR configuration, documented wait validation, PLL_SYNC convenience helper, top-level disabled-by-default init step, and RX_CAL bridge wrappers for the shared register file.                  |
| ESP-IDF port adapter          | Usable foundation | Allocated dw3000_port_t wrapper with ESP-IDF SPI polling transactions, command-phase DW3000 headers, active-low reset GPIO, IRQ GPIO read, delay, microsecond timestamp, and recursive mutex callbacks. |

## Hardware Bring-Up Test Apps

| App                         | Status              | Coverage                                                                                              |
| --------------------------- | ------------------- | ----------------------------------------------------------------------------------------------------- |
| `hal_basic_info`            | Hardware pass       | ESP-IDF port setup plus informative DEV_ID, SYS_STATUS, SYS_CFG, SYS_TIME, SYS_STATE, EUI, and PANADR reads. |
| `hal_default_init`          | Hardware pass       | Default HAL initialization and readbacks across status, MAC, STS, CIA, GPIO, AES, PMSC, and selected raw registers. |
| `hal_aes_engine`            | Hardware pass       | AES key RAM, scratch RAM, AES-GCM encrypt/decrypt, AES status, and AES_DONE event generation.          |
| `hal_gpio_irq`              | Hardware pass       | Active IRQ GPIO validation using AES_DONE as a deterministic interrupt source.                                  |
| `hal_rx_basic`              | Build pass          | RX arm, RXFCG/error polling, RX_FINFO, RX buffer read, RX timestamp, and frame-pattern validation; needs two-node hardware log confirmation. |
| `hal_tx_basic`              | Build pass          | TX buffer/frame control, immediate TX start, TXFRS polling, TX timestamp, and repeated known-frame transmission; needs two-node hardware log confirmation. |

## Remaining

| Area                         | Needed next                                   | Why it matters                                                                 |
| ---------------------------- | --------------------------------------------- | ------------------------------------------------------------------------------ |
| RX/TX hardware proof         | Flash/run `hal_rx_basic` plus `hal_tx_basic` and confirm pass logs. | Proves the frame path, timestamps, immediate TX/RX, and interrupt/status clearing. |
| ACC/CIR hardware proof       | Read accumulator/CIR data after a real received frame. | ACC/CIR reads are only meaningful after CIADONE on an actual receive path.      |
| Repeatable host/unit tests   | Add focused mocks under a future test folder. | Hardware tests prove the board; host tests catch regressions without hardware.  |

## Current Priority

1. Run `hal_rx_basic` plus `hal_tx_basic` on two boards.
2. Add ACC/CIR inspection after RX is proven.
