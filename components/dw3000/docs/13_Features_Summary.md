# DW3000 Features Summary

This document summarizes the DW3000 feature set so we can discuss the component structure and implementation scope more clearly.

The DW3000 is not just a basic UWB PHY. It combines a UWB transceiver, timestamping engine, secure ranging support, selected MAC offload, hardware AES, calibration hooks, and several system-integration features that matter for real RTLS products.

## 1. At a Glance

At a high level, the DW3000 provides:

- IEEE 802.15.4 HRP UWB with IEEE 802.15.4z BPRF support
- Channels 5 and 9
- Data rates of 850 kb/s and 6.8 Mb/s
- Precise TX and RX timestamping for ranging and location
- STS-based secure timestamping
- Support for ToF, TDoA, and optionally PDoA
- Host control over SPI with IRQ-driven status handling
- MAC assists such as FCS, frame filtering, auto-ACK, and wait-for-response
- Hardware AES for CCM* and GCM operations
- Low-power states, delayed TX/RX scheduling, and SNIFF mode
- OTP-backed calibration and configuration storage
- External sync support for tightly synchronized anchor deployments

## 2. Core Radio and PHY Features

### UWB PHY operation

The DW3000 is a single-chip UWB transceiver implementing the HRP UWB PHY. It supports both classic IEEE 802.15.4-style packets and IEEE 802.15.4z secure ranging packet formats.

Key PHY capabilities:

- UWB channel 5 and channel 9 operation
- Configurable preamble lengths and preamble codes
- 16 MHz and 64 MHz PRF operation
- Standard and STS-enabled packet formats
- Standard frames up to 127 bytes
- Extended frames up to 1023 bytes in non-standard extended PHR mode

Implication for the library:

- PHY configuration should be treated as a first-class subsystem, not a few loose fields
- Frame mode, preamble, channel, PRF, SFD, and PHR mode all need explicit configuration types

### Packet formats

The DW3000 supports four packet structures:

- `SP0`: standard UWB packet without STS
- `SP1`: STS between SFD and PHR
- `SP2`: STS after payload
- `SP3`: STS-only ranging packet

These are important because they affect security, compatibility, timing, and library behavior.

## 3. Timestamping and Ranging Features

### High-resolution timestamps

The most important DW3000 capability is precise timestamping. TX and RX timestamps are generated in hardware with about 15.65 ps resolution and are adjusted using antenna delay calibration.

What this enables:

- Two-way ranging
- Time-of-flight distance measurement
- TDoA positioning
- Scheduled TX/RX operations
- Time-synchronized system behavior

### Delayed transmission and delayed reception

The DW3000 can schedule TX and RX at precisely defined future times using:

- absolute delayed time
- TX timestamp-relative delay
- RX timestamp-relative delay
- reference time plus offset

This is a major feature, not just a convenience. It is central to:

- TWR response timing
- TDMA-style scheduling
- low-power receive windows
- deterministic ranging exchanges

Implication for the library:

- delayed operations deserve clear APIs, validation, and timestamp helper utilities
- lateness and `HPDWARN` handling should be visible in the API design

### Location scheme support

The DW3000 is suitable for three major location approaches:

- `ToF`: two-way ranging between peers
- `TDoA`: synchronized anchors comparing receive timestamps
- `PDoA`: bearing estimation on PDoA-capable variants

PDoA is only available on the PDoA variants of the family, so device capability detection matters.

## 4. Secure Ranging Features

### STS: secure timestamping

One of the DW3000's defining features is STS, the Scrambled Timestamp Sequence. It protects timestamp-based ranging against preamble replay and early-detect style attacks.

STS characteristics:

- AES-128-based pseudo-random sequence generation
- shared key and IV/nonce model
- secure CIR accumulation for timestamp extraction
- separate quality indicators for deciding whether a secure timestamp is trustworthy

This is one of the main reasons to choose DW3000 over DW1000 in secure ranging systems.

### STS modes and tradeoffs

- `SP1` is efficient and early in the packet, but requires correct seed alignment
- `SP2` is more interoperability-friendly, but has extra security caveats unless payload design is careful
- `SP3` is ideal for pure ranging exchanges with no payload

Implication for the library:

- STS configuration should not be buried inside generic PHY config
- keys, IV/counter handling, STS length, mode, and quality validation should have dedicated abstractions

### SDC mode

The DW3000 also supports SDC, a non-secure but collision-tolerant alternative to AES-based STS for high-density systems where security is not required.

## 5. Receive-Side Features

The RX path is one of the richer parts of the DW3000.

It includes:

- preamble detect with configurable PAC
- SFD timeout and preamble timeout handling
- CIR accumulation
- CIA-based first-path detection
- RX timestamp refinement
- STS CIR support
- frame integrity reporting

Additional RX features:

- delayed RX
- double-buffered RX
- SNIFF mode for low-power listening
- diagnostics and quality indicators

### Double buffering

Double buffering allows the host to read one received frame while the chip receives the next one. This is important for:

- higher packet rates
- back-to-back frame handling
- reduced frame loss under host latency

### SNIFF mode

SNIFF mode cycles RX on and off to reduce average power during preamble detection. This matters for battery-powered nodes that spend long periods listening.

Implication for the library:

- RX is not just `start_rx()` and `read_frame()`
- the library will eventually need explicit support for timeouts, buffer selection, diagnostics, and low-power RX modes

## 6. MAC Hardware Features

The DW3000 does not implement a full MAC, but it offloads several useful MAC tasks.

### Hardware-supported MAC features

