# 8 Register Set (Part 4)

> **Source:** DW3000 Family User Manual, Version 1.1 (© Decawave Ltd 2019, revised 28 May 2021)
> **Pages:** 205-237

---

### 8.2.14 Register file: 0x0F – Digital diagnostics interface

| ID   | Length (octets) | Type | Mnemonic | Description                   |
| ---- | --------------- | ---- | -------- | ----------------------------- |
| 0x0F | 79              | -    | DIG_DIAG | Digital diagnostics interface |

Register file 0x0F is the Digital Diagnostics interface. It contains a number of Sub-registers that give diagnostics information. An overview of these is given by Table 40. Each of these Sub-registers is separately described in the sub-sections below.

**Table 40: Register file: 0x0F – Digital diagnostics interface overview**

| OFFSET in Register 0x0F | Mnemonic   | Description                                    |
| ----------------------- | ---------- | ---------------------------------------------- |
| 0x00                    | EVC_CTRL   | Event counter control                          |
| 0x04                    | EVC_PHE    | PHR error counter                              |
| 0x06                    | EVC_RSE    | RSD error counter                              |
| 0x08                    | EVC_FCG    | Frame check sequence good counter              |
| 0x0A                    | EVC_FCE    | Frame check sequence error counter             |
| 0x0C                    | EVC_FFR    | Frame filter rejection counter                 |
| 0x0E                    | EVC_OVR    | RX overrun error counter                       |
| 0x10                    | EVC_STO    | SFD timeout counter                            |
| 0x12                    | EVC_PTO    | Preamble timeout counter                       |
| 0x14                    | EVC_FWTO   | RX frame wait timeout counter                  |
| 0x16                    | EVC_TXFS   | TX frame sent counter                          |
| 0x18                    | EVC_HPW    | Half period warning counter                    |
| 0x1A                    | EVC_SWCE   | SPI write CRC error counter                    |
| 0x1C                    | EVC_RES1   | Digital diagnostics reserved area 1            |
| 0x24                    | DIAG_TMC   | Test mode control register                     |
| 0x28                    | EVC_CPQE   | STS quality error counter                      |
| 0x2A                    | EVC_VWARN  | Low voltage warning error counter              |
| 0x2C                    | SPI_MODE   | SPI mode                                       |
| 0x30                    | SYS_STATE  | System state                                   |
| 0x3C                    | FCMD_STAT  | Fast command status                            |
| 0x48                    | CTR_DBG    | Current value of the low 32-bits of the STS IV |
| 0x4C                    | SPICRCINIT | SPI CRC LFSR initialisation code               |

---

#### 8.2.14.1 Sub-register 0x0F:00 – Event counter control

| ID    | Length (octets) | Type | Mnemonic | Description           |
| ----- | --------------- | ---- | -------- | --------------------- |
| 0F:00 | 1               | SRW  | EVC_CTRL | Event counter control |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x00 is the event counter control register.

**REG:0F:00 – EVC_CTRL – Event counter control**

| Bits | Field   | Default | Description           |
| ---- | ------- | ------- | --------------------- |
| 0    | EVC_EN  | 0       | Event Counters Enable |
| 1    | EVC_CLR | 0       | Event Counters Clear  |
| 7:2  | -       | 0       | Reserved              |

Fields in the EVC_CTRL register are intended to be self-clearing. So, the event counters can be enabled or cleared, but cannot be disabled. The bits of the EVC_CTRL register identified above are individually described below:

| Field                         | Description of fields within Sub-register 0x0F:00 – Event counter control                                                                                                                                                                                                                                                                                                                                                                                     |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_EN<br>reg:0F:00<br>bit:0  | Event Counters Enable. The EVC_EN bit acts to enable the event counters. When EVC_EN bit is zero none of the event counters will update. When EVC_EN bit is set to 1 it enables event counting. A number of Sub-registers of Register file: 0x0F – Digital diagnostics interface, contain counters of various system events. If the host system has no interest in these event counters then a small amount of power is saved by not enabling event counting. |
| EVC_CLR<br>reg:0F:00<br>bit:1 | Event Counters Clear. The EVC_CLR bit acts to clear event counters to zero. This cannot be done while EVC_EN bit is set. The correct procedure to clear the event counters is to write 0x02 to EVC_CTRL to disable counting and clear the counter values to zero, and then to write 0x01 to EVC_CTRL to re-enable counting if required.                                                                                                                       |
| -<br>reg:0F:00<br>bits:7–2    | The remaining bits of EVC_CTRL are reserved and should always be set to zero to avoid any malfunction of the device.                                                                                                                                                                                                                                                                                                                                          |

---

#### 8.2.14.2 Sub-register 0x0F:04 – PHR error counter

| ID    | Length (octets) | Type | Mnemonic | Description             |
| ----- | --------------- | ---- | -------- | ----------------------- |
| 0F:04 | 2               | RO   | EVC_PHE  | PHR error event counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x04 is the PHY Header Error event counter.

**REG:0F:04 – EVC_PHE – PHR Error Counter**

| Bits  | Field   | Default | Description                  |
| ----- | ------- | ------- | ---------------------------- |
| 11:0  | EVC_PHE | 0       | 12-bit PHR error event count |
| 15:12 | -       | 0       | Reserved                     |

| Field                             | Description of fields within Sub-register 0x0F:04 – PHR error counter                                                                                                                                                                                                                          |
| --------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_PHE<br>reg:0F:04<br>bits:11–0 | PHR Error Event Counter. The EVC_PHE field is a 12-bit counter of PHY Header Errors. This counts the reporting of RXPHE error events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                   | The remaining bits of this register are reserved.                                                                                                                                                                                                                                              |

---

#### 8.2.14.3 Sub-register 0x0F:06 – RSD error counter

| ID    | Length (octets) | Type | Mnemonic | Description             |
| ----- | --------------- | ---- | -------- | ----------------------- |
| 0F:06 | 2               | RO   | EVC_RSE  | RSD error event counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x06 is the RSD error event counter.

**REG:0F:06 – EVC_RSE – RSD error counter**

| Bits  | Field   | Default | Description                  |
| ----- | ------- | ------- | ---------------------------- |
| 11:0  | EVC_RSE | 0       | 12-bit RSD error event count |
| 15:12 | -       | 0       | Reserved                     |

| Field                             | Description of fields within Sub-register 0x0F:06 – RSD error counter                                                                                                                                                                                                                                                                                                                  |
| --------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_RSE<br>reg:0F:06<br>bits:11–0 | Reed Solomon decoder (Sync Loss) Error Event Counter. The EVC_RSE field is a 12-bit counter of the non-correctable error events that can occur during Reed Solomon decoding. This counts the reporting of RXFSL error events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                   | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                                                                                      |

---

#### 8.2.14.4 Sub-register 0x0F:08 – FCS good counter

| ID    | Length (octets) | Type | Mnemonic | Description                             |
| ----- | --------------- | ---- | -------- | --------------------------------------- |
| 0F:08 | 2               | RO   | EVC_FCG  | Frame Check Sequence good event counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x08 is the FCS good event counter.

**REG:0F:08 – EVC_FCG – Frame Check Sequence good event counter**

| Bits  | Field   | Default | Description                 |
| ----- | ------- | ------- | --------------------------- |
| 11:0  | EVC_FCG | 0       | 12-bit FCS good event count |
| 15:12 | -       | 0       | Reserved                    |

| Field                             | Description of fields within Sub-register 0x0F:08 – FCS good counter                                                                                                                                                                                                                                                                  |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_FCG<br>reg:0F:08<br>bits:11–0 | Frame Check Sequence Good Event Counter. The EVC_FCG field is a 12-bit counter of the frames received with good CRC/FCS sequence. This counts the reporting of RXFCG events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                   | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                                     |

---

#### 8.2.14.5 Sub-register 0x0F:0A – FCS error counter

| ID    | Length (octets) | Type | Mnemonic | Description                        |
| ----- | --------------- | ---- | -------- | ---------------------------------- |
| 0F:0A | 2               | RO   | EVC_FCE  | Frame Check Sequence error counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x0A is the FCS error event counter.

**REG:0F:0A – EVC_FCE – FCS error counter**

| Bits  | Field   | Default | Description                  |
| ----- | ------- | ------- | ---------------------------- |
| 11:0  | EVC_FCE | 0       | 12-bit FCS error event count |
| 15:12 | -       | 0       | Reserved                     |

| Field                             | Description of fields within Sub-register 0x0F:0A – FCS error counter                                                                                                                                                                                                                                                                 |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_FCE<br>reg:0F:0A<br>bits:11–0 | Frame Check Sequence Error Event Counter. The EVC_FCE field is a 12-bit counter of the frames received with bad CRC/FCS sequence. This counts the reporting of RXFCE events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                   | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                                     |

---

#### 8.2.14.6 Sub-register 0x0F:0C – Frame filter rejection counter

| ID    | Length (octets) | Type | Mnemonic | Description                    |
| ----- | --------------- | ---- | -------- | ------------------------------ |
| 0F:0C | 1               | RO   | EVC_FFR  | Frame filter rejection counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x0C is the frame filter rejection counter.

**REG:0F:0C – EVC_FFR – Frame filter rejection counter**

| Bits | Field   | Default | Description                        |
| ---- | ------- | ------- | ---------------------------------- |
| 7:0  | EVC_FFR | 0       | 8-bit frame filter rejection count |

| Field                            | Description of fields within Sub-register 0x0F:0C – Frame filter rejection counter                                                                                                                                                                                                                                                                                |
| -------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_FFR<br>reg:0F:0C<br>bits:7–0 | Frame Filter Rejection Event Counter. The EVC_FFR field is an 8-bit counter of the frames rejected by the receive frame filtering function. This is essentially a count of the reporting of ARFE events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |

---

#### 8.2.14.7 Sub-register 0x0F:0E – RX overrun error counter

| ID    | Length (octets) | Type | Mnemonic | Description              |
| ----- | --------------- | ---- | -------- | ------------------------ |
| 0F:0E | 1               | RO   | EVC_OVR  | RX overrun error counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x0E is the RX overrun error counter.

**REG:0F:0E – EVC_OVR – RX overrun error counter**

| Bits | Field   | Default | Description                  |
| ---- | ------- | ------- | ---------------------------- |
| 7:0  | EVC_OVR | 0       | 8-bit RX overrun error count |

