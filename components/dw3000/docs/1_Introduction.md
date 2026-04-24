# Chapter 1 — Introduction

> **Source:** DW3000 Family User Manual, Version 1.1 (© Decawave Ltd 2019, revised 28 May 2021)
> **Document:** "How to Use, Configure and Control the DW3000 UWB Transceiver"

---

## 1.1 About the DW3000

The DW3000 is a family of **fully integrated, low-power, single-chip CMOS radio transceiver ICs** implementing HRP UWB PHY as specified by the **IEEE 802.15.4** standard [1], including the BPRF mode specified by the **IEEE 802.15.4z** amendment [2].

There are two device variants distinguished by their device identifier:
- **Non-PDoA** variant: device ID `0xDECA0302`
- **PDoA** variant: device ID `0xDECA0312`

### Key Features

- Supports UWB channels **5** (6489.6 MHz) and **9** (7987.2 MHz)
- Supports **2-way ranging**, **TDoA**, and optionally **PDoA** location schemes
- Low external component count
- Supports enhanced **Time-of-Flight security modes** using STS
- Integrated **AES CCM\*** and **AES GCM 128/192/256** functionality
- Worldwide UWB Radio Regulatory compliance
- Low power consumption — suitable for **coin cell battery** powered applications
- Data rates of **850 kb/s** and **6.8 Mb/s**
- Packet length from **zero to 1023 octets**
- Integrated MAC support features
- Up to **38 MHz SPI interface** to host MCU
- Provides **precision location and data transfer simultaneously**
- Asset location accuracy of **~10 cm**
- High **multipath fading immunity**
- Supports **high tag densities** in RTLS
- Package options: **QFN40** (5 mm × 5 mm) and **WLCSP52** (3.1 mm × 3.5 mm)

### Device Variants

| IC Variant | Package  | PDoA Support | Operating Temperature |
| ---------- | -------- | ------------ | --------------------- |
| DW3110     | WLCSP52  | No           | −40°C to +85°C        |
| DW3120     | WLCSP52  | Yes          | −40°C to +85°C        |
| DW3210     | QFN40    | No           | −40°C to +85°C        |
| DW3220     | QFN40    | Yes          | −40°C to +85°C        |

---

## 1.2 About This Document

This user manual describes the **operation and programming** of the DW3000, and discusses design choices to consider when implementing systems using it. Information in the DW3000 Datasheet [5] is not reproduced here — this manual should be used alongside the datasheet.

### Document Sections

| Section | Title | Content |
| ------- | ----- | ------- |
| **2** | Overview of the DW3000 | Overview, interfacing via SPI, operating modes and states |
| **3** | Message transmission | Transmitter functionality, packet formats, timestamps, delayed TX |
| **4** | Message reception | Receiver functionality, RX sequences, buffering, SNIFF mode |
| **5** | Media Access Control (MAC) hardware features | MAC-level functionality provided by the IC |
| **6** | Secure ranging / timestamping | Secure timestamping capability (STS) |
| **7** | Other features of the IC | External sync, PA, OTP, temperature, brownout |
| **8** | The DW3000 register set | Complete register map with all user-accessible bit fields |
| **9** | Fast Commands | Single-octet SPI commands to place the device into TX or RX |
| **10** | Calibration | Parameters requiring calibration, methodology, and frequency |
| **11** | Location schemes | Design trade-offs for building systems based on the DW3000 |
| **12** | APPENDIX 1: Two-way ranging | Introduction to TWR proximity system implementations |
| **13** | APPENDIX 2: Abbreviations and acronyms | Full glossary |
| **14** | APPENDIX 3: References | Standards and document references |
| **15** | Document History | Revision history |

### Driver Software

Decawave also provides **DW3000 device driver software** as source code [3]. This includes:
- API functions to initialise, configure, and control the DW3000
- API functions for transmission, reception, and IC feature control
- Targeted for **ARM Cortex-M** but readily portable to other microprocessor systems (including ESP32)
- Simple examples including a two-way ranging demonstration

---

## 1.3 References

| Ref | Document |
| --- | -------- |
| [1] | IEEE Std 802.15.4™-2020 — IEEE Standard for Low-Rate Wireless Networks |
| [2] | IEEE Std 802.15.4z™-2020 — Amendment 1: Enhanced UWB PHYs and Associated Ranging Techniques |
| [3] | DW3000 APIs and Simple Examples — source code at www.decawave.com |
| [4] | IEEE Std 802.15.8™-2017 — IEEE Standard for Wireless MAC and PHY for Peer Aware Communications |
| [5] | DW3000 Datasheet |
| [6] | APS312 — Production tests for DW3000 based products (application note, www.decawave.com) |

---

## 1.4 Abbreviations and Acronyms