- automatic FCS generation on TX
- automatic FCS checking on RX
- frame filtering by type, PAN ID, short address, and EUI-64
- automatic immediate ACK generation
- TX followed by automatic wait-for-response RX
- pseudo clear channel assessment

This is enough to reduce host overhead significantly while still leaving full MAC policy to host software.

### Auto-ACK and wait-for-response

These are especially important for:

- IEEE 802.15.4 data exchanges
- ranging handshakes
- low-latency turnaround
- simpler host-side state machines

Implication for the library:

- MAC control should likely become its own subsystem, not just a few bits inside init config
- ACK timing and W4R timing deserve dedicated APIs or config structures

## 7. Security and AES Features

The DW3000 has a hardware AES-DMA engine that supports:

- AES CCM*
- AES GCM
- 128-bit, 192-bit, and 256-bit keys
- encryption and decryption
- operation on TX, RX, and scratch buffers

This matters for more than payload encryption. It also supports secure STS key handling workflows, including loading encrypted STS material without exposing plaintext on the SPI bus.

Implication for the library:

- AES support is large enough to justify a dedicated API area later
- security support should be treated separately from STS, even though the features are related

## 8. Host Interface and Control Features

### SPI interface

The DW3000 is controlled over SPI and supports:

- normal register reads and writes
- addressed sub-register access
- masked writes
- single-octet fast commands
- optional SPI CRC mode

### IRQ and status model

The device exposes many status bits and event enables through `SYS_STATUS` and `SYS_ENABLE`, with write-1-to-clear behavior for many events.

This means the component will eventually need a clear interrupt model covering:

- TX complete
- RX complete
- RX errors and timeouts
- auto-ACK / wait-for-response transitions
- brownout and SPI readiness
- CIA completion and diagnostics readiness

### Fast commands

Fast commands are a notable part of the DW3000 programming model. They provide one-octet control for:

- immediate TX and RX
- delayed TX and RX
- TX with wait-for-response
- pseudo CCA
- clearing IRQs
- double-buffer toggling

Implication for the library:

- fast commands should be represented explicitly, not hidden entirely behind generic register writes

## 9. Power and System-State Features

The DW3000 has a meaningful state machine:

- `INIT_RC`
- `IDLE_RC`
- `IDLE_PLL`
- `TX_WAIT`
- `TX`
- `RX_WAIT`
- `RX`
- `SLEEP`
- `DEEPSLEEP`

Important power-management features:

- low-power sleep and deep sleep
- AON retention of configuration
- wake by pin, SPI chip select, or timer depending on mode
- optional automatic calibrations or measurements on wake

Implication for the library:

- sleep/wake cannot stay as a trivial afterthought if we want full device coverage
- wake sequencing, retained config, and post-wake recalibration should be modeled explicitly

## 10. Calibration and Device Tuning Features

The DW3000 depends heavily on calibration for best performance.

Main calibration areas:

- crystal trim
- TX power and spectral shaping
- antenna delay
- PLL recalibration over temperature, especially on channel 9
- RX tuning values loaded from OTP

These are essential for:

- ranging accuracy
- regulatory compliance
- link quality
- temperature robustness

Implication for the library:

- calibration should be treated as a real subsystem, not a collection of magic constants

## 11. Manufacturing and Device-Specific Features

### OTP memory

The DW3000 includes OTP memory used for:

- factory calibration values
- EUI programming
- AES key storage
- XTAL trim
- customer manufacturing data

This makes OTP important both for manufacturing workflows and for runtime bring-up.

### On-chip measurements and brownout

The chip can measure:

- internal temperature
- supply voltage

It also provides brownout detection. These are useful for:

- health monitoring
- environment-aware recalibration
- fault handling in battery-powered systems

## 12. Synchronization and Infrastructure Features

### External synchronization

The DW3000 can synchronize its internal timebase to an external clock and sync pulse. This is especially valuable in wired-anchor TDoA systems.

What it enables:

- deterministic timebase reset
- tightly aligned anchor clocks
- externally synchronized timestamping
- deterministic TX relative to external sync

This is a high-value advanced feature for RTLS infrastructure, even if it is not part of the first implementation milestone.

### External PA support

The device also supports controlling an external power amplifier and RF path switching through GPIOs in supported designs.

## 13. What This Means for the Library

The DW3000 library should be thought of as several subsystems rather than one flat API:

- core device and state control
- register and fast-command transport
- PHY configuration
- TX/RX operation and timestamping
- MAC assists
- STS and secure ranging
- AES/security services
- power management
- calibration and OTP
- diagnostics and measurement
- sync and infrastructure features

If we try to place all of this into only `dw3000_api.h` plus one generic type header, the design will become crowded very quickly.

## 14. Suggested Implementation Priorities

A practical implementation order would be:

1. Core bring-up, reset, state transitions, register I/O, and fast commands
2. Basic PHY config, TX, RX, timestamps, delayed TX/RX
3. MAC assists: filtering, auto-ACK, wait-for-response
4. Diagnostics, double buffering, and RX quality reporting
5. STS configuration and secure timestamp validation
6. Calibration, OTP, sleep/wake, and sensor functions
7. AES engine workflows, external sync, and advanced infrastructure support

## 15. Bottom Line

The DW3000 is best understood as a secure ranging and location transceiver platform, not merely a UWB packet radio.

Its standout capabilities are:

- precise timestamping
- secure STS-based ranging
- deterministic delayed TX/RX
- MAC assists for efficient exchanges
- calibration and OTP support for real products
- infrastructure features for TDoA and advanced RTLS deployments

That breadth is the main reason the component should be structured into focused headers and subsystems.