| Field                            | Description of fields within Sub-register 0x0F:0E – RX overrun error counter                                                                                                                                                                                                                                                                                                                                                                |
| -------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_OVR<br>reg:0F:0E<br>bits:7–0 | RX Overrun Error Event Counter. The EVC_OVR field is a 12-bit counter of receive overrun events. This is essentially a count of the reporting of RXOVRR events in Sub-register 0x00:44 – System event status. The EVC_OVR will be incremented once for each RX frame discarded that happens while an overrun condition persists. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |

---

#### 8.2.14.8 Sub-register 0x0F:10 – SFD timeout error counter

| ID    | Length (octets) | Type | Mnemonic | Description               |
| ----- | --------------- | ---- | -------- | ------------------------- |
| 0F:10 | 2               | RO   | EVC_STO  | SFD timeout error counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x10 is the SFD timeout error counter.

**REG:0F:10 – EVC_STO – SFD Timeout Error Counter**

| Bits  | Field   | Default | Description                    |
| ----- | ------- | ------- | ------------------------------ |
| 11:0  | EVC_STO | 0       | 12-bit SFD timeout error count |
| 15:12 | -       | 0       | Reserved                       |

| Field                             | Description of fields within Sub-register 0x0F:10 – SFD timeout error counter                                                                                                                                                                                                                                               |
| --------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_STO<br>reg:0F:10<br>bits:11–0 | SFD timeout errors Event Counter. The EVC_STO field is a 12-bit counter of SFD Timeout Error events. This is essentially a count of the reporting of RXSTO events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                   | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                           |

---

#### 8.2.14.9 Sub-register 0x0F:12 – Preamble detection timeout event counter

| ID    | Length (octets) | Type | Mnemonic | Description                              |
| ----- | --------------- | ---- | -------- | ---------------------------------------- |
| 0F:12 | 2               | RO   | EVC_PTO  | Preamble detection timeout event counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x12 is the preamble timeout event counter.

**REG:0F:12 – EVC_PTO – Preamble detection timeout event counter**

| Bits  | Field   | Default | Description                         |
| ----- | ------- | ------- | ----------------------------------- |
| 11:0  | EVC_PTO | 0       | 12-bit preamble timeout event count |
| 15:12 | -       | 0       | Reserved                            |

| Field                             | Description of fields within Sub-register 0x0F:12 – Preamble detection timeout event counter                                                                                                                                                                                                                                                 |
| --------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_PTO<br>reg:0F:12<br>bits:11–0 | Preamble Detection Timeout Event Counter. The EVC_PTO field is a 12-bit counter of Preamble detection Timeout events. This is essentially a count of the reporting of RXPTO events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                   | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                                            |

---

#### 8.2.14.10 Sub-register 0x0F:14 – RX frame wait timeout event counter

| ID    | Length (octets) | Type | Mnemonic | Description                   |
| ----- | --------------- | ---- | -------- | ----------------------------- |
| 0F:14 | 1               | RO   | EVC_FWTO | RX frame wait timeout counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x14 is the RX frame wait timeout event counter.

**REG:0F:14 – EVC_FWTO – RX frame wait timeout event counter**

| Bits | Field    | Default | Description                       |
| ---- | -------- | ------- | --------------------------------- |
| 7:0  | EVC_FWTO | 0       | 8-bit RX frame wait timeout count |

| Field                             | Description of fields within Sub-register 0x0F:14 – RX frame wait timeout event counter                                                                                                                                                                                                                                                      |
| --------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_FWTO<br>reg:0F:14<br>bits:7–0 | RX Frame Wait Timeout Event Counter. The EVC_FWTO field is an 8-bit counter of receive frame wait timeout events. This is essentially a count of the reporting of the RXFTO events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |

---

#### 8.2.14.11 Sub-register 0x0F:16 – TX frame sent counter

| ID    | Length (octets) | Type | Mnemonic | Description           |
| ----- | --------------- | ---- | -------- | --------------------- |
| 0F:16 | 2               | RO   | EVC_TXFS | TX frame sent counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x16 is the TX frame sent counter.

**REG:0F:16 – EVC_TXFS – TX frame sent counter**

| Bits  | Field    | Default | Description                |
| ----- | -------- | ------- | -------------------------- |
| 11:0  | EVC_TXFS | 0       | 12-bit TX frame sent count |
| 15:12 | -        | 0       | Reserved                   |

| Field                              | Description of fields within Sub-register 0x0F:16 – TX frame sent counter                                                                                                                                                                                                                                                                                             |
| ---------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_TXFS<br>reg:0F:16<br>bits:11–0 | TX Frame Sent Event Counter. The EVC_TXFS field is a 12-bit counter of transmit frames sent. This is incremented every time a frame is sent. It is essentially a count of the reporting of the TXFRS events in Sub-register 0x00:44 – System event status. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |
| -<br>bits:15–12                    | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                                                                     |

---

#### 8.2.14.12 Sub-register 0x0F:18 – Half period warning counter

| ID    | Length (octets) | Type | Mnemonic | Description                 |
| ----- | --------------- | ---- | -------- | --------------------------- |
| 0F:18 | 1               | RO   | EVC_HPW  | Half period warning counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x18 is the half period warning counter.

**REG:0F:18 – EVC_HPW – Half period warning counter**

| Bits | Field   | Default | Description                     |
| ---- | ------- | ------- | ------------------------------- |
| 7:0  | EVC_HPW | 0       | 8-bit half period warning count |

| Field                            | Description of fields within Sub-register 0x0F:18 – Half period warning counter                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| -------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_HPW<br>reg:0F:18<br>bits:7–0 | Half Period Warning Event Counter. The EVC_HPW field is an 8-bit counter of "Half Period Warnings". This is a count of the reporting of the HPDWARN events in Sub-register 0x00:44 – System event status. These relate to late invocation of delayed transmission or reception functionality. Please refer to the description of the HPDWARN bit for more details of this event and its meaning. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in Sub-register 0x0F:00 – Event counter control. |

---

#### 8.2.14.13 Sub-register 0x0F:1A – SPI write CRC error counter

| ID    | Length (octets) | Type | Mnemonic | Description                 |
| ----- | --------------- | ---- | -------- | --------------------------- |
| 0F:1A | 1               | RO   | EVC_SWCE | SPI write CRC error counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x1A is the SPI write CRC error counter.

**REG:0F:1A – EVC_SWCE – SPI write CRC error**

| Bits | Field    | Default | Description                     |
| ---- | -------- | ------- | ------------------------------- |
| 7:0  | EVC_SWCE | 0       | 8-bit SPI write CRC error count |

| Field                             | Description of fields within Sub-register 0x0F:1A – SPI write CRC error counter                                                                                                                                                                                                                                                                                                                                                  |
| --------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_SWCE<br>reg:0F:1A<br>bits:7–0 | SPI Write CRC Error Counter. The EVC_SWCE field is an 8-bit counter of "SPI Write CRC Error" events. This is a count of the reporting of the SPICRCE events in Sub-register 0x00:44 – System event status. These events only arise when SPI CRC mode is enabled. See section 2.3.1.3 – SPI CRC mode for more details. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |

---

#### 8.2.14.14 Sub-register 0x0F:1C – EVC reserved area 1

| ID    | Length (octets) | Type | Mnemonic | Description                         |
| ----- | --------------- | ---- | -------- | ----------------------------------- |
| 0F:1C | 8               | RO   | EVC_RES1 | Digital diagnostics reserved area 1 |

Register file: 0x0F – Digital diagnostics interface, Sub-register 0x1C is a reserved register.

---

#### 8.2.14.15 Sub-register 0x0F:24 – Digital diagnostics test mode control

| ID    | Length (octets) | Type | Mnemonic | Description                |
| ----- | --------------- | ---- | -------- | -------------------------- |
| 0F:24 | 4               | RW   | DIAG_TMC | Test mode control register |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x24 is the test mode control register.

**REG:0F:24 – DIAG_TMC – Digital diagnostics test mode control**

| Bits  | Field    | Default | Description                       |
| ----- | -------- | ------- | --------------------------------- |
| 3:0   | -        | 0       | Reserved                          |
| 4     | TX_PSTM  | 0       | Transmit Power Spectrum Test Mode |
| 20:5  | -        | 0       | Reserved                          |
| 21    | HIRQ_POL | 0       | Host interrupt polarity           |
| 23:22 | -        | 0       | Reserved                          |
| 24    | CIA_WDEN | 0       | Enable the CIA watchdog           |
| 25    | -        | 0       | Reserved                          |
| 26    | CIA_RUN  | 0       | Run the CIA manually              |
| 31:27 | -        | 0       | Reserved                          |

| Field                           | Description of fields within Sub-register 0x0F:24 – Digital diagnostics test mode control                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| -<br>reg:0F:24<br>bit: various  | These bits of the DIAG_TMC register are reserved and should always be set to zero to avoid any malfunction of the device.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| TX_PSTM<br>reg:2F:24<br>bit:4   | Transmit Power Spectrum Test Mode. This test mode is provided to help support regulatory approvals spectral testing. When the TX_PSTM bit is set it enables a repeating transmission of the data from the TX_BUFFER. To use this test mode, the operating channel, preamble code, data length, offset, etc. should all be set-up as if for a normal transmission. The start-to-start delay between packets is programmed in the DX_TIME register. This is a special use of that register, and the value is programmed in periods of one half of the 499.2 MHz fundamental frequency (~4 ns). To send one packet per millisecond, a value of 249600 or 0x0003CF00 should be programmed into the DX_TIME register. The minimum valid value is to be programmed to DX_TIME register is 2. A time value less than the packet's duration will cause packets to be sent back-to-back. When the mode, the delay and TX buffer have been configured and the TX_PSTM bit is set, the repeated TX mode is initiated by issuing a TX start command, CMD_TX. |
| HIRQ_POL<br>reg:0F:24<br>bit:21 | Host interrupt polarity. This bit allows the system integrator the ability to control the polarity of the IRQ line from the DW3000. When HIRQ_POL is 1 the IRQ output line from the DW3000 is active high, and, when HIRQ_POL is 0 the IRQ output line from the DW3000 is active low. Active high operation is recommended for low power applications so that the interrupt is in its 0 V logical inactive state when the DW3000 is in **SLEEP** or **DEEPSLEEP** states.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| CIA_WDEN<br>reg:0F:24<br>bit:24 | Enable the CIA watchdog. When this configuration is 1 (the default) an internal watchdog timer is started whenever the CIA begins the processing a received CIR, for either the preamble or STS sequences. The watchdog time is fixed at 120 µs. If the CIA completes before the watchdog timer elapses the watchdog timer is stopped, otherwise the CIAERR event status flag is asserted and the CIA is stopped. This avoids any possibility of run-away processing in the CIA block. If CIA_WDEN is set to 0, the CIA watchdog will be disabled. This might be tried if the CIAERR events occur to see if good RX timestamp results can be achieved if the CIA was given more time. If the CIA watchdog is disabled the host should include its own watchdog timeout to recover in the event that the CIA takes too long, i.e. the CIADONE event status flag is not arriving.                                                                                                                                                                  |
| CIA_RUN<br>reg:0F:24<br>bit:26  | Run the CIA manually. Normally this control bit will not be required because by default the CIARUNE configuration in Sub-register 0x11:08 – Sequencing control is set to 1 which causes the CIA to be run automatically when a packet is received. If CIARUNE is 0, then the CIA_RUN bit may be used to run the CIA after the packet is received. The CIA_IPATOV and CIA_STS bits in Sub-register 0x00:10 – System configuration should be set to select which CIA analysis is required. The CIA_RUN bit will automatically clear when it is acted upon. NB: To run the CIA manually, after a receive event when the receiver is off, the receive clock will need to be forced on using the RX_CLK control in Sub-register 0x11:04 – Clock control.                                                                                                                                                                                                                                                                                              |