| Abbreviation | Full Title | Description |
| ------------ | ---------- | ----------- |
| **ACK** | Acknowledgement (frame) | A frame sent in response to a received frame indicating successful reception. DW3000 can auto-generate these. |
| **AES** | Advanced Encryption Standard | Symmetric block cipher used to encrypt data. Implemented in a dedicated hardware block in the DW3000. |
| **AGC** | Automatic Gain Control | Automatically adjusts the gain of the receiver depending on the received signal power. |
| **AON** | Always-On Block | Section of memory in DW3000 whose contents are retained during SLEEP and DEEPSLEEP states. Used to save and restore device configuration. |
| **BPM** | Burst Position Modulation | Modulation scheme where information is conveyed by the position of a burst of pulses within a symbol. |
| **BPSK** | Binary Phase-Shift Keying | Modulation scheme where information is conveyed by whether pulses are positive or negative. |
| **CIA** | Channel Impulse Analyser | Algorithm that processes the CIR (in the accumulator) to find the leading edge during RX timestamp estimation. May be applied to CIR from preamble and/or STS sequences. |
| **CIR** | Channel Impulse Response | The impulse response of the communications channel between transmitter and receiver as detected by DW3000. Also called CIRE (estimate) in some contexts. |
| **CRC** | Cyclic Redundancy Check | Error detecting code appended to the frame (same as FCS). |
| **DPS** | Dynamic Preamble Select | Anti-spoofing mechanism allowing devices to change preamble codes during ranging (IEEE 802.15.4 feature). |
| **ESD** | Electrostatic Discharge | Sudden flow of electrical current between electrically charged objects. DW3000 is resistant to ESD to the limits specified in the Datasheet. |
| **EUI** | Extended Unique Identifier | 64-bit IEEE device address. |
| **FAST_RC** | Fast RC Oscillator | Fast RC oscillator running at approx. 120 MHz. |
| **FCS** | Frame Check Sequence | CRC appended to the frame in the transmitter to allow error detection at the receiver. |
| **IF** | Intermediate Frequency | A frequency to which a carrier frequency is shifted as an intermediate step in transmission or reception. |
| **LCSS** | Low Cross-Correlation Sum Set | Block in the DW3000 receiver that modifies the STS correlation function to reduce side-lobes and produce a cleaner CIR accumulation of the STS. |
| **LDC** | Low Duty-Cycle | Regulatory rules that limit the duration of UWB transmissions per unit time in certain channels. |
| **LDO** | Low Drop-Out Voltage Regulator | Linear voltage regulator. DW3000 uses a number of internal LDOs. |
| **LNA** | Low Noise Amplifier | Front-end circuit that amplifies very low-level received signals while minimising added noise. |
| **LOS** | Line of Sight | Radio channel configuration with a direct line of sight between transmitter and receiver. |
| **NLOS** | Non Line of Sight | Radio channel configuration without a direct line of sight between transmitter and receiver. |
| **OTP** | One-Time Programmable (Memory) | Internal memory in DW3000 that can be programmed once to store identification and calibration values. |
| **PAC** | Preamble Acquisition Chunk | A group of preamble symbols correlated together in the preamble detection process. Configurable size — see DTUNE0. |
| **PDoA** | Phase Difference of Arrival | The difference in phase of a signal received at a pair of antennas, providing bearing information of the transmitter. |
| **PHR** | PHY Header | Part of the packet that comes before the PHY payload. |
| **PHY** | Physical Layer | The lowest layer in the OSI model; defines the physical interface to the communications medium. |
| **PLL** | Phase Locked Loop | Used in DW3000 to generate stable carrier frequencies and system clocks. |
| **PRF** | Pulse Repetition Frequency | The frequency at which pulses are repeated in the preamble and data portions of a packet, depending on chosen configuration. |
| **PSR** | Preamble Symbol Repetitions | Defines the overall preamble length. A larger number gives a longer preamble. |
| **RF** | Radio Frequency | Signals in the range of 3 kHz to 300 GHz. |
| **RMARKER** | Ranging Marker | The time when the peak pulse location associated with the first chip period following the SFD is at the local antenna. |
| **RSSI** | Received Signal Strength Indication | Measured or estimated receive power level. |
| **RTLS** | Real Time Location Systems | System intended to provide information on the location of various items in real-time. |
| **RX** | Receive / Receiver | The receiver section of a transceiver or the operation of receiving signals. |
| **SAR** | Successive Approximation Register ADC | Type of ADC using a digital binary search to converge on the correct digital representation of the analog input level. |
| **SECDED** | Single Error Correct, Double Error Detect | A parity check sequence (used in the PHR) allowing correction of a single-bit error and detection (not correction) of a double-bit error. |
| **SFD** | Start of Frame Delimiter | Marks the completion of the preamble section of the packet and the start of the payload or STS section. (IEEE 802.15.4 defined.) |
| **SHR** | Synchronisation Header | Consists of the preamble and the SFD. (IEEE 802.15.4 context.) |
| **SPI** | Serial Peripheral Interface | Industry-standard synchronous serial interface between ICs. |
| **SP0–SP3** | STS Packet Configuration 0–3 | Packet configuration variants defining STS position within the frame. SP3: preamble + SFD + STS only. |
| **STS** | Scrambled Timestamp Sequence | Sequence of pseudo-randomised pulses generated using a deterministic random bit generator (DRBG), included in UWB packets for secure ToA. |
| **TDoA** | Time Difference of Arrival | Location method using the difference in packet arrival times at physically different, synchronised locations to determine transmitter position. |
| **ToA** | Time of Arrival | The receive time of a packet, generally referenced to the RMARKER. Also called the receive timestamp. |
| **ToF** | Time of Flight | The time taken for a radio signal to travel between the transmitting and receiving antenna. |
| **TX** | Transmit / Transmitter | The transmitter section of a transceiver or the operation of transmitting signals. |
| **UWB** | Ultra Wide-Band | A radio scheme employing channel bandwidths of, or in excess of, 500 MHz. |
| **WSN** | Wireless Sensor Networks | A network of wireless nodes intended to enable monitoring and control of the physical environment. |
