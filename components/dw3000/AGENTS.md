# DW3000 Component Agent Guide

Use this guide when working anywhere under `components/dw3000/`.

## Documentation Search Order

1. Start with the markdown documents in `components/dw3000/docs/*.md`.
2. Pick the most relevant chapter from the topic index below.
3. Only if the markdown chapter does not answer the question fully, consult the full manual PDF:
   `components/dw3000/docs/DW3000 User Manual.pdf`

The markdown files are chapter-oriented extracts/summaries of the DW3000 user manual and should be the default source for implementation, register, timing, RX/TX flow, STS, calibration, and MAC behavior questions.

## Topic Routing

- Device capabilities, variants, document scope: `docs/1_Introduction.md`
- Bring-up, SPI, interrupts, GPIO, operating states, sleep/wake, channels: `docs/2_Overview.md`
- TX flow, delayed TX, TX timestamps, TX power: `docs/3_Message_Transmission.md`
- RX flow, timestamps, delayed RX, double buffering, SNIFF, diagnostics: `docs/4_Message_Reception.md`
- MAC behavior, frame filtering, auto-ACK, wait-for-response, AES usage: `docs/5_MAC_Hardware_Features.md`
- Secure ranging, STS, SDC, key/nonce handling, STS quality: `docs/6_Secure_Ranging_Timestamping.md`
- External sync, OTP, temperature/voltage measurement, brownout, PA support: `docs/7_Other_Features.md`
- Register definitions:
  `docs/8_Register_Set_1.md`,
  `docs/8_Register_Set_2.md`,
  `docs/8_Register_Set_3.md`,
  `docs/8_Register_Set_4.md`
- Single-octet fast commands and command behavior: `docs/9_Fast_Commands.md`
- Crystal trim, TX power/spectrum, antenna delay, PLL recalibration: `docs/10_Calibration.md`
- System-level location scheme context: `docs/11_Location_Schemes.md`
- Two-way ranging appendix, acronyms, references: `docs/12_Appendices.md`

## Chapter Index

### `docs/1_Introduction.md`

Introduces the DW3000 family, key features, device variants, and the structure of the manual. Use this for high-level orientation before digging into implementation details.

### `docs/2_Overview.md`

Covers platform bring-up and core operating model: SPI transactions, interrupt/GPIO behavior, SYNC pin usage, device states, clocks, data rate, preamble/channel basics, reset, sleep/deepsleep, AON retention, and default configuration flow.

### `docs/3_Message_Transmission.md`

Explains how transmission works end-to-end: packet formats, TX setup sequence, important TX registers, TX timestamp semantics, delayed transmission, extended frames, TX-related interrupts, and transmit power configuration.

### `docs/4_Message_Reception.md`

Explains the receive pipeline: preamble/SFD/STS/PHR/data reception, RX timestamps, delayed RX, TDoA/PDoA support, double-buffer operation, low-power SNIFF mode, diagnostics, CIR/CIA data, and RX signal quality estimation.

### `docs/5_MAC_Hardware_Features.md`

Describes the DW3000 MAC-side hardware features: frame structure, CRC, frame filtering, automatic acknowledgement, automatic wait-for-response flows, pseudo-CCA, and AES-backed confidentiality/authenticity support for TX and RX.

### `docs/6_Secure_Ranging_Timestamping.md`

Focuses on secure timestamping and ranging: why STS exists, packet mode placement, STS key/IV/counter handling, CIA processing of STS CIR, STS quality metrics, and SDC support.

### `docs/7_Other_Features.md`

Collects supporting features that do not fit the main TX/RX chapters: external synchronization modes, external PA usage, OTP memory layout/programming/reading, on-chip temperature and voltage measurement, and brownout detection.

### `docs/8_Register_Set_1.md`

Starts the register reference. Use this first for core system registers such as device ID, addressing, system configuration/control/status, TX/RX timing registers, channel control, double-buffer status, and AES configuration/key material.

### `docs/8_Register_Set_2.md`

Continues the register reference with STS configuration/status, RX tuning, external sync and RX calibration, GPIO control, digital receiver configuration, RF analog controls, transmitter calibration, PLL, and crystal trim registers.

### `docs/8_Register_Set_3.md`

Continues the register reference with AON control, OTP memory interface, and the CIA register block for timestamps, TDoA/PDoA outputs, and preamble/STS diagnostic registers.

### `docs/8_Register_Set_4.md`

Finishes the register reference with digital diagnostics/event counters, system state, fast-command status, PMSC control, SNIFF/LED/bias registers, RX/TX buffers, accumulator memory, scratch RAM, AES key RAM, and indirect pointer interfaces.

### `docs/9_Fast_Commands.md`

Documents each fast command opcode and when to use it, including immediate and delayed TX/RX, wait-for-response flows, clear-IRQ behavior, and double-buffer toggling.

### `docs/10_Calibration.md`

Explains what must be calibrated for good performance: crystal trim, transmit power and spectrum tuning, antenna delay, and channel-9 PLL recalibration behavior over temperature.

### `docs/11_Location_Schemes.md`

Provides system-level RTLS context, comparing ToF, TDoA, and PDoA approaches, including tradeoffs around anchor synchronization, message count, power, density, and location engine assumptions.

### `docs/12_Appendices.md`

Contains reference material such as single-sided and double-sided two-way ranging schemes, plus abbreviations, references, and document history.

## When To Use The PDF

Consult `docs/DW3000 User Manual.pdf` only when:

- the relevant markdown chapter is missing detail
- you need a figure, table, or wording not preserved in the markdown
- you need to cross-check behavior spanning multiple chapters
- you suspect the markdown omitted a caveat, exception, or register note

When using the PDF, prefer reading only the relevant chapter or page range first, then return to code changes with a short note about what was confirmed there.