---

#### 8.2.14.16 Sub-register 0x0F:28 – STS quality error counter

| ID    | Length (octets) | Type | Mnemonic | Description               |
| ----- | --------------- | ---- | -------- | ------------------------- |
| 0F:28 | 1               | RO   | EVC_CPQE | STS quality error counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x28 is the STS quality error counter.

**REG:0F:28 – EVC_CPQE – STS quality error counter**

| Bits | Field    | Default | Description                   |
| ---- | -------- | ------- | ----------------------------- |
| 7:0  | EVC_CPQE | 0       | 8-bit STS quality error count |

| Field                             | Description of fields within Sub-register 0x0F:28 – STS quality error counter                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| --------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_CPQE<br>reg:0F:28<br>bits:7–0 | STS Quality Error Counter. The EVC_CPQE field is an 8-bit counter of receive packets for which the STS quality assessment measurements are below the threshold. This is a count of the reporting of received packets with STS where the CIA algorithm has detected a quality problem and set one of the error flags of the STS_TOAST field within Sub-register 0x0C:08 – STS receive time stamp and status. The CPERR count may increment by 2 per frame (e.g. in PDOA mode 3 where STS analysis is done separately on two halves of the STS). NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |

---

#### 8.2.14.17 Sub-register 0x0F:2A – Low voltage warning error counter

| ID    | Length (octets) | Type | Mnemonic  | Description                       |
| ----- | --------------- | ---- | --------- | --------------------------------- |
| 0F:2A | 1               | RO   | EVC_VWARN | Low voltage warning error counter |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x2A is the low voltage warning error counter.

**REG:0F:2A – EVC_VWARN – Low voltage warning error counter**

| Bits | Field     | Default | Description                           |
| ---- | --------- | ------- | ------------------------------------- |
| 7:0  | EVC_VWARN | 0       | 8-bit low voltage warning event count |

| Field                              | Description of fields within Sub-register 0x0F:2A – Low voltage warning error counter                                                                                                                                                                                                                                                                                                                                                                                                     |
| ---------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| EVC_VWARN<br>reg:0F:2A<br>bits:7–0 | Low voltage warning Error Counter. The EVC_VWARN field is an 8-bit counter of brown-out warnings. This is a count of the occurrence of low voltage warnings, as reported through the VWARN events status flag in Sub-register 0x00:44 – System event status. This counts individual brown-out events detected even when the VWARN event flag is not being cleared by the host. NB: For this counter to be active, counting needs to be enabled by the setting the EVC_EN bit in EVC_CTRL. |

---

#### 8.2.14.18 Sub-register 0x0F:2C – SPI mode configuration

| ID    | Length (octets) | Type | Mnemonic | Description |
| ----- | --------------- | ---- | -------- | ----------- |
| 0F:2C | 1               | RO   | SPI_MODE | SPI mode    |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x2C is the SPI mode configuration.

**REG:0F:2C – SPI_MODE – SPI mode configuration**

| Bits | Field    | Default | Description                       |
| ---- | -------- | ------- | --------------------------------- |
| 1:0  | SPI_MODE | 0       | SPI clock phase and polarity bits |
| 7:2  | -        | 0       | Reserved                          |

| Field                             | Description of fields within Sub-register 0x0F:2C – SPI mode configuration                                                                                  |
| --------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SPI_MODE<br>reg:0F:2A<br>bits:1–0 | SPI Mode. These two bits can be used to read the SPI mode of the device: SPI Clock Phase and Polarity. Bit [0] = SPI CLK polarity. Bit [1] = SPI CLK phase. |
| -<br>bits:7–2                     | The remaining bits are reserved.                                                                                                                            |

---

#### 8.2.14.19 Sub-register 0x0F:30 – System state

| ID    | Length (octets) | Type | Mnemonic  | Description   |
| ----- | --------------- | ---- | --------- | ------------- |
| 0F:30 | 4               | RO   | SYS_STATE | System states |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x30 is the system states diagnostic register.

**REG:0F:30 – SYS_STATE – System state**

| Bits  | Field      | Default | Description                           |
| ----- | ---------- | ------- | ------------------------------------- |
| 3:0   | TX_STATE   | 0       | Current Transmit State Machine value  |
| 7:4   | -          | 0       | Reserved                              |
| 11:8  | RX_STATE   | 0       | Current Receive State Machine value   |
| 15:12 | -          | 0       | Reserved                              |
| 18:16 | TSE_STATE  | 0       | (reserved / not documented in detail) |
| 23:16 | PMSC_STATE | 0       | Current PMSC State Machine value      |
| 31:24 | -          | 0       | Reserved                              |

| Field                                 | Description of fields within Sub-register 0x0F:30 – System state                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| ------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| TX_STATE<br>reg:0F:30<br>bit: 3-0     | Current Transmit State Machine value:<br>0x0 - IDLE: Transmitter is IDLE<br>0x1 - PREAMBLE: Transmitting preamble<br>0x2 - SFD: Transmitting SFD<br>0x3 - PHR: Transmitting PHY Header data<br>0x4 - SDE: Transmitting PHR parity SECDED bits<br>0x5 - DATA: Transmitting data block (330 symbols)                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| -<br>reg:0F:30<br>bit: 7-4            | Reserved                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| RX_STATE<br>reg:0F:30<br>bit: 11-8    | Current Receive State Machine value:<br>0x00 - IDLE: Receiver is in idle<br>0x01 - START_ANALOG: Start analog receiver blocks<br>0x04 - RX_RDY: Receiver ready<br>0x05 - PREAMBLE_FND: Receiver is waiting to detect preamble<br>0x06 - PRMBL_TIMEOUT: Preamble timeout<br>0x07 - SFD_FND: SFD found<br>0x08 - CNFG_PHR_RX: Configure for PHR reception<br>0x09 - PHR_RX_STRT: PHR reception started<br>0x0A - DATA_RATE_RDY: Ready for data reception<br>0x0C - DATA_RX_SEQ: Data reception<br>0x0D - CNFG_DATA_RX: Configure for data<br>0x0E - PHR_NOT_OK: PHR error<br>0x0F - LAST_SYMBOL: Received last symbol<br>0x10 - WAIT_RSD_DONE: Wait for Reed Solomon decoder to finish<br>0x11 - RSD_OK: Reed Solomon correct<br>0x12 - RSD_NOT_OK: Reed Solomon error |
| -<br>reg:0F:30<br>bit: 15-12          | Reserved                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| PMSC_STATE<br>reg:0F:30<br>bit: 23-16 | Current PMSC State Machine value:<br>0x0 – WAKEUP: DW3000 is in WAKEUP<br>0x1, 0x2 – IDLE_RC: DW3000 is in IDLE_RC<br>0x3 – IDLE: DW3000 is in IDLE<br>0x8 – 0xF – TX: DW3000 is in TX state<br>0x12 – 0x19 – RX: DW3000 is in RX state                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| -<br>reg:0F:30<br>bit: 31-24          | Reserved                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |

---

#### 8.2.14.20 Sub-register 0x0F:3C – FCMD status

| ID    | Length (octets) | Type | Mnemonic  | Description         |
| ----- | --------------- | ---- | --------- | ------------------- |
| 0F:3C | 1               | RO   | FCMD_STAT | Fast command status |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x3C is the fast command status.

**REG:0F:3C – FCMD_STAT – Fast command status register**

| Bits | Field     | Default | Description                                   |
| ---- | --------- | ------- | --------------------------------------------- |
| 4:0  | FCMD_STAT | 0       | Value of the currently executing fast command |
| 7:5  | -         | 0       | Reserved                                      |

| Field                              | Description of fields within Sub-register 0x0F:3C – FCMD status                                                                        |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| FCMD_STAT<br>reg:0F:2A<br>bits:4–0 | Fast command status. It stores the value of the currently executing fast command. For more information on fast commands see section 9. |
| -<br>bits:7–5                      | The remaining bits of this register are reserved.                                                                                      |

---

#### 8.2.14.21 Sub-register 0x0F:48 – Counter debug

| ID    | Length (octets) | Type | Mnemonic | Description                                    |
| ----- | --------------- | ---- | -------- | ---------------------------------------------- |
| 0F:48 | 4               | RO   | CTR_DBG  | Current value of the low 32-bits of the STS IV |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x48 is the counter debug register. It contains the current value of the low 32-bits of STS IV.

---

#### 8.2.14.22 Sub-register 0x0F:4C – SPI CRC Initialisation

| ID    | Length (octets) | Type | Mnemonic   | Description                      |
| ----- | --------------- | ---- | ---------- | -------------------------------- |
| 0F:4C | 1               | RO   | SPICRCINIT | SPI CRC LFSR initialisation code |

Register file: 0x0F – Digital diagnostics interface, sub-register 0x4C is the register that contains SPI CRC LFSR initialisation code for the SPI CRC function. The value here is used to initialise the SPI CRC calculation shift register at the start of each SPI transaction, when SPI CRC mode is enabled via the SPI_CRCEN bit in Sub-register 0x00:10 – System configuration. For more details of SPI CRC operation please refer to § 2.3.1.3 – SPI CRC mode, and § 2.3.1.3.3 for details of the CRC generation polynomial and shift register implementation.

**REG:00:18 – SPICRCINIT – SPI CRC initialisation code register**

| Bits | Field      | Default | Description                      |
| ---- | ---------- | ------- | -------------------------------- |
| 7:0  | SPICRCINIT | 0       | SPI CRC LFSR initialisation code |
| 31:8 | -          | 0       | Reserved                         |

| Field                               | Description                                                |
| ----------------------------------- | ---------------------------------------------------------- |
| SPICRCINIT<br>reg:00:18<br>bits:7–0 | SPI CRC LFSR initialisation code for the SPI CRC function. |
| -<br>bits:31–0                      | The remaining bits of this register are reserved.          |

---

### 8.2.15 Register file: 0x11 – PMSC control and status

| ID   | Length (octets) | Type | Mnemonic  | Description                              |
| ---- | --------------- | ---- | --------- | ---------------------------------------- |
| 0x11 | 34              | -    | PMSC_CTRL | Power management, timing and seq control |

Register file 0x11 is concerned with the use of the PMSC. It contains a number of Sub-registers. An overview of these is given by Table 41. Each of these Sub-registers is separately described in the sub-sections below.

**Table 41: Register file: 0x11 – PMSC control and status overview**

| OFFSET in Register 0x11 | Mnemonic  | Description                           |
| ----------------------- | --------- | ------------------------------------- |
| 00                      | SOFT_RST  | Soft reset of the device blocks       |
| 04                      | CLK_CTRL  | PMSC clock control register           |
| 08                      | SEQ_CTRL  | PMSC control register 1               |
| 12                      | TXFSEQ    | PMSC fine grain TX sequencing control |
| 16                      | LED_CTRL  | LED control register                  |
| 1A                      | RX_SNIFF  | Sniff mode configuration              |
| 1F                      | BIAS_CTRL | Analog blocks' calibration values     |

---

#### 8.2.15.1 Sub-register 0x11:00 – Soft reset

| ID    | Length (octets) | Type | Mnemonic | Description                     |
| ----- | --------------- | ---- | -------- | ------------------------------- |
| 11:00 | 2               | RW   | SOFT_RST | Soft reset of the device blocks |

Register file: 0x11 – PMSC control and status, Sub-register 0x00 is the soft reset command register. This register allows a software applied reset to be applied to the IC.

**REG:11:00 – SOFT_RST – Soft reset of the device blocks**

| Bits | Field     | Default | Description                            |
| ---- | --------- | ------- | -------------------------------------- |
| 8:0  | SOFTRESET | 0x1FF   | Nine bits that reset various IC blocks |
| 15:9 | -         | 0       | Reserved                               |

The SOFTRESET bits map as follows (bit 8 down to bit 0): GPIO_RST, PMSC_RST, HIF_RST, TX_RST, RX_RST, BIST_RST, CIA_RST, PRGN_RST, ARM_RST.

| Field                              | Description of fields within Sub-register 0x11:00 – Soft reset                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SOFTRESET<br>reg:11:00<br>bits:8–0 | These nine bits reset various the IC blocks, e.g. GPIO, TX, RX, Host Interface and the PMSC itself, essentially allowing a reset of the IC under software control. These bits should be cleared to zero to force a reset and then returned to one for normal operation. The correct procedure to achieve this reset is to: (a) Set SYS_CLK to 01; (b) Clear SOFTRESET to all zero's; (c) Set SOFTRESET to all ones. The AON block is not reset by this activity and so may take action following the reset depending on the configuration within Sub-register 0x0A:00 – AON on wake configuration. |
| -<br>bits:15–9                     | The remaining bits of this register are reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |

---

#### 8.2.15.2 Sub-register 0x11:04 – Clock control

| ID    | Length (octets) | Type | Mnemonic | Description                 |
| ----- | --------------- | ---- | -------- | --------------------------- |
| 11:04 | 4               | RW   | CLK_CTRL | PMSC clock control register |

Register file: 0x11 – PMSC control and status, Sub-register 0x00 is a 32-bit control register relating to enabling clocking to various digital blocks within the DW3000.

**REG:11:04 – CLK_CTRL – PMSC clock control register**

| Bits  | Field        | Default | Description                              |
| ----- | ------------ | ------- | ---------------------------------------- |
| 1:0   | SYS_CLK      | 0       | System Clock Selection field             |
| 3:2   | RX_CLK       | 0       | Receiver Clock Selection                 |
| 5:4   | TX_CLK       | 0       | Transmitter Clock Selection              |
| 6     | ACC_CLK_EN   | 0       | Force Accumulator Clock Enable           |
| 7     | -            | 0       | Reserved                                 |
| 8     | CIA_CLK_EN   | 0       | Force CIA Clock Enable                   |
| 9     | -            | 0       | Reserved                                 |
| 10    | SAR_CLK_EN   | 0       | Analog-to-Digital Converter Clock Enable |
| 14:11 | -            | 0       | Reserved                                 |
| 15    | ACC_MCLK_EN  | 0       | Accumulator Memory Clock Enable          |
| 16    | GPIO_CLK_EN  | 0       | GPIO clock Enable                        |
| 17    | -            | 0       | Reserved                                 |
| 18    | GPIO_DCLK_EN | 0       | GPIO De-bounce Clock Enable              |
| 19    | GPIO_DRST_N  | 0       | GPIO de-bounce reset (NOT), active low   |
| 22:20 | -            | 0       | Reserved                                 |
| 23    | LP_CLK_EN    | 0       | Kilohertz clock Enable                   |
| 31:24 | -            | 0       | Reserved (default 0xF0)                  |

| Field                               | Description of fields within Sub-register 0x11:04 – Clock control                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| -                                   | Bits marked '-' are reserved and should be preserved at their reset value.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| SYS_CLK<br>reg:11:04<br>bits:1,0    | System Clock Selection field. The SYS_CLK field selects the source of clock for DW3000 system clock. Allowed values are: 00: Auto – The system clock will run off the FAST_RC/4 clock until (assuming AINIT2IDLE is set to 1) the AON transfer has completed, it will then switch to FAST_RC(120MHz) clock until the PLL is calibrated and locked, and then it will switch to the 125 MHz PLL clock. 01: Force system clock to be the FAST_RC/4 clock. 10: Force system clock to the 125 MHz PLL clock. (If this clock is not present the IC will essentially lock up with further SPI communications impossible. In this case an external reset will be needed to recover.) 11: Force system clock to FAST_RC. This control is used for certain procedures, e.g. before a soft reset (SOFTRESET).                       |
| RX_CLK<br>reg:11:04<br>bits:3,2     | Receiver Clock Selection. This selects the source of clock for the DW3000 receiver. Allowed values are: 00: Auto – The RX clock will be disabled until it is required for an RX operation, at which time it will be enabled to use the 125 MHz PLL clock. 01, 10, 11: Force RX clock enable and sourced from the 125 MHz PLL clock. (NB: ensure PLL clock is present).                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| TX_CLK<br>reg:11:04<br>bits:5,4     | Transmitter Clock Selection. This selects the source of clock for the DW3000 transmitter. Allowed values are: 00: Auto – The TX clock will be disabled until it is required for a TX operation, at which time it will be enabled to use the 125 MHz PLL clock. 01, 10, 11: Force TX clock enable and sourced from the 125 MHz PLL clock. (NB: ensure PLL clock is present). This control is used for certain procedures, e.g. when setting up the continuous transmission mode that is used during power output calibration and regulatory testing.                                                                                                                                                                                                                                                                      |
| ACC_CLK_EN<br>reg:11:04<br>bit:6    | Force Accumulator Clock Enable. In normal operation this bit should be set to 0 to allow the PMSC to control the accumulator clock as necessary for normal receiver operation. If the host system wants to read the accumulator data, both this ACC_CLK_EN bit and the ACC_MCLK_EN bit (below) need to be set to 1 to allow the accumulator reading to operate correctly.                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| CIA_CLK_EN<br>reg:11:04<br>bit:8    | Force CIA Clock Enable. In normal operation this bit should be set to 0 to allow the PMSC to control the CIA clock as necessary for normal CIA operation. If the host system wants to run CIA manually using CIA_RUN then this bit should be set to 1.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| SAR_CLK_EN<br>reg:11:00<br>bit:10   | (temperature and voltage) Analog-to-Digital Convertor Clock Enable. The DW3000 is equipped with 8-bit A/D convertors to sample the IC temperature and its input battery voltage. The IC can automatically sample the temperature and voltage as it wakes up from **SLEEP** or **DEEPSLEEP**. This is controlled by the ONW_RUN_SAR bit in Sub-register 0x0A:00 – AON on wake configuration. If the host system wants to initiate temperature and/or voltage measurements at other times then the clock to the Analog-to-Digital Convertor needs to be enabled via this SAR_CLK_EN bit. For more details of this functionality, please refer to section 7.4 – Measuring IC temperature and voltage.                                                                                                                       |
| ACC_MCLK_EN<br>reg:11:04<br>bit:15  | Accumulator Memory Clock Enable. In normal operation this bit should be set to 0 to allow the PMSC to control the accumulator memory clock as necessary for normal receiver operation. If the host system wants to read the accumulator data, both this ACC_MCLK_EN bit and the ACC_CLK_EN bit (above) need to be set to 1 to allow the accumulator reading to operate correctly.                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| GPIO_CLK_EN<br>reg:11:04<br>bit:16  | GPIO clock Enable. In order to use the GPIO port lines the GPIO_CLK_EN enable must be set to 1 to enable the clock into the GPIO block. The bit must also be set to 1 to take the GPIO port out of its reset state.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| GPIO_DCLK_EN<br>reg:11:04<br>bit:18 | GPIO De-bounce Clock Enable. The DW3000 GPIO port includes a de-bounce functionality that may be applied to input lines being used as an interrupt source. The de-bounce filter circuit clocks the GPIO inputs into the DW3000 and removes short transients by requiring that the input persists for two cycles of this clock before it will be seen by the interrupt handling logic. In order to use the GPIO port de-bounce functionality this GPIO_DCLK_EN bit must be set to 1 to enable the clock into the GPIO block. The GPIO_DRST_N bit (below) must also be set to 1 to take the GPIO port de-bounce filter circuit out of its reset state. This GPIO_DCLK_EN bit also serves to enable the clock that controls the LED blink functionality and so must be enabled in order for the LEDs to function correctly. |
| GPIO_DRST_N<br>reg:11:04<br>bit:19  | GPIO de-bounce reset (NOT), active low. In order to use the GPIO port de-bounce filter circuit the GPIO_DRST_N bit must be set to 1 to take the de-bounce filter circuit out of its reset state. The GPIO_DCLK_EN enable bit (above) must also be set to 1 to enable the clock into the GPIO de-bounce circuit.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| LP_CLK_EN<br>reg:11:00<br>bit:23    | Kilohertz clock Enable. When this bit is set to 1 it enables the divider. The divider value is set by LP_CLK_DIV in Sub-register 0x11:08 – Sequencing control.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |

---

#### 8.2.15.3 Sub-register 0x11:08 – Sequencing control

| ID    | Length (octets) | Type | Mnemonic | Description                      |
| ----- | --------------- | ---- | -------- | -------------------------------- |
| 11:08 | 4               | RW   | SEQ_CTRL | PMSC sequencing control register |

Register file: 0x11 – PMSC control and status, Sub-register 0x08 is a 32-bit control register. The SEQ_CTRL register contains the following sub-fields:

**REG:11:08 – SEQ_CTRL – Sequencing control register**

| Bits  | Field      | Default | Description                              |
| ----- | ---------- | ------- | ---------------------------------------- |
| 7:0   | -          | 0       | Reserved                                 |
| 8     | AINIT2IDLE | 1       | Automatic IDLE_RC to IDLE_PLL transition |
| 10:9  | -          | 0       | Reserved                                 |
| 11    | ATX2SLP    | 0       | After TX automatically Sleep             |
| 12    | ARX2SLP    | 0       | After RX automatically Sleep             |
| 14:13 | -          | 0       | Reserved                                 |
| 15    | PLL_SYNC   | 1       | 1 GHz clock for external SYNC modes      |
| 16    | -          | 0       | Reserved                                 |
| 17    | CIARUNE    | 1       | CIA run enable                           |
| 22:18 | -          | 0       | Reserved                                 |
| 23    | FORCE2INIT | 0       | Force to IDLE_RC state                   |
| 25:24 | -          | 0       | Reserved                                 |
| 31:26 | LP_CLK_DIV | 1       | Kilohertz clock divisor                  |

| Field                                 | Description of fields within Sub-register 0x11:08 – Sequencing control                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| ------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -                                     | Bits marked '-' are reserved and should be preserved at their reset value.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| AINIT2IDLE<br>reg:11:08<br>bit:8      | Automatic **IDLE_RC** to **IDLE_PLL**. The AINIT2IDLE bit is responsible for transitioning the IC from **IDLE_RC** state to **IDLE_PLL** state. If AINIT2IDLE is set to 1 before entering SLEEP then upon wake up the IC will automatically transition into **IDLE_PLL** state as soon as the clock PLL has locked. By default AINIT2IDLE is clear which means that after a reset, (or when coming out of SLEEP), the IC will stay in the **IDLE_RC** state until AINIT2IDLE is set. This may facilitate lower energy use while setting up the IC before transitioning through IDLE state into TX or RX states. |
| ATX2SLP<br>reg:11:08<br>bit:11        | After TX automatically Sleep. If this bit is set then the DW3000 will automatically transition into **SLEEP** or **DEEPSLEEP** state after transmission of a packet has completed so long as there are no unmasked interrupts pending. This bit is cleared when the DW3000 wakes from sleep. Before using this ATX2SLP feature the AON configurations in Register file: 0x0A – Always-on system control interface should be set to allow for the appropriate DW3000 wake up functionality. Note: The SLEEP_EN bit in Sub-register 0x0A:14 – AON configuration has to be set to enable this functionality.       |
| ARX2SLP<br>reg:11:08<br>bit:12        | After RX automatically Sleep. If this bit is set then the DW3000 will automatically transition into **SLEEP** state after a receive attempt so long as there are no unmasked interrupts pending. Before using ARX2SLP the AON configurations in Register file: 0x0A – Always-on system control interface should be set to allow for the appropriate DW3000 wake up functionality. This bit is cleared when the DW3000 wakes from sleep. Note: SLEEP_EN bit in Sub-register 0x0A:14 – AON configuration has to be set to enable this functionality.                                                              |
| PLL_SYNC<br>reg:11:08<br>bit:15       | This enables a 1 GHz clock used for some external SYNC modes. If this is not required then to save power the PLL_SYN configuration should be left set to 0. See Register file: 0x04 – External sync control and RX calibration for more details.                                                                                                                                                                                                                                                                                                                                                                |
| CIARUNE<br>reg:11:08<br>bit:17        | CIA run enable. This bit enables the running of the CIA algorithm. CIARUNE is 1 by default which means that the CIA algorithm is run automatically. When CIARUNE is set to zero the CIA algorithm will not be run and the RX_STAMP in Sub-register 0x00:64 – Receive time stamp will not be updated, however the CIA_RUN bit may be used to run the CIA after the packet is received. The CIA_IPATOV and CIA_STS bits in Sub-register 0x00:10 – System configuration should be set to select which CIA analysis is required.                                                                                    |
| FORCE2INIT<br>reg:11:08<br>bit:23     | Force to **IDLE_RC** state. When device is in **IDLE_PLL**, but host needs to change the state back to **IDLE_RC**, e.g. it is not actively ranging and needs to save power. The host needs to clear the AINIT2IDLE bit and set this FORCE2INIT bit. Prior to this the host needs to set SYS_CLK to 0x3 to force it to FAST_RC. The device will then transition into the **IDLE_RC** state. The host should finally clear this FORCE2INIT bit back to 0.                                                                                                                                                        |
| LP_CLK_DIV<br>reg:11:08<br>bits:31–26 | Kilohertz clock divisor. This field specifies a clock divider designed to give a kilohertz range clock that is used in the DW3000 for the LED blink functionality and also for the GPIO de-bounce functionality. The input to the kHz divider is the 19.2 MHz clock (which is the raw 38.4 MHz XTAL ÷ 2). The LP_CLK_DIV field specifies the top 6 bits of a 10-bit counter allowing divisors up to 2016 or clock frequencies from 9.5 kHz up to 600 kHz.                                                                                                                                                       |

---

#### 8.2.15.4 Sub-register 0x11:12 – TX fine control

| ID    | Length (octets) | Type | Mnemonic | Description                           |
| ----- | --------------- | ---- | -------- | ------------------------------------- |
| 11:12 | 4               | RW   | TXFSEQ   | PMSC fine grain TX sequencing control |

Register file: 0x11 – PMSC control and status, Sub-register 0x12 is used to control TX fine grain power sequencing function. The TXFSEQ register contains the following sub-fields:

**REG:11:12 – TXFSEQ – Fine grain TX sequencing control register**

| Bits | Field     | Default   | Description                          |
| ---- | --------- | --------- | ------------------------------------ |
| 31:0 | TXFINESEQ | 0x4d28874 | Fine grain TX power sequencing value |

| Field                               | Description of fields within Sub-register 0x11:12 – TX fine control                                                                                                                                                                                                                |
| ----------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| TXFINESEQ<br>reg:11:12<br>bits:31–0 | Writing 0x0d20874 to this field will disable TX fine grain power sequencing, this is required for certain test and calibration modes (Continuous Wave transmission). To enable fine grain power sequencing the default value of 0x4d28874 should be written back to this register. |
| -                                   | Bits marked '-' are reserved and should be preserved at their reset value.                                                                                                                                                                                                         |

---

#### 8.2.15.5 Sub-register 0x11:16 – LED control

| ID    | Length (octets) | Type | Mnemonic | Description          |
| ----- | --------------- | ---- | -------- | -------------------- |
| 11:16 | 4               | RW   | LED_CTRL | LED control register |

Register file: 0x11 – PMSC control and status, Sub-register 0x16 is a 32-bit LED control register.

**REG:11:16 – LED_CTRL – LED control register**

| Bits  | Field      | Default | Description                    |
| ----- | ---------- | ------- | ------------------------------ |
| 7:0   | BLINK_TIM  | 0x20    | Blink time count value         |
| 8     | BLINK_EN   | 1       | Blink Enable                   |
| 15:9  | -          | 0       | Reserved                       |
| 19:16 | FORCE_TRIG | 0       | Manually triggers an LED blink |
| 31:20 | -          | 0       | Reserved                       |

| Field                                 | Description of fields within Sub-register 0x11:16 – LED control                                                                                                                                                                                                                      |
| ------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| -                                     | Bits marked '-' are reserved and should be preserved at their reset value.                                                                                                                                                                                                           |
| BLINK_TIM<br>reg:11:16<br>bits:7–0    | Blink time count value. This field determines how long the LEDs remain lit after an event that causes them to be set on. This time is specified in units of 14 ms so the default value of 0x20 will give an on blink of 400 ms followed by an off blink of 400 ms.                   |
| BLINK_EN<br>reg:11:16<br>bit:8        | Blink Enable. When this bit is set to 1 the LED blink feature is enabled. Because the LED blink counter uses the low frequency KHZCLK timer, this timer must be enabled as per Sub-register 0x11:04 – Clock control and configured as per Sub-register 0x11:08 – Sequencing control. |
| FORCE_TRIG<br>reg:11:16<br>bits:19-16 | Manually triggers an LED blink. There is one trigger bit per LED IO.                                                                                                                                                                                                                 |

---

#### 8.2.15.6 Sub-register 0x11:1A – SNIFF mode

| ID      | Length (octets) | Type | Mnemonic | Description                       |
| ------- | --------------- | ---- | -------- | --------------------------------- |
| 0x11:1A | 4               | RW   | RX_SNIFF | Receiver SNIFF mode configuration |

Register file: 0x11 – PMSC control and status, Sub-register 0x1A is a 32-bit register used for configuration of SNIFF mode, which is a power saving technique that can be employed to reduce the power consumption of preamble detection. For normal preamble reception the receiver searches for preamble continually, while in SNIFF mode the receiver samples ("sniffs") the air periodically on a timed basis returning to receiver idle mode in between.

The transmitting device needs to be sending a sufficiently long preamble to allow for the SNIFF mode to operate and leave sufficient preamble remaining thereafter to get a good reception and RX timestamp. The power saving is dependent on the configured on/off times for this sampling. See additionally section 4.5 Low-Power SNIFF mode.

**REG:11:1A – RX_SNIFF – SNIFF Mode Configuration**

| Bits  | Field     | Default | Description                                        |
| ----- | --------- | ------- | -------------------------------------------------- |
| 3:0   | SNIFF_ON  | 0       | SNIFF Mode ON time (PAC units)                     |
| 7:4   | -         | 0       | Reserved                                           |
| 15:8  | SNIFF_OFF | 0       | SNIFF Mode OFF time (~1 µs units / 128 clk cycles) |
| 31:16 | -         | 0       | Reserved                                           |

| Field                               | Description of fields within Sub-register 0x11:1A – SNIFF mode                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| ----------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -                                   | Bits marked '-' are reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| SNIFF_ON<br>reg:11:1A<br>bits:3–0   | SNIFF Mode ON time. This parameter is specified in units of PAC. For details of PAC and its role in preamble, please refer to section 4.1.1. A value of zero will disable SNIFF Mode. A non-zero value will enable Preamble Detection Mode and select how long the receiver is turned on during the preamble hunt. NB: This must be a minimum of 2 for the IC to correctly make a preamble detection decision. If preamble is detected during this time window the receiver will remain on and will continue to attempt reception of the packet. If no preamble is detected the receiver will be returned to idle mode for the time configured by the SNIFF_OFF parameter before sampling the air again. |
| SNIFF_OFF<br>reg:11:1A<br>bits:15–8 | SNIFF Mode OFF time specified in µs. This parameter is specified in units of approximately 1 µs, or 128 system clock cycles. A value of zero will disable SNIFF Mode. A non-zero value will enable SNIFF Mode and select how long the receiver is turned off for during the preamble hunt. Please refer to the SNIFF_ON description above for more details of this feature.                                                                                                                                                                                                                                                                                                                              |

As an example, with a 1024 preamble length, a roughly 50% duty cycle (on 50% and off 50%) can be configured with a PAC of 8 symbols, SNIFF_ON set to 3 PAC intervals, and SNIFF_OFF set to 24 microseconds. The performance cost in terms of receiver sensitivity is < 1 dB.

---

#### 8.2.15.7 Sub-register 0x11:1F – Bias control

| ID    | Length (octets) | Type | Mnemonic  | Description                       |
| ----- | --------------- | ---- | --------- | --------------------------------- |
| 11:1F | 2               | RW   | BIAS_CTRL | Analog blocks' calibration values |

Register file: 0x11 – PMSC control and status, Sub-register 0x1F is used to configure analog blocks. It stores per device calibration values. Each device will have calibration values stored in OTP and these should be written here on power up to ensure optimal device operation.

**REG:11:1F – BIAS_CTRL – Analog blocks' calibration values**

| Bits  | Field     | Default | Description                              |
| ----- | --------- | ------- | ---------------------------------------- |
| 12:0  | BIAS_CTRL | -       | Analog blocks' calibration control value |
| 15:13 | -         | 0       | Reserved                                 |

| Field                               | Description                                                                                                                                                                                                                                                  |
| ----------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| BIAS_CTRL<br>reg:11:1f<br>bits:13–0 | This register controls analog blocks. Each device will have optimised values stored in OTP (BIASTUNE_CAL) and that value should be written to this register on power up, and also after SLEEP/DEEPSLEEP. This will ensure the device will perform optimally. |
| -                                   | Bits marked '-' are reserved and should be preserved at their reset value.                                                                                                                                                                                   |

---

### 8.2.16 Register file: 0x12 – RX frame buffer 0

| ID  | Length (octets) | Type | Mnemonic    | Description            |
| --- | --------------- | ---- | ----------- | ---------------------- |
| 12  | 1024            | ROD  | RX_BUFFER_0 | RX frame data buffer 0 |

Register file 0x12:00 is the main receive data buffer. The data from the received frame is available in the received buffer. Assuming successful reception of a good frame, the full length of received data (as reported by the RXFLEN field of REG_RX_FINFO), will be available in the RX_BUFFER_0 beginning at offset 0. Note since the reported length includes the FCS the host system will probably choose not to read these final two octets.

Write operations to the RX_BUFFER_0 are NOT supported; a write operation to the RX_BUFFER_0 will corrupt the buffer contents.

---

### 8.2.17 Register file: 0x13 – RX frame buffer 1

| ID  | Length (octets) | Type | Mnemonic    | Description            |
| --- | --------------- | ---- | ----------- | ---------------------- |
| 13  | 1024            | ROD  | RX_BUFFER_1 | RX frame data buffer 1 |

Register file 0x13:00 is the second receive data buffer. The data from the received frame is available in this received buffer, only when double buffer mode is enabled and the main buffer (RX_BUFFER_0) is full. Assuming successful reception of a good frame, the full length of received data (as reported in SET_2) by the RXFLEN field of register 0x18:E8, copy of the RX_FINFO), will be available in the RX_BUFFER_1 beginning at offset 0. Note since the reported length includes the FCS the host system will probably choose not to read these final two octets.

Write operations to the RX_BUFFER_1 are NOT supported; a write operation to the RX_BUFFER_0 will corrupt the buffer contents.

---

### 8.2.18 Register file: 0x14 – Transmit data buffer

| ID  | Length (octets) | Type | Mnemonic  | Description          |
| --- | --------------- | ---- | --------- | -------------------- |
| 14  | 1024            | WO   | TX_BUFFER | Transmit data buffer |

Register file 0x14:00 is the transmit data buffer. Data from the transmit buffer is transmitted during the data payload portion of the transmitted packet. Section 3 Message transmission discusses the basics of packet transmission and details the various parts of the TX packet.

The general procedure is to write the data frame for transmission into the TX_BUFFER, set the frame length and other details in the TX_FCTRL register and initiate transmission using in the start TX command (CMD_TX).

Note that read operations from the transmit data buffer are NOT supported.

---

### 8.2.19 Register file: 0x15 – Accumulator CIR memory

| ID  | Length (octets) | Type | Mnemonic | Description                            |
| --- | --------------- | ---- | -------- | -------------------------------------- |
| 15  | 12288           | RO   | ACC_MEM  | Read access to accumulator data memory |

Register file 0x15:00 is a large bank of memory that holds the accumulated channel impulse response (CIR) data. Really this is a channel impulse response estimate (CIRE) but the abbreviation CIR is used to mean the same thing throughout this manual. To accurately determine this timestamp the DW3000 incorporates a channel impulse analyser (CIA) algorithm to processes the CIR (in the accumulator) to find the leading edge and adjust the RMARKER receive timestamp as reported in Sub-register 0x00:64 – Receive time stamp.

The host system does not need to access the ACC_MEM in normal operation, however it may be of interest to the system design engineers to visualise the radio channel for diagnostic purposes.

The accumulator data memory actually contains one or two CIR depending on the STS packet configuration being employed. One CIR is generated by the accumulation of the repeated symbols of the preamble correlated against the expected symbol as determined by the RX preamble code configuration, PCODE in Sub-register 0x01:14 – Channel control. The other CIR is generated by the accumulation of the STS sequence correlated against the expected STS pulse pattern cryptographically generated locally in the receiver.

Accessing the accumulator CIR memory is a little different than accessing other register files in two ways: Firstly for every SPI read access of the accumulator memory, a single dummy octet is output before the first byte of valid accumulator data. Secondly the offset specified in the SPI transaction is not the byte index, but instead is the sample index, where each accumulator sample is an complex value provided as a 24-bit (3-octet) real value followed by a 24-bit (3-octet) imaginary value. Each value is actually 18-bit precision, with the upper 6-bits being all zero or ones depending on the sign of the value.

Prior to reading from the accumulator CIR memory both ACC_CLK_EN bit and the ACC_MCLK_EN bit (in Sub-register 0x11:04 – Clock control) need to be set to 1 to allow the accumulator reading to operate correctly.

The preamble CIR begins at sample index 0 and has a span of one symbol, which is 992 samples at 16 MHz PRF, and 1016 samples at 64 MHz PRF. The STS CIR begins at sample index 1024 and has a span of 512 sample times irrespective of PRF setting. When PDоA Mode 3 is used the STS is used to determine two CIR estimates. The first will begin at sample 1024 and the second at sample 1536. Both CIRs are 512 samples long.

When reading from CIR memory with an offset less than 127, a normal SPI read can be used, see 2.3.1.2 Transaction formats of the SPI interface. However to read data from CIR memory with offset greater than 127, an indirect SPI read has to be done. To perform an indirect SPI read indirect pointers need to be used: PTR_ADDR_A or PTR_ADDR_B. Firstly the register address needs to be programmed into e.g. indirect pointer A (PTR_ADDR_A) and offset into PTR_OFFSET_A and then the indirect pointer register (INDIRECT_PTR_A) needs to be read as normal to read out the required data.

Note, when reading out the CIR data, the first byte of the transaction data (see Figure 1) is a dummy byte and should be ignored.

**Table 42: Example SPI indexed read of accumulator CIR memory**

| Input Octets | Output Octets                             | Comment                            |
| ------------ | ----------------------------------------- | ---------------------------------- |
| -            | -                                         | SPI transaction header first byte  |
| -            | -                                         | SPI transaction header second byte |
| -            | \<Dummy Octet\>                           | Ignore/skip this 1st octet output  |
| -            | sample [735] real part low 8-bits         | First Complex value (6 octets)     |
| -            | sample [735] real part middle 8-bits      |                                    |
| -            | sample [735] real part high 8-bits        |                                    |
| -            | sample [735] imaginary part low 8-bits    | First Complex value (6 octets)     |
| -            | sample [735] imaginary part middle 8-bits | Second Complex value (6 octets)    |
| -            | sample [735] imaginary part high 8-bits   |                                    |
| -            | sample [736] real part low 8-bits         | Second Complex value (6 octets)    |
| -            | sample [736] real part middle 8-bits      | … etc. …                           |
| -            | sample [736] real part high 8-bits        |                                    |
| -            | sample [736] imaginary part low 8-bits    |                                    |
| -            | sample [736] imaginary part middle 8-bits |                                    |
| -            | sample [736] imaginary part high 8-bits   |                                    |
| -            | … etc. …                                  |                                    |

---

### 8.2.20 Register file: 0x16 – Scratch RAM

| ID  | Length (octets) | Type | Mnemonic    | Description               |
| --- | --------------- | ---- | ----------- | ------------------------- |
| 16  | 127             | RW   | SCRATCH_RAM | Scratch RAM memory buffer |

Register file 0x16:00 is the scratch RAM memory buffer. This memory can be used as a temporary store, or during AES/DMA operations. The data will not be preserved if the device is powered off or when it enters **SLEEP** or **DEEPSLEEP** state.

---

### 8.2.21 Register file: 0x17 – AES KEY RAM

| ID  | Length (octets) | Type | Mnemonic    | Description                                          |
| --- | --------------- | ---- | ----------- | ---------------------------------------------------- |
| 17  | -               | RW   | AES_KEY_RAM | AES KEY RAM – storage for up to 8 x 128-bit AES KEYs |

Register file 0x17 is a large bank of memory that holds up to 8 128-bit AES KEYs as shown in Table 43 below. The 32-bit KEY words need to be programmed starting with the MSB 32-bit word in the lowest register memory address.

**Table 43: Register file: 0x17 – AES KEY RAM overview**

| OFFSET in Register 0x17 | Register data                                                                                                                                                                                                                                                                                                                                             |
| ----------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 00                      | 1st AES key in RAM: For sample AES KEY: 0x112233445566778899aabbccddeeff, it should be written as follows: 0x17:00 = AES_KEY [127-96], byte[3] = 0x11 … byte[0] = 0x33; 0x17:04 = AES_KEY [95-64], byte[3] = 0x44 … byte[0] = 0x77; 0x17:08 = AES_KEY [63-32], byte[3] = 0x88 … byte[0] = 0xbb; 0x17:0C = AES_KEY [31-0], byte[3] = 0xcc … byte[0] = 0xff |
| 10                      | 2nd AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |
| 20                      | 3rd AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |
| 30                      | 4th AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |
| 40                      | 5th AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |
| 50                      | 6th AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |
| 60                      | 7th AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |
| 70                      | 8th AES key in RAM, for formatting see above                                                                                                                                                                                                                                                                                                              |

---

### 8.2.22 Register file: 0x18 – Double buffer diagnostic register set

| ID   | Length (octets) | Type | Mnemonic | Description                           |
| ---- | --------------- | ---- | -------- | ------------------------------------- |
| 0x18 | -               | RO   | DB_DIAG  | Double buffer diagnostic register set |

Register file 0x18 is a large bank of memory that holds the double buffer diagnostic register set. It contains two swinging sets corresponding to two receive buffers (SET_1 and SET_2). Each set is 232 bytes long. The first set starts at address 0x18:00, and the second at 0x18:E8, as shown in Table 44 below. The RDB_DMODE configuration specifies how much diagnostic data will be logged. The minimum configuration (i.e. when RDB_DMODE is set to 1) only logs 7 registers, the maximum logs all CIA diagnostic registers.

**Table 44: Register file: 0x18 – Double buffer diagnostic register set overview**

| OFFSET in Register 0x18 |           | RDB_DMODE | MINDIAG | Register data |
| ----------------------- | --------- | --------- | ------- | ------------- |
| **SET_1**               | **SET_2** |           |         |               |
| 00                      | E8        | 1         | 0 or 1  | RX_FINFO      |
| 04                      | EC        | 1         | 0 or 1  | RX_TIME       |
| 0C                      | F4        | 1         | 0 or 1  | CIA_DIAG_0    |
| 10                      | F8        | 1         | 0 or 1  | TDOA          |
| 14                      | FC        | 1         | 0 or 1  | PDOA          |
| 18                      | 100       | 1         | 0 or 1  | Reserved      |
| 1C                      | 104       | 1         | 0       | IP_DIAG_12    |
| 20                      | 108       | 2         | 0 or 1  | IP_TS         |
| 24                      | 10C       | 2         | 0 or 1  | Reserved      |
| 28                      | 110       | 2         | 0 or 1  | STS_TS        |
| 2C                      | 114       | 2         | 0 or 1  | Reserved      |
| 30                      | 118       | 2         | 0 or 1  | STS1_TS       |
| 34                      | 11C       | 2         | 0 or 1  | Reserved      |
| 38                      | 120       | 4         | 0 or 1  | CIA_DIAG_1    |
| 3C                      | 124       | 4         | 0       | IP_DIAG_0     |
| 40                      | 128       | 4         | 0       | IP_DIAG_1     |
| 44                      | 12C       | 4         | 0       | IP_DIAG_2     |
| 48                      | 130       | 4         | 0       | IP_DIAG_3     |
| 4C                      | 134       | 4         | 0       | IP_DIAG_4     |
| 50                      | 138       | 4         | 0       | Reserved      |
| 54                      | 13C       | 4         | 0       | Reserved      |
| 58                      | 140       | 4         | 0       | Reserved      |
| 5C                      | 144       | 4         | 0       | IP_DIAG_8     |
| 60                      | 148       | 4         | 0       | Reserved      |
| 64                      | 14C       | 4         | 0       | Reserved      |
| 68                      | 150       | 4         | 0       | Reserved      |
| 6C                      | 154       | 4         | 0       | STS_DIAG_0    |
| 70                      | 158       | 4         | 0       | STS_DIAG_1    |
| 74                      | 15C       | 4         | 0       | STS_DIAG_2    |
| 78                      | 160       | 4         | 0       | STS_DIAG_3    |
| 7C                      | 164       | 4         | 0       | STS_DIAG_4    |
| 80                      | 168       | 4         | 0       | Reserved      |
| 84                      | 16C       | 4         | 0       | Reserved      |
| 88                      | 170       | 4         | 0       | Reserved      |
| 8C                      | 174       | 4         | 0       | STS_DIAG_8    |
| 90                      | 178       | 4         | 0       | Reserved      |
| 94                      | 17C       | 4         | 0       | Reserved      |
| 98                      | 180       | 4         | 0       | Reserved      |
| 9C                      | 184       | 4         | 0       | STS_DIAG_12   |
| A0                      | 188       | 4         | 0       | Reserved      |
| A4                      | 18C       | 4         | 0       | Reserved      |
| A8                      | 190       | 4         | 0       | Reserved      |
| AC                      | 194       | 4         | 0       | Reserved      |
| B0                      | 198       | 4         | 0       | Reserved      |
| B4                      | 19C       | 4         | 0       | STS1_DIAG_0   |
| B8                      | 1A0       | 4         | 0       | STS1_DIAG_1   |
| BC                      | 1A4       | 4         | 0       | STS1_DIAG_2   |
| C0                      | 1A8       | 4         | 0       | STS1_DIAG_3   |
| C4                      | 1AC       | 4         | 0       | STS1_DIAG_4   |
| C8                      | 1B0       | 4         | 0       | Reserved      |
| CC                      | 1B4       | 4         | 0       | Reserved      |
| D0                      | 1B8       | 4         | 0       | Reserved      |
| D4                      | 1BC       | 4         | 0       | STS1_DIAG_8   |
| D8                      | 1C0       | 4         | 0       | Reserved      |
| DC                      | 1C4       | 4         | 0       | Reserved      |
| E0                      | 1C8       | 4         | 0       | Reserved      |
| E4                      | 1CC       | 4         | 0       | STS1_DIAG_12  |

---

### 8.2.23 Register file: 0x1D – Indirect pointer A

| ID  | Length (octets) | Type | Mnemonic       | Description        |
| --- | --------------- | ---- | -------------- | ------------------ |
| 1D  | -               | RW   | INDIRECT_PTR_A | Indirect pointer A |

Register file 0x1E:00 is the indirect pointer A. When reading or writing to this register the host will access register that was programmed into PTR_ADDR_A, at offset programmed into PTR_OFFSET_A. The indirect register address is needed when accessing the register contents starting at offset > 127.

---

### 8.2.24 Register file: 0x1E – Indirect pointer B

| ID  | Length (octets) | Type | Mnemonic       | Description        |
| --- | --------------- | ---- | -------------- | ------------------ |
| 1E  | -               | RW   | INDIRECT_PTR_B | Indirect pointer B |

Register file 0x1E:00 is the indirect pointer B. When reading or writing to this register the host will access register that was programmed into PTR_ADDR_B, at offset programmed into PTR_OFFSET_B. The indirect register address is needed when accessing the register contents starting at offset > 127.

---

### 8.2.25 Register file: 0x1F – FINT status and indirect pointer interface

| ID  | Length (octets) | Type | Mnemonic   | Description                                                       |
| --- | --------------- | ---- | ---------- | ----------------------------------------------------------------- |
| 1F  | 19              | -    | IN_PTR_CFG | Indirect pointer configuration and fast interrupt status register |

Register file 0x1F contains the reduced status events set status register and the indirect pointer configuration interface. The latter allows read/write access to registers with register subaddresses > 127. For example to read 10 bytes from RX_BUFFER_0 from any offset <= 127, a normal SPI read transaction should be used. However to read the same bytes from RX_BUFFER_0 with offset > 127, the indirect access needs to be used. This is achieved as follows (either pointer A (INDIRECT_PTR_A) or pointer B (INDIRECT_PTR_B) can be used for this):

- Program the base address of the register to be accessed through the indirect pointer, if using pointer A then program PTR_ADDR_A
- Program the offset of the register to be accessed, if using pointer A then program PTR_OFFSET_A
- Read/Write the desired data by using a standard SPI read/write transaction from INDIRECT_PTR_A.

There are a number of sub-registers in this register file. An overview of these sub-registers is given by Table 45, and each is then separately described in the sub-sections below.

**Table 45: Register file: 0x1F – FINT status and indirect pointer interface overview**

| OFFSET in Register 0x1F | Mnemonic     | Description                                                 |
| ----------------------- | ------------ | ----------------------------------------------------------- |
| 0x00                    | FINT_STAT    | Fast System Event Status Register                           |
| 0x04                    | PTR_ADDR_A   | Base address of register to be accessed through pointer A   |
| 0x08                    | PTR_OFFSET_A | Offset address of register to be accessed through pointer A |
| 0x0C                    | PTR_ADDR_B   | Base address of register to be accessed through pointer B   |
| 0x10                    | PTR_OFFSET_B | Offset address of register to be accessed through pointer B |

---

#### 8.2.25.1 Sub-register 0x1F:00 – Fast system event status

| ID    | Length (octets) | Type | Mnemonic  | Description                       |
| ----- | --------------- | ---- | --------- | --------------------------------- |
| 1F:00 | 1               | RO   | FINT_STAT | Fast system event status register |

Register file: 0x1F – FINT status and indirect pointer interface, sub-register 0x00 is the reduced system event status register, FINT_STAT. It contains status bits that indicate the occurrence of different system events or status changes. It is a reduced set of SYS_STATUS events. Reading the FINT_STAT register returns the state of the status bits as long as the matching SYS_ENABLE event bit is set. Thus it will report the events which gave rise to the interrupt. Generally these event status bits are latched so that the event is captured. The bits will have to be cleared by writing to relevant SYS_STATUS bits. The FINT_STAT register contains the system event status bits identified and described below:

**REG:1F:00 – FINT_STAT – Fast system status register**

| Bits | Field     | Default | Description                 |
| ---- | --------- | ------- | --------------------------- |
| 0    | TXOK      | 0       | TX frame sent OK            |
| 1    | CCA_FAIL  | 0       | CCA fail                    |
| 2    | RXTSERR   | 0       | RX timestamp error (CIAERR) |
| 3    | RXOK      | 0       | RX frame received OK        |
| 4    | RXERR     | 0       | RX error                    |
| 5    | RXTO      | 0       | RX timeout                  |
| 6    | SYS_EVENT | 0       | System event                |
| 7    | SYS_PANIC | 0       | System panic / error        |

| Field                           | Description of fields within Sub-register 0x1F:00 – Fast system event status                                                                                                                                                                                                                                                                        |
| ------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -<br>reg:1F:00<br>bit:various   | Bits marked '-' are reserved and should not be changed.                                                                                                                                                                                                                                                                                             |
| TXOK<br>reg:1F:00<br>bit:0      | This bit will be set if any of the following events are set in the SYS_STATUS register: TXFRB or TXPRS or TXPHS or TXFRS. This is a READ ONLY status flag – it will be cleared by writing 1 to relevant status bits (as listed above) in Sub-register 0x00:44 – System event status, whichever ones are set.                                        |
| CCA_FAIL<br>reg:1F:00<br>bit:1  | This bit will be set if any of the following events are set in the SYS_STATUS register: AAT or CCA_FAIL. This is a READ ONLY status flag – it will be cleared by writing 1 to Status bit in Sub-register 0x00:44 – System event status.                                                                                                             |
| RXTSERR<br>reg:1F:00<br>bit:2   | This bit will be set if CIAERR event is set in the SYS_STATUS register. This is a READ ONLY status flag – it will be cleared by writing 1 to CIAERR status bit in Sub-register 0x00:44 – System event status.                                                                                                                                       |
| RXOK<br>reg:1F:00<br>bit:3      | This bit will be set if any of the following events are set in the SYS_STATUS register: RXFR and CIADONE or RXFCG. This is a READ ONLY status flag – it will be cleared by writing 1 to relevant status bits (as listed above) in Sub-register 0x00:44 – System event status.                                                                       |
| RXERR<br>reg:1F:00<br>bit:4     | This bit will be set if any of the following events are set in the SYS_STATUS register: RXFCE or RXFSL or RXPHE or ARFE or RXSTO or RXOVRR. This is a READ ONLY status flag – it will be cleared by writing 1 to relevant status bits (as listed above) in Sub-register 0x00:44 – System event status, whichever ones are set.                      |
| RXTO<br>reg:1F:00<br>bit:5      | This bit will be set if any of the following events are set in the SYS_STATUS register: RXFTO or RXPTO. This is a READ ONLY status flag – it will be cleared by writing 1 to RXFTO or RXPTO status bits in Sub-register 0x00:44 – System event status, whichever ones are set.                                                                      |
| SYS_EVENT<br>reg:1F:00<br>bit:6 | This bit will be set if any of the following events are set in the SYS_STATUS register: VT_DET or GPIOIRQ or RCINIT or SPIRDY. This is a READ ONLY status flag – it will be cleared by writing 1 to relevant status bits (as listed above) in Sub-register 0x00:44 – System event status, whichever ones are set.                                   |
| SYS_PANIC<br>reg:1F:00<br>bit:7 | This bit will be set if any of the following events are set in the SYS_STATUS register: AES_ERR or CMD_ERR or SPI_UNF or SPI_OVF or SPIERR or PLL_HILO or VWARN. This is a READ ONLY status flag – it will be cleared by writing 1 to relevant status bits (as listed above) in Sub-register 0x00:44 – System event status, whichever ones are set. |

---

#### 8.2.25.2 Sub-register 0x1F:04 – Pointer A reg base address

| ID    | Length (octets) | Type | Mnemonic   | Description                                                            |
| ----- | --------------- | ---- | ---------- | ---------------------------------------------------------------------- |
| 1F:04 | 1               | RW   | PTR_ADDR_A | Base address of the register to be accessed through indirect pointer A |

Register file: 0x1F – FINT status and indirect pointer interface, sub-register 0x04 contains the base address of the register to be accessed through indirect pointer A.

**REG:1F:04 – PTR_ADDR_A – Base address of the register to access**

| Bits | Field     | Default | Description                                             |
| ---- | --------- | ------- | ------------------------------------------------------- |
| 4:0  | PTRA_BASE | 0       | Base address of the register to access (INDIRECT_PTR_A) |
| 7:5  | -         | 0       | Reserved                                                |

| Field                              | Description of fields within Sub-register 0x1F:04 – Pointer A reg base address               |
| ---------------------------------- | -------------------------------------------------------------------------------------------- |
| PTRA_BASE<br>reg:1F:04<br>bits:4-0 | The base address of the register to be accessed through indirect pointer A (INDIRECT_PTR_A). |
| –<br>reg:1F:04<br>bits:7-5         | Bits marked '-' are reserved and should not be changed.                                      |

---

#### 8.2.25.3 Sub-register 0x1F:08 – Pointer A reg offset address

| ID    | Length (octets) | Type | Mnemonic     | Description                                                              |
| ----- | --------------- | ---- | ------------ | ------------------------------------------------------------------------ |
| 1F:08 | 2               | RW   | PTR_OFFSET_A | Offset address of the register to be accessed through indirect pointer A |

Register file: 0x1F – FINT status and indirect pointer interface, sub-register 0x08 contains the offset address of the register to be accessed through indirect pointer A.

**REG:1F:08 – PTR_OFFSET_A – Base address of the register to access**

| Bits | Field    | Default | Description                                               |
| ---- | -------- | ------- | --------------------------------------------------------- |
| 14:0 | PTRA_OFS | 0       | Offset address of the register to access (INDIRECT_PTR_A) |
| 15   | -        | 0       | Reserved                                                  |

| Field                              | Description of fields within Sub-register 0x1F:08 – Pointer A reg offset address               |
| ---------------------------------- | ---------------------------------------------------------------------------------------------- |
| PTRA_OFS<br>reg:1F:08<br>bits:14-0 | The offset address of the register to be accessed through indirect pointer A (INDIRECT_PTR_A). |
| –<br>reg:1F:04<br>bit:15           | Bits marked '-' are reserved and should not be changed.                                        |

---

#### 8.2.25.4 Sub-register 0x1F:0C – Pointer B reg base address

| ID    | Length (octets) | Type | Mnemonic   | Description                                                            |
| ----- | --------------- | ---- | ---------- | ---------------------------------------------------------------------- |
| 1F:0C | 1               | RW   | PTR_ADDR_B | Base address of the register to be accessed through indirect pointer B |

Register file: 0x1F – FINT status and indirect pointer interface, sub-register 0x0C contains the base address of the register to be accessed through indirect pointer B. **Note: in the API indirect pointer B is reserved for double buffer SET_2 access.**

**REG:1F:0C – PTR_ADDR_B – Base address of the register to access**

| Bits | Field     | Default | Description                                             |
| ---- | --------- | ------- | ------------------------------------------------------- |
| 4:0  | PTRB_BASE | 0       | Base address of the register to access (INDIRECT_PTR_B) |
| 7:5  | -         | 0       | Reserved                                                |

| Field                              | Description of fields within Sub-register 0x1F:0C – Pointer B reg base address               |
| ---------------------------------- | -------------------------------------------------------------------------------------------- |
| PTRB_BASE<br>reg:1F:0C<br>bits:4-0 | The base address of the register to be accessed through indirect pointer B (INDIRECT_PTR_B). |
| –<br>reg:1F:0C<br>bits:7-5         | Bits marked '-' are reserved and should not be changed.                                      |

---

#### 8.2.25.5 Sub-register 0x1F:10 – Pointer B reg offset address

| ID    | Length (octets) | Type | Mnemonic     | Description                                                              |
| ----- | --------------- | ---- | ------------ | ------------------------------------------------------------------------ |
| 1F:10 | 2               | RW   | PTR_OFFSET_B | Offset address of the register to be accessed through indirect pointer B |

Register file: 0x1F – FINT status and indirect pointer interface, sub-register 0x10 contains the offset address of the register to be accessed through indirect pointer B. **Note: in the API indirect pointer B is reserved for double buffer SET_2 access.**

**REG:1F:10 – PTR_OFFSET_B – Base address of the register to access**

| Bits | Field    | Default | Description                                               |
| ---- | -------- | ------- | --------------------------------------------------------- |
| 14:0 | PTRB_OFS | 0       | Offset address of the register to access (INDIRECT_PTR_B) |
| 15   | -        | 0       | Reserved                                                  |

| Field                              | Description of fields within Sub-register 0x1F:10 – Pointer B reg offset address               |
| ---------------------------------- | ---------------------------------------------------------------------------------------------- |
| PTRA_OFS<br>reg:1F:10<br>bits:14-0 | The offset address of the register to be accessed through indirect pointer B (INDIRECT_PTR_B). |
| –<br>reg:1F:10<br>bit:15           | Bits marked '-' are reserved and should not be changed.                                        |
