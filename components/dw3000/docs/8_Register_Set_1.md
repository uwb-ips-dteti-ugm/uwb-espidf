# 8 The DW3000 Register Set

The DW3000 is controlled by an associated host microcontroller system using the SPI interface to access a series of registers within the device. The DW3000 register set includes configuration registers, status registers, control registers, data buffer registers, and diagnostic registers. Section 8.1 gives an overview of the register layout and section 8.2 describes each individual parameter in detail. There is also a set of single octet commands to initiate certain IC activities (e.g. TX, RX, etc) which are described in section 9 – Fast Commands. The SPI transaction formats are described in section 2.3 – The SPI interface.

---

## 8.1 Register Map Overview

The register map overview is given in Table 18. This lists the registers in address order, by register file ID and it gives a brief high level description of the register. Section 8.2 gives a detailed description of each register.

> **Note:** When writing to any of the DW3000 registers care must be taken not to write beyond the documented length of the selected register and not to write to any of the reserved register locations. Doing so may cause the device to malfunction.

**Table 18: Register Map Overview**

| ID        | Mnemonic       | Description                                                                        |
| --------- | -------------- | ---------------------------------------------------------------------------------- |
| 0x00–0x01 | GEN_CFG_AES    | Main register bank and Advanced Encryption Standard configuration set of registers |
| 0x02      | STS_CFG        | Scrambled timestamp sequence configuration                                         |
| 0x03      | RX_TUNE        | RX tuning register                                                                 |
| 0x04      | EXT_SYNC       | External synchronisation control                                                   |
| 0x05      | GPIO_CTRL      | GPIO control registers                                                             |
| 0x06      | DRX            | Digital receiver tuning and configuration                                          |
| 0x07      | RF_CONF        | Analog RF configuration                                                            |
| 0x08      | RF_CAL         | Transmitter calibration block                                                      |
| 0x09      | FS_CTRL        | Frequency synthesiser control block                                                |
| 0x0A      | AON            | Always-On register set                                                             |
| 0x0B      | OTP_IF         | One Time Programmable memory interface                                             |
| 0x0C–0x0E | CIA            | Channel Impulse Analysis control and diagnostic block                              |
| 0x0F      | DIG_DIAG       | Digital diagnostics                                                                |
| 0x11      | PMSC           | Power Management System Control block                                              |
| 0x12      | RX_BUFFER_0    | Receive data buffer                                                                |
| 0x13      | RX_BUFFER_1    | Second receive data buffer                                                         |
| 0x14      | TX_BUFFER      | Transmit data buffer                                                               |
| 0x15      | ACC_MEM        | Read access to Channel Impulse Response data                                       |
| 0x16      | SCRATCH_RAM    | Scratch RAM                                                                        |
| 0x17      | AES_RAM        | AES key RAM                                                                        |
| 0x18      | SET_1, SET_2   | Double buffer diagnostic sets                                                      |
| 0x1D      | INDIRECT_PTR_A | Indirect pointer A buffer                                                          |
| 0x1E      | INDIRECT_PTR_B | Indirect pointer B buffer                                                          |
| 0x1F      | IN_PTR_CFG     | Indirect pointer access configuration, and "fast" status register                  |

---

## 8.2 Detailed Register Description

### 8.2.1 Terminology

Section 8.1 gives an overview of the DW3000 register set presenting all top level register file addresses in Table 18. This section describes in detail the contents and functionality of these register files and their sub-registers in separate sub sections. In each case the row from Table 18 is reproduced with a hexadecimal register ID, its length, type, mnemonic and one line description as follows:

| ID  | Length (octets) | Type | Mnemonic | Description |
| --- | --------------- | ---- | -------- | ----------- |

This is followed by a description of the parameters within that register file. All parameters are presented with format `REG:RR:SS`, where `RR` is register file ID (5-bits) and `SS` (7-bits) is the sub address. Where a register is made up of individual bits or bit-fields these are identified with mnemonic and default power-on-reset values.

Because many parameters are 4-octets long, the default presentation of the register values is as a 32-bit value. This may be sub-divided into fields of various bit widths down to single bit values. It should be noted that when reading these values via the SPI interface the octets are output least significant octet first. The indexed addressing modes allow individual octets to be accessed – a technique that may be employed to reduce SPI traffic when only part of a register needs to be read or written to.

> **Note:** Unused or reserved registers return `0xDEADDEAD` when read. Unused or reserved bits/bit fields within registers return the appropriate bits/bit fields from `0xDEADDEAD`.

---

### 8.2.2 Register File: 0x00-0x01 – General Configuration Registers and AES

| ID        | Length (octets) | Type | Mnemonic    | Description                                                       |
| --------- | --------------- | ---- | ----------- | ----------------------------------------------------------------- |
| 0x00–0x01 | 121             | -    | GEN_CFG_AES | Main register bank and Advanced Encryption Standard configuration |

Register files 0x00 and 0x01 are concerned with the use of the various device configurations and AES block configuration. It contains a number of sub-registers. An overview of these is given in Table 19. Each of these sub-registers is separately described in the sub-sections below.

**Table 19: Register File 0x00-0x01 – General Configuration Registers Overview**

| Register File ID | Offset | Mnemonic      | Description                               |
| ---------------- | ------ | ------------- | ----------------------------------------- |
| 0x00             | 0x00   | DEV_ID        | Device Identifier                         |
| 0x00             | 0x04   | EUI_64        | Extended Unique Identifier                |
| 0x00             | 0x0C   | PANADR        | PAN Identifier and Short Address          |
| 0x00             | 0x10   | SYS_CFG       | System Configuration                      |
| 0x00             | 0x14   | FF_CFG        | Frame Filter Configuration                |
| 0x00             | 0x18   | SPI_RD_CRC    | SPI CRC read status                       |
| 0x00             | 0x1C   | SYS_TIME      | System Time Counter                       |
| 0x00             | 0x24   | TX_FCTRL      | Transmit Frame Control                    |
| 0x00             | 0x2C   | DX_TIME       | Delayed Send or Receive time              |
| 0x00             | 0x30   | DREF_TIME     | Delayed Send or Receive Reference time    |
| 0x00             | 0x34   | RX_FWTO       | Receive Frame Wait Timeout period         |
| 0x00             | 0x38   | SYS_CTRL      | System Control                            |
| 0x00             | 0x3C   | SYS_ENABLE    | System Event Enable Mask                  |
| 0x00             | 0x44   | SYS_STATUS    | System Event Status Register              |
| 0x00             | 0x4C   | RX_FINFO      | RX Frame Information                      |
| 0x00             | 0x64   | RX_TIME       | Receive Time Stamp                        |
| 0x00             | 0x74   | TX_TIME       | Transmit Time Stamp                       |
| 0x01             | 0x00   | TX_RAWST      | Unadjusted/raw TX timestamp               |
| 0x01             | 0x04   | TX_ANTD       | 16-bit Delay from Transmit to Antenna     |
| 0x01             | 0x08   | ACK_RESP_T    | Acknowledgement Time and Response Time    |
| 0x01             | 0x0C   | TX_POWER      | TX Power Control                          |
| 0x01             | 0x14   | CHAN_CTRL     | Channel Control Register                  |
| 0x01             | 0x18   | LE_PEND_01    | Low Energy device address 0 and 1         |
| 0x01             | 0x1C   | LE_PEND_23    | Low Energy device address 2 and 3         |
| 0x01             | 0x20   | SPI_COLLISION | SPI Collision Status                      |
| 0x01             | 0x24   | RDB_STATUS    | RX Double Buffer Status                   |
| 0x01             | 0x28   | RDB_DIAG      | RX Double Buffer Diagnostic Configuration |
| 0x01             | 0x30   | AES_CFG       | AES Configuration                         |
| 0x01             | 0x34   | AES_IV0       | The 3rd IV word for the AES GCM/CCM* core |
| 0x01             | 0x38   | AES_IV1       | The 2nd IV word for the AES GCM/CCM* core |
| 0x01             | 0x3C   | AES_IV2       | The 1st IV word for the AES GCM/CCM* core |
| 0x01             | 0x40   | AES_IV3       | The 4th IV word for the AES GCM/CCM* core |
| 0x01             | 0x44   | DMA_CFG       | The DMA Configuration Register            |
| 0x01             | 0x4C   | AES_START     | Start AES operation                       |
| 0x01             | 0x50   | AES_STS       | The AES Status                            |
| 0x01             | 0x54   | AES_KEY       | The 128-bit KEY for the AES GCM/CCM* core |

---

#### 8.2.2.1 Sub-register 0x00:00 – Device Identifier

| ID    | Length (octets) | Type | Mnemonic | Description                                                       |
| ----- | --------------- | ---- | -------- | ----------------------------------------------------------------- |
| 00:00 | 4               | RO   | DEV_ID   | Device Identifier – includes device type and revision information |

Register file 0x00-0x01, sub-register 0x00 is the device identifier. This is hard-coded into the silicon. The value in this register is read-only and cannot be overwritten by the host system. The device ID will be changed for any silicon updates. It is expected that the host system will validate that the device ID is the expected value, supported by its software, before proceeding to use the IC.

**REG:00:00 – DEV_ID – Device Identifier**

| Bits  | Field  | Default  |
| ----- | ------ | -------- |
| 31:16 | RIDTAG | 0xDECA   |
| 15:8  | MODEL  | 0x03     |
| 7:4   | VER    | (varies) |
| 3:0   | REV    | (varies) |

| Field  | Bits                 | Description                                                                                                                                                                                                             |
| ------ | -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| REV    | reg:00:00 bits:3–0   | Revision. This number will be updated for minor corrections and changes in device's operation.                                                                                                                          |
| VER    | reg:00:00 bits:7–4   | Version. This number will be updated if a new version is produced that has significant differences from the previous version. There are currently two versions: `0xDECA0302` (non-PDoA) and `0xDECA0312` (PDoA device). |
| MODEL  | reg:00:00 bits:15–8  | The MODEL identifies the device type. The DW3000 is device type `0x03`.                                                                                                                                                 |
| RIDTAG | reg:00:00 bits:31–16 | Register Identification Tag. Remains constant for all Decawave parts. The value is `0xDECA` in hex.                                                                                                                     |

> **For the production DW3000 the Device ID is set to `0xDECA0312` or `0xDECA0302`. The register descriptions in this user manual relate to that DW3000 device and are not valid for any earlier sample parts.**

---

#### 8.2.2.2 Sub-register 0x00:04 – Extended Unique Identifier

| ID    | Length (octets) | Type | Mnemonic | Description                                                 |
| ----- | --------------- | ---- | -------- | ----------------------------------------------------------- |
| 00:04 | 8               | RW   | EUI_64   | Extended Unique Identifier – the 64-bit IEEE device address |

Register file 0x00-0x01, sub-register 0x04 of register file 0x00 is the Extended Unique Identifier register. For IEEE802.15.4 standard compliance every device should have a unique 64-bit device identifier. The high-order 24-bits of the EUI are a *company identifier* assigned by the IEEE Registration Authority to the manufacturer. The low 40-bits of the EUI are the *extension identifier* uniquely chosen by the manufacturer for each device manufactured and never repeated.

The EUI register is used by the Receive Frame Filtering function. When frame filtering is operational the DW3000 decodes each received frame according to the IEEE802.15.4 standard MAC rules and any 64-bit destination address present must match the EUI register before the frame will be accepted.

**REG:00:04 – EUI – Extended Unique Identifier (octets output least significant first)**

| Octet Index | Description                                        |
| ----------- | -------------------------------------------------- |
| 0           | Bits 7 to 0 of the extension identifier            |
| 1           | Bits 15 to 8 of the extension identifier           |
| 2           | Bits 23 to 16 of the extension identifier          |
| 3           | Bits 31 to 24 of the extension identifier          |
| 4           | Bits 39 to 32 of the extension identifier          |
| 5           | Bits 7 to 0 of the OUI (manufacturer company ID)   |
| 6           | Bits 15 to 8 of the OUI (manufacturer company ID)  |
| 7           | Bits 23 to 16 of the OUI (manufacturer company ID) |

The ordering of octets read from the Extended Unique Identifier register is designed to be directly compatible with the octet ordering of the 64-bit source address fields of IEEE802.15.4 standard MAC frames, easing the task of inserting it into a frame for transmission.

---

#### 8.2.2.3 Sub-register 0x00:0C – PAN Identifier and Short Address

| ID    | Length (octets) | Type | Mnemonic | Description                      |
| ----- | --------------- | ---- | -------- | -------------------------------- |
| 00:0C | 4               | RW   | PANADR   | PAN Identifier and Short Address |

Register file 0x00-0x01, sub-register 0x0C of register file 0x00 contains two 16-bit parameters, the *PAN Identifier* and the *Short Address*. When the DW3000 is powered up or reset both the PAN Identifier and the Short Address in this register are reset to the value `0xFFFF`. The host software (MAC) should program the appropriate values into this register if it wishes to use the DW3000's receive frame filtering or automatic acknowledgement generation functions.

**REG:00:0C – PANADR – PAN Identifier and Short Address**

| Bits  | Field     | Default |
| ----- | --------- | ------- |
| 31:16 | PAN_ID    | 0xFFFF  |
| 15:0  | SHORTADDR | 0xFFFF  |

| Field     | Bits                 | Description                                                                                                                                                                                                                                                                                                                                                     |
| --------- | -------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SHORTADDR | reg:00:0C bits:15–0  | Short Address. The host software needs to program this register if it is using the DW3000's receive frame filtering functionality, with or without the automatic acknowledgement generation function. The short address is typically assigned to a node by the coordinator function at MAC (or higher) layer as part of network association. Default: `0xFFFF`. |
| PAN_ID    | reg:00:0C bits:31–16 | PAN Identifier. The host software needs to program this register if it is using the DW3000's receive frame filtering functionality, with or without the automatic acknowledgement generation function. The PAN ID is typically assigned as part of network association. Default: `0xFFFF`.                                                                      |

---

#### 8.2.2.4 Sub-register 0x00:10 – System Configuration

| ID    | Length (octets) | Type | Mnemonic | Description                 |
| ----- | --------------- | ---- | -------- | --------------------------- |
| 00:10 | 4               | RW   | SYS_CFG  | System configuration bitmap |

Register file 0x00-0x01, sub-register 0x10 of register file 0x00 is the system configuration register. This is a bitmapped register. Each bit field is separately identified and described below.

**REG:00:10 – SYS_CFG – System Configuration Bit Map**

| Bits  | Field        |
| ----- | ------------ |
| 31:19 | - (reserved) |
| 18    | FAST_AAT     |
| 17:16 | PDOA_MODE    |
| 15    | CP_SDC       |
| 14    | - (reserved) |
| 13:12 | CP_SPC       |
| 11    | AUTO_ACK     |
| 10    | RXAUTR       |
| 9     | RXWTOE       |
| 8     | CIA_STS      |
| 7     | CIA_IPATOV   |
| 6     | SPI_CRCEN    |
| 5     | PHR_6M8      |
| 4     | PHR_MODE     |
| 3     | DIS_DRXB     |
| 2     | DIS_FCE      |
| 1     | DIS_FCS_TX   |
| 0     | FFEN         |

| Field      | Bits                 | Description                                                                                                                                                                                                                                                                                                                                                                                                    |
| ---------- | -------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| FFEN       | reg:00:10 bit:0      | Frame Filtering Enable. Enables the frame filtering functionality in the DW3000 receiver, designed to follow IEEE802.15.4 rules. When enabled, received frames must pass the frame filtering rules before being considered a good frame. Configure FF_CFG (0x00:14) before enabling this bit. See section 5.4.                                                                                                 |
| DIS_FCS_TX | reg:00:10 bit:1      | Disable auto-FCS Transmission. If this bit is not set, the DW3000 automatically calculates and appends the two Frame-Check-Sequence bytes at the end of each transmitted frame (packet configs 0, 1, and 2). When DIS_FCS_TX is set, TXFLEN (frame length) octets are sent directly from the TX buffer.                                                                                                        |
| DIS_FCE    | reg:00:10 bit:2      | Disable frame check error handling. For normal IEEE802.15.4 operation this bit should be set to zero. Setting to one makes the DW3000 treat the frame as valid, ignoring errors in the CRC frame check sequence.                                                                                                                                                                                               |
| DIS_DRXB   | reg:00:10 bit:3      | Disable Double RX Buffer. Double buffering is enabled when DIS_DRXB = 0 (default), and disabled when DIS_DRXB = 1. See section 4.4 for details on double buffering.                                                                                                                                                                                                                                            |
| PHR_MODE   | reg:00:10 bit:4      | PHR type selection. 0 = Standard Frame mode per IEEE802.15.4 (max 127 octets payload). 1 = Long Frame mode per IEEE802.15.8 (max 1023 octets payload). Both nodes must be configured for the same PHR mode for successful communications.                                                                                                                                                                      |
| PHR_6M8    | reg:00:10 bit:5      | PHR rate. Sets the PHR rate to match the 6.81 Mb/s data rate when set. When 0, PHR is sent at 850 kb/s. This bit is ignored if a data rate other than 6.81 Mb/s is selected.                                                                                                                                                                                                                                   |
| SPI_CRCEN  | reg:00:10 bit:6      | Enable SPI CRC functionality. When set, enables a CRC for SPI read and write transactions. SPI write accesses are assumed to have a CRC byte appended; any mismatch is flagged in the SPICRCERR event status bit. SPI read accesses have a CRC computed internally available in the SPI_RD_CRC register. See §2.3.1.3.                                                                                         |
| CIA_IPATOV | reg:00:10 bit:7      | Select CIA processing of the preamble CIR. Default 1. When set, the CIA analyses the preamble CIR and computes an RX timestamp estimate. This is written to IP_TOA (0x0C:00) and also to RX_STAMP (0x00:64). When CIA_IPATOV = 0, the CIA will not run on the preamble CIR. CIA analysis is started upon SFD detection unless an STS is expected, in which case it is delayed until after the STS is received. |
| CIA_STS    | reg:00:10 bit:8      | Select CIA processing of the STS CIR. Default 1. When set and STS is included in the packet (as configured via CP_SPC), the CIA analyses the STS CIR and computes an RX timestamp estimate. This is written to STS_TOA (0x0C:08) and also to RX_STAMP (0x00:64), overwriting the preamble-based estimate. When CIA_STS = 0, the CIA will not analyse the STS CIR.                                              |
| RXWTOE     | reg:00:10 bit:9      | Receive Wait Timeout Enable. When set, RX Enable will initialise an RX_FWTO down counter which will disable the receiver if no valid frame is received before the timeout occurs. The timeout period is set in RX_FWTO (0x00:34). The timeout is signalled by the RXFTO event status bit in SYS_STATUS (0x00:44).                                                                                              |
| RXAUTR     | reg:00:10 bit:10     | Receiver Auto-Re-enable. Default 0. In single-buffered mode: after a frame reception failure (except a frame wait timeout), the receiver re-enables to re-attempt reception. **In double-buffered mode: Not supported – double buffer mode must be used with RXAUTR set to 0.**                                                                                                                                |
| AUTO_ACK   | reg:00:10 bit:11     | Automatic Acknowledgement Enable. Default 0. Enable for the automatic acknowledgement function. See section 5.5 for details.                                                                                                                                                                                                                                                                                   |
| CP_SPC     | reg:00:10 bits:13-12 | STS packet configuration. The IC supports four STS packet configuration formats: 0 = SP0 – No STS in the packet; 1 = SP1 – STS is between the SDF and the PHR; 2 = SP2 – STS is after the data (end of packet); 3 = SP3 – STS is after the SDF but there is no PHR or data. **N.B. When CP_SPC = 2, data length must be non-zero. This parameter must be set before CP_LOADIV is asserted.**                   |
| -          | reg:00:10 bit:14     | Reserved.                                                                                                                                                                                                                                                                                                                                                                                                      |
| CP_SDC     | reg:00:10 bit:15     | Super Deterministic Code (SDC) mode. When set, the SDC mode uses a pre-programmed sequence in the transmitter and receiver; STS KEY/IV values are ignored. When clear, standard IEEE 802.15.4z counter mode is used.                                                                                                                                                                                           |
| PDOA_MODE  | reg:00:10 bits:17-16 | PDoA mode. 0x0 = PDoA disabled; 0x1 = PDoA Mode 1; 0x2 = reserved/not supported; 0x3 = PDoA Mode 3. See section 4.2.                                                                                                                                                                                                                                                                                           |
| FAST_AAT   | reg:00:10 bit:18     | Enable fast RX to TX turn around mode. When set, the receiver will not wait for CIADONE before signalling end of reception (RXFR). The host can then start processing the received frame. If AUTO_ACK is configured, the AAT bit will also be set to signal ACK is being sent (assuming it has been requested and other frame filtering rules pass).                                                           |
| -          | reg:00:10 bits:31-19 | Reserved.                                                                                                                                                                                                                                                                                                                                                                                                      |

---

#### 8.2.2.5 Sub-register 0x00:14 – Frame Filter Configuration

| ID    | Length (octets) | Type | Mnemonic | Description                       |
| ----- | --------------- | ---- | -------- | --------------------------------- |
| 00:14 | 2               | RW   | FF_CFG   | Frame filter configuration bitmap |

Register file 0x00-0x01, sub-register 0x14 of register file 0x00 is the IEEE802.15.4 standard MAC frame address filter configuration register. This is a bitmapped register. Each bit field is separately identified and described below.

> **Note: For any of these bits to apply, the FFEN bit in SYS_CFG (0x00:10) must also be set.**

**REG:00:14 – FF_CFG – Frame Filter Configuration Bit Map**

| Bits  | Field        | Default |
| ----- | ------------ | ------- |
| 31:16 | - (reserved) | 0       |
| 15    | LSADRAPE     | 0       |
| 14    | SSADRAPE     | 0       |
| 13    | LE3_PEND     | 0       |
| 12    | LE2_PEND     | 0       |
| 11    | LE1_PEND     | 0       |
| 10    | LE0_PEND     | 0       |
| 9     | FFIB         | 0       |
| 8     | FFBC         | 0       |
| 7     | FFAE         | 0       |
| 6     | FFAF         | 0       |
| 5     | FFAMULTI     | 0       |
| 4     | FFAR         | 0       |
| 3     | FFAM         | 0       |
| 2     | FFAA         | 0       |
| 1     | FFAD         | 0       |
| 0     | FFAB         | 0       |

| Field    | Bits             | Description                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| FFAB     | reg:00:14 bit:0  | Frame Filtering Allow Beacon frame reception. IEEE802.15.4 frames begin with three bits indicating the frame type (b3 to b0). For beacon frames these are binary `000`. When FFAB = 1 beacon frames will be accepted (assuming all other frame filtering rules are passed). When FFAB = 0 beacon frames will be ignored. See section 5.4.                                                                                                        |
| FFAD     | reg:00:14 bit:1  | Frame Filtering Allow Data frame reception. For data frames the type bits are binary `001`. When FFAD = 1 data frames will be accepted. When FFAD = 0 data frames will be ignored. See section 5.4.                                                                                                                                                                                                                                              |
| FFAA     | reg:00:14 bit:2  | Frame Filtering Allow Acknowledgment frame reception. For acknowledgment frames the type bits are binary `010`. When FFAA = 1 acknowledgment frames will be accepted. When FFAA = 0 they will be ignored. See section 5.4.                                                                                                                                                                                                                       |
| FFAM     | reg:00:14 bit:3  | Frame Filtering Allow MAC command frame reception. For MAC command frames the type bits are binary `011`. When FFAM = 1 MAC command frames will be accepted. When FFAM = 0 they will be ignored. See section 5.4.                                                                                                                                                                                                                                |
| FFAR     | reg:00:14 bit:4  | Frame Filtering Allow Reserved frame types. For reserved frames the type bits are binary `100`. When FFAR = 1 these frames are accepted. Since the frame types are unknown no further frame decoding is done (no address matching etc.) and software will be responsible for validating and interpreting these frames. See section 5.4.                                                                                                          |
| FFAMULTI | reg:00:14 bit:5  | Frame Filtering Allow Multipurpose frames. For multipurpose frames the type bits are binary `101`. When FFAMULTI = 1 these frames are accepted. When FFAMULTI = 0 they will be ignored. The frame filter decodes the frame control fields to determine the minimum expected frame length and will reject the frame if it is too short. See section 5.4.                                                                                          |
| FFAF     | reg:00:14 bit:6  | Frame Filtering Allow Fragmented/Frack frames. For fragmented frames the type bits are binary `110`. When FFAF = 1 these frames are accepted. When FFAF = 0 they will be ignored. The frame filter decodes the frame control fields to determine the minimum expected frame length and will reject the frame if it is too short. See section 5.4.                                                                                                |
| FFAE     | reg:00:14 bit:7  | Frame Filtering Allow Extended frames. For extended frames the type bits are binary `111`. When FFAE = 1 these frames are accepted. When FFAE = 0 they will be ignored. The frame filter decodes the frame control fields to determine the minimum expected frame length and will reject the frame if it is too short. See section 5.4.                                                                                                          |
| FFBC     | reg:00:14 bit:8  | Frame Filtering Behave as PAN Coordinator. When FFBC = 1 the device behaves as a PAN coordinator, accepting frames with only source addressing fields where: (a) Source PAN ID must match PAN_ID set (for MAC and data frame types); (b) Destination PAN ID must match PAN_ID set (for Multipurpose frame type). See section 5.4.                                                                                                                |
| FFIB     | reg:00:14 bit:9  | Frame Filtering allow MAC Implicit Broadcast. When FFIB = 0, the destination addressing (PAN ID and destination address fields, if present) in the received frame must either be set to broadcast (`0xFFFF`) or a specific destination address. When FFIB = 1, frames without destination PAN ID and destination address are treated as though they are addressed to the broadcast PAN ID and broadcast short (16-bit) address. See section 5.4. |
| LE0_PEND | reg:00:14 bit:10 | Data pending for device at LE0 address. When set and the source address of a received MAC command Data Request frame matches the address set in LE_ADDR0 in LE_PEND_01 (0x01:18), the automatically transmitted ACK will have the PEND bit set. Note: AUTO_ACK in SYS_CFG and frame filtering rules in FF_CFG must also be configured. See section 5.4.                                                                                          |
| LE1_PEND | reg:00:14 bit:11 | Data pending for device at LE1 address. When set and the source address of a received MAC command Data Request frame matches LE_ADDR1 in LE_PEND_01 (0x01:18), the auto-transmitted ACK will have the PEND bit set.                                                                                                                                                                                                                              |
| LE2_PEND | reg:00:14 bit:12 | Data pending for device at LE2 address. When set and the source address of a received MAC command Data Request frame matches LE_ADDR2 in LE_PEND_23 (0x01:1C), the auto-transmitted ACK will have the PEND bit set.                                                                                                                                                                                                                              |
| LE3_PEND | reg:00:14 bit:13 | Data pending for device at LE3 address. When set and the source address of a received MAC command Data Request frame matches LE_ADDR3 in LE_PEND_23 (0x01:1C), the auto-transmitted ACK will have the PEND bit set.                                                                                                                                                                                                                              |
| SSADRAPE | reg:00:14 bit:14 | Short Source Address Data Request ACK with PEND Enable. When SSADRAPE = 1, any auto-ACK sent as a reply to any MAC command Data Request from any node with a short (16-bit) source address will have the PEND bit set. When SSADRAPE = 0, the auto-ACK will have the PEND bit set only if the address matches one of the four 16-bit addresses programmed in LE_PEND_01 and LE_PEND_23 and a matching bit in LE0_PEND–LE3_PEND is set.           |
| LSADRAPE | reg:00:14 bit:15 | Long Source Address Data Request ACK with PEND Enable. When LSADRAPE = 1, any auto-ACK sent as a reply to any MAC command Data Request from any node with a long (64-bit) source address will have the PEND bit set. When LSADRAPE = 0, such auto-ACKs will not have the PEND bit set.                                                                                                                                                           |

---

#### 8.2.2.6 Sub-register 0x00:18 – SPI CRC Read Status

| ID    | Length (octets) | Type | Mnemonic   | Description         |
| ----- | --------------- | ---- | ---------- | ------------------- |
| 00:18 | 1               | RO   | SPI_RD_CRC | SPI CRC read status |

Register file 0x00-0x01, sub-register 0x18 of register file 0x00 is the status register for the SPI read CRC value of the SPI CRC function. This is the CRC value resulting from each SPI read when SPI CRC mode is enabled via the SPI_CRCEN bit in SYS_CFG (0x00:10). For more details of SPI CRC operation see §2.3.1.3 – SPI CRC mode.

---

#### 8.2.2.7 Sub-register 0x00:1C – System Time Counter

| ID    | Length (octets) | Type | Mnemonic | Description                  |
| ----- | --------------- | ---- | -------- | ---------------------------- |
| 00:1C | 4               | RO   | SYS_TIME | System time counter (32-bit) |

Register file 0x00-0x01, sub-register 0x1C of register file 0x00 is the System Time Counter register. The device's system time and time stamps are designed to be based on time units nominally at 64 GHz, or more precisely 499.2 MHz × 128 = 63.8976 GHz.

The SYS_TIME register only counts in the **IDLE_PLL**, **RX** and **TX** states (when the digital PLL is enabled), and it only contains the 32 most significant bits of the device's system time. In all other states the system time counter is disabled and this register is not updated. When active the SYS_TIME register is incremented at a rate of 125 MHz and in units of 2. The least significant bit of SYS_TIME is always zero. The internal device's counter wrap period is 2⁴⁰ ÷ (128×499.2 MHz) = 17.2074 seconds.

> **Note:** Once this register is read the system time value is latched and any subsequent read will return the same value. To clear the current value in the register an SPI write transaction is necessary; the following read of the SYS_TIME register will return a new value.

---

#### 8.2.2.8 Sub-register 0x00:24 – Transmit Frame Control

| ID    | Length (octets) | Type | Mnemonic | Description            |
| ----- | --------------- | ---- | -------- | ---------------------- |
| 00:24 | 6               | RW   | TX_FCTRL | Transmit frame control |

Register file 0x00-0x01, sub-register 0x24 of register file 0x00 is the transmit frame control register. It contains a number of TX control fields. Each field is separately identified and described below.

**REG:00:24 – TX_FCTRL – Transmit Frame Control (Octets 0 to 3, 32-bits)**

| Bits  | Field        | Default |
| ----- | ------------ | ------- |
| 31:26 | - (reserved) | 0       |
| 25:16 | TXB_OFFSET   | 0       |
| 15:12 | TXPSR        | 0b0001  |
| 11    | TR           | 1       |
| 10    | TXBR         | 1       |
| 9:0   | TXFLEN       | 0       |

**REG:00:28 – TX_FCTRL – Transmit Frame Control (Octets 4 to 5, 16-bits)**

| Bits | Field        | Default |
| ---- | ------------ | ------- |
| 15:8 | FINE_PLEN    | 0       |
| 7:0  | - (reserved) | 0       |

| Field      | Bits                 | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| ---------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| TXFLEN     | reg:00:24 bits:9–0   | Transmit Frame Length. The DW3000 supports frame lengths up to 1023 bytes. IEEE802.15.4 standard frames can be up to 127 bytes long. The extended frame mode is enabled via the PHR_MODE bits of SYS_CFG (0x00:10). The value specified by TXFLEN determines the length of the data portion of the transmitted frame. This length includes the two-octet CRC appended automatically at the end of the frame, unless DIS_FCS_TX in SYS_CFG is set. The frame length is also copied into the PHY Header so that the receiving device knows how much data to decode.                                                                                                                                                             |
| TXBR       | reg:00:24 bit:10     | Transmit Bit Rate. Sets the bit rate for the data portion of the packet. 0 = 850 kb/s; 1 = 6.81 Mb/s.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| TR         | reg:00:24 bit:11     | Transmit Ranging enable. This bit has no operational effect on the DW3000; however it is copied into the ranging bit in the PHY header (PHR) of the transmitted packet, identifying it as a ranging frame. In the DW3000 receiver the time-stamping does not depend on or use the ranging bit in the received PHR.                                                                                                                                                                                                                                                                                                                                                                                                            |
| TXPSR      | reg:00:24 bits:15–12 | Transmit Preamble Symbol Repetitions (PSR). This sets the length of the transmitted preamble sequence in symbols. Each preamble symbol is approximately 1 μs in duration. The two TXPSR bits are copied into the PHY Header. There are four standard preamble lengths defined in 802.15.4 UWB PHY: 16, 64, 1024, and 4096 symbols. The DW3000 also supports non-standard intermediary lengths via this register. **Table 20: Preamble Length Selection:** TXPSR=`0001`→64; TXPSR=`0010`→1024; TXPSR=`0011`→4096; TXPSR=`0100`→32; TXPSR=`0101`→128; TXPSR=`0110`→1536; TXPSR=`1001`→256; TXPSR=`1010`→2048; TXPSR=`1101`→512. Other values are reserved. When auto ACK is used, the ACK frame PSR is set based on this field. |
| TXB_OFFSET | reg:00:24 bits:25–16 | Transmit buffer index offset. This 10-bit field specifies an index in the transmit buffer of the first octet to be transmitted. The TX frame begins at the TXB_OFFSET index and continues for TXFLEN minus 2 octets (less 2 for the CRC). Care should be taken that TXB_OFFSET plus the frame length does not extend past the end of TX_BUFFER. **Errata:** When TXB_OFFSET > 127, a value of 128 must be added (it will be internally subtracted by the device). The host must then execute a register read at address 0x08:00 (SAR control) after configuring this TX_FCTRL register.                                                                                                                                       |
| -          | reg:00:24 bits:31–26 | Reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| FINE_PLEN  | reg:00:28 bits:15–8  | Fine PSR control. When FINE_PLEN = 0, the preamble length configured in the TXPSR field is used. When FINE_PLEN is non-zero the preamble length is given by the expression: 8 × (FINE_PLEN + 1). For example, FINE_PLEN = 4 gives a PSR of 40 symbols. The maximum FINE_PLEN value 0xFF gives a PSR of 2048 symbols. This field allows fine tuning of preamble length to trade off message on-air time (and resultant power consumption) versus operational performance.                                                                                                                                                                                                                                                      |
| -          | reg:00:28 bits:7–0   | Reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |

---

#### 8.2.2.9 Sub-register 0x00:2C – Delayed Send or Receive Time

| ID    | Length (octets) | Type | Mnemonic | Description                           |
| ----- | --------------- | ---- | -------- | ------------------------------------- |
| 00:2C | 4               | RW   | DX_TIME  | Delayed send or receive time (32-bit) |

Register file 0x00-0x01, sub-register 0x2C of register file 0x00, the Delayed Send or Receive Time, is used to specify a time in the future to either turn on the receiver to be ready to receive a packet, or to turn on the transmitter and send a packet. The units are one half of the 499.2 MHz fundamental frequency (~4 ns). The least significant bit of this register is ignored, i.e. the smallest value that can be specified is 2, i.e. ~8 ns.

Delayed send can be initiated by any of the following commands: `CMD_DTX`, `CMD_DTX_TS`, `CMD_DTX_RS`, `CMD_DTX_W4R`, `CMD_DTX_TS_W4R`, or `CMD_DTX_RS_W4R`. Delayed receive can be initiated by: `CMD_DRX`, `CMD_DRX_TS`, or `CMD_DRX_RS`. For more information see sections 3.3 (Delayed transmission) and 4.3 (Delayed receive).

---

#### 8.2.2.10 Sub-register 0x00:30 – Delayed Send or Receive Reference Time

| ID    | Length (octets) | Type | Mnemonic  | Description                                     |
| ----- | --------------- | ---- | --------- | ----------------------------------------------- |
| 00:30 | 4               | RW   | DREF_TIME | Delayed send or receive reference time (32-bit) |

Register file 0x00-0x01, sub-register 0x30, the Delayed Send or Receive Reference Time, is used to specify a reference time (e.g. the time at which a Beacon was sent). Any value in DX_TIME is added to DREF_TIME before either the receiver or transmitter are turned on. The unit is one half of the 499.2 MHz fundamental frequency (~4 ns). The least significant bit of this register is ignored, i.e. the smallest specifiable value is 2 (~8 ns).

Delayed send with respect to reference time is initiated by `CMD_DTX_REF` or `CMD_DTX_REF_W4R`. Delayed receive with respect to reference time is initiated by `CMD_DRX_REF`. For more information see sections 3.3 (Delayed transmission) and 4.3 (Delayed receive).

---

#### 8.2.2.11 Sub-register 0x00:34 – Receive Frame Wait Timeout Period

| ID    | Length (octets) | Type | Mnemonic | Description                       |
| ----- | --------------- | ---- | -------- | --------------------------------- |
| 00:34 | 3               | RW   | RX_FWTO  | Receive frame wait timeout period |

Register file 0x00-0x01, sub-register 0x34 of register file 0x00 is the receive frame wait timeout period. It is a 20-bit wide register with a unit of 512/499.2 MHz (~1.0256 μs). The receive frame wait timeout function allows the host processor to enter a low power state awaiting a valid receive frame and be woken up by the DW3000 when either a frame is received or the programmed timeout has elapsed.

The frame wait timeout is enabled by the RXWTOE bit in SYS_CFG (0x00:10). When the receiver is enabled and RXWTOE is set, the frame wait timeout counter starts counting the programmed period. Thereafter:

- a) The Receive Frame Wait Timeout period elapses → the receiver is disabled and the RXFTO (Receiver Frame Wait Timeout) bit is set in SYS_STATUS, and the counter resets.
- b) A valid receive frame arrives and sets the RXFR and RXFCG bits in the status register → this stops the receive frame wait timer counter so RXFTO will not be set.
- c) The host can issue a transceiver off command (`CMD_TRXOFF`) at any time to stop the RX and disable RXFTO.

> **Note:** The RX frame wait timeout period should only be programmed when the IC is in **IDLE_PLL** state, before the receiver is enabled. Programming the RXFTO at other times (e.g. in RX state) may result in unpredictable behaviour.

---

#### 8.2.2.12 Sub-register 0x00:38 – System Control

| ID    | Length (octets) | Type | Mnemonic | Description    |
| ----- | --------------- | ---- | -------- | -------------- |
| 00:38 | 1               | RW   | SYS_CTRL | System control |

Register file 0x00-0x01, sub-register 0x38 of register file 0x00 is the system control register. This register is used when testing Continuous Frame test mode (see TX_PSTM in DIAG_TMC). Bit 0 needs to be set to 1 to start the transmissions.

---

#### 8.2.2.13 Sub-register 0x00:3C – System Event Enable Mask

| ID    | Length (octets) | Type | Mnemonic   | Description                       |
| ----- | --------------- | ---- | ---------- | --------------------------------- |
| 00:3C | 6               | RW   | SYS_ENABLE | System event enable mask register |

Register file 0x00-0x01, sub-register 0x3C of register file 0x00 is the system event mask register. These are aligned with the event status bits in the SYS_STATUS register. Whenever a bit in SYS_ENABLE is set (to 1) and the corresponding bit in SYS_STATUS is also set, then an interrupt will be generated asserting the hardware IRQ output line. The interrupt condition may be removed by clearing the corresponding bit in this SYS_ENABLE register (by setting it to 0) or by clearing the corresponding latched bit in the SYS_STATUS register (generally by writing a 1 to the bit).

**REG:00:3C – SYS_ENABLE – System Event Enable Mask (Octets 0 to 3)**

| Bits  | Field        | Default |
| ----- | ------------ | ------- |
| 31:30 | - (reserved) | 0       |
| 29    | ARFE_EN      | 0       |
| 28    | CPERR_EN     | 0       |
| 27    | HPDWARN_EN   | 0       |
| 26    | RXSTO_EN     | 0       |
| 25    | PLL_HILO_EN  | 0       |
| 24    | RCINIT_EN    | 0       |
| 23    | SPIRDY_EN    | 1       |
| 22    | - (reserved) | 0       |
| 21    | RXPTO_EN     | 0       |
| 20    | RXOVRR_EN    | 0       |
| 19    | VWARN_EN     | 0       |
| 18    | CIAERR_EN    | 0       |
| 17    | RXFTO_EN     | 0       |
| 16    | RXRFSL_EN    | 0       |
| 15    | RXFCE_EN     | 0       |
| 14    | RXFCG_EN     | 0       |
| 13    | RXFR_EN      | 0       |
| 12    | RXPHE_EN     | 0       |
| 11    | RXPHD_EN     | 0       |
| 10    | CIADONE_EN   | 0       |
| 9     | RXSFDD_EN    | 0       |
| 8     | RXPRD_EN     | 0       |
| 7     | TXFRS_EN     | 0       |
| 6     | TXPHS_EN     | 0       |
| 5     | TXPRS_EN     | 0       |
| 4     | TXFRB_EN     | 0       |
| 3     | AAT_EN       | 0       |
| 2     | SPICRCE_EN   | 0       |
| 1     | CPLOCK_EN    | 0       |
| 0     | - (reserved) | 0       |

**REG:00:40 – SYS_ENABLE – System Event Enable Mask (Octets 4 and 5)**

| Bits  | Field        | Default |
| ----- | ------------ | ------- |
| 15:13 | - (reserved) | 0       |
| 12    | CCA_FAIL_EN  | 1       |
| 11    | SPI_ERR_EN   | 1       |
| 10    | SPI_UNF_EN   | 1       |
| 9     | SPI_OVF_EN   | 1       |
| 8     | CDM_ERR_EN   | 0       |
| 7     | AES_ERR_EN   | 0       |
| 6     | AES_DONE_EN  | 0       |
| 5     | GPIOIRQ_EN   | 0       |
| 4     | VT_DET_EN    | 0       |
| 3:2   | - (reserved) | 0       |
| 1     | RXPREJ_EN    | 0       |
| 0     | - (reserved) | 0       |

| Field       | Bits             | Description                                                                                                                                                                                                                                                                       |
| ----------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| -           | reg:00:3C bit:0  | Reserved.                                                                                                                                                                                                                                                                         |
| CPLOCK_EN   | reg:00:3C bit:1  | Mask clock PLL lock event. When CPLOCK_EN = 1 and the CPLOCK event status bit is 1, the hardware IRQ interrupt line will be asserted to generate an interrupt.                                                                                                                    |
| SPICRCE_EN  | reg:00:3C bit:2  | Mask SPI CRC Error event. When SPICRCE_EN = 1 and the SPICRCE event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                            |
| AAT_EN      | reg:00:3C bit:3  | Mask automatic acknowledge trigger event. When AAT_EN = 1 and the AAT event status bit is 1, the hardware IRQ interrupt line will be asserted. **AAT should be masked when the automatic acknowledge is not enabled so that spurious interrupts cannot affect system behaviour.** |
| TXFRB_EN    | reg:00:3C bit:4  | Mask transmit frame begins event. When TXFRB_EN = 1 and the TXFRB event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                        |
| TXPRS_EN    | reg:00:3C bit:5  | Mask transmit preamble sent event. When TXPRS_EN = 1 and the TXPRS event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                       |
| TXPHS_EN    | reg:00:3C bit:6  | Mask transmit PHY Header Sent event. When TXPHS_EN = 1 and the TXPHS event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                     |
| TXFRS_EN    | reg:00:3C bit:7  | Mask transmit frame sent event. When TXFRS_EN = 1 and the TXFRS event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                          |
| RXPRD_EN    | reg:00:3C bit:8  | Mask receiver preamble detected event. When RXPRD_EN = 1 and the RXPRD event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                   |
| RXSFDD_EN   | reg:00:3C bit:9  | Mask receiver SFD detected event. When RXSFDD_EN = 1 and the RXSFDD event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                      |
| CIADONE_EN  | reg:00:3C bit:10 | Mask CIA processing done event. When CIADONE_EN = 1 and the CIADONE event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                      |
| RXPHD_EN    | reg:00:3C bit:11 | Mask receiver PHY header detect event. When RXPHD_EN = 1 and the RXPHD event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                   |
| RXPHE_EN    | reg:00:3C bit:12 | Mask receiver PHY header error event. When RXPHE_EN = 1 and the RXPHE event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                    |
| RXFR_EN     | reg:00:3C bit:13 | Mask receiver data frame ready event. When RXFR_EN = 1 and the RXFR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                      |
| RXFCG_EN    | reg:00:3C bit:14 | Mask receiver FCS good event. When RXFCG_EN = 1 and the RXFCG event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                            |
| RXFCE_EN    | reg:00:3C bit:15 | Mask receiver FCS error event. When RXFCE_EN = 1 and the RXFCE event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                           |
| RXRFSL_EN   | reg:00:3C bit:16 | Mask receiver Reed Solomon Frame Sync Loss event. When RXRFSL_EN = 1 and the RXRFSL event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                      |
| RXFTO_EN    | reg:00:3C bit:17 | Mask Receive Frame Wait Timeout event. When RXFTO_EN = 1 and the RXFTO event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                   |
| CIAERR_EN   | reg:00:3C bit:18 | Mask leading edge detection processing error event. When CIAERR_EN = 1 and the CIAERR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                    |
| VWARN_EN    | reg:00:3C bit:19 | Mask Voltage warning event. When VWARN_EN = 1 and the VWARN event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                              |
| RXOVRR_EN   | reg:00:3C bit:20 | Mask Receiver overrun event. When RXOVRR_EN = 1 and the RXOVRR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                           |
| RXPTO_EN    | reg:00:3C bit:21 | Mask Preamble detection timeout event. When RXPTO_EN = 1 and the RXPTO event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                   |
| -           | reg:00:3C bit:22 | Reserved.                                                                                                                                                                                                                                                                         |
| SPIRDY_EN   | reg:00:3C bit:23 | Mask SPI ready event. Default 1. When SPIRDY_EN = 1 and the SPIRDY event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                       |
| RCINIT_EN   | reg:00:3C bit:24 | Mask IDLE RC event. When RCINIT_EN = 1 and the RCINIT event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                                    |
| PLL_HILO_EN | reg:00:3C bit:25 | Mask PLL Losing Lock warning event. When PLL_HILO_EN = 1 and the PLL_HILO event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                |
| RXSTO_EN    | reg:00:3C bit:26 | Mask Receive SFD timeout event. When RXSTO_EN = 1 and the RXSTO event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                          |
| HPDWARN_EN  | reg:00:3C bit:27 | Mask Half Period Delay Warning event. When HPDWARN_EN = 1 and the HPDWARN event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                |
| CPERR_EN    | reg:00:3C bit:28 | Mask Scramble Timestamp Sequence (STS) error event. When CPERR_EN = 1 and the CPERR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                      |
| ARFE_EN     | reg:00:3C bit:29 | Mask Automatic Frame Filtering rejection event. When ARFE_EN = 1 and the ARFE event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                            |
| RXPREJ_EN   | reg:00:40 bit:1  | Mask Receiver Preamble Rejection event. When RXPREJ_EN = 1 and the RXPREJ event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                |
| VT_DET_EN   | reg:00:40 bit:4  | Mask Voltage/Temperature variation detection interrupt event. When VT_DET_EN = 1 and the VT_DET event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                          |
| GPIOIRQ_EN  | reg:00:40 bit:5  | Mask GPIO interrupt event. When GPIOIRQ_EN = 1 and the GPIOIRQ event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                           |
| AES_DONE_EN | reg:00:40 bit:6  | Mask AES done interrupt event. When AES_DONE_EN = 1 and the AES_DONE event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                     |
| AES_ERR_EN  | reg:00:40 bit:7  | Mask AES error interrupt event. When AES_ERR_EN = 1 and the AES_ERR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                      |
| CDM_ERR_EN  | reg:00:40 bit:8  | Mask CMD error interrupt event. When CDM_ERR_EN = 1 and the CMD_ERR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                      |
| SPI_OVF_EN  | reg:00:40 bit:9  | Mask SPI overflow interrupt event. When SPI_OVF_EN = 1 and the SPIOVF event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                    |
| SPI_UNF_EN  | reg:00:40 bit:10 | Mask SPI underflow interrupt event. When SPI_UNF_EN = 1 and the SPIUNF event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                   |
| SPI_ERR_EN  | reg:00:40 bit:11 | Mask SPI error interrupt event. When SPI_ERR_EN = 1 and the SPIERR event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                       |
| CCA_FAIL_EN | reg:00:40 bit:12 | Mask CCA fail interrupt event. When CCA_FAIL_EN = 1 and the CCA_FAIL event status bit is 1, the hardware IRQ interrupt line will be asserted.                                                                                                                                     |

---

#### 8.2.2.14 Sub-register 0x00:44 – System Event Status

| ID    | Length (octets) | Type | Mnemonic   | Description                  |
| ----- | --------------- | ---- | ---------- | ---------------------------- |
| 00:44 | 6               | SRW  | SYS_STATUS | System event status register |

Register file 0x00-0x01, sub-register 0x44 of register file 0x00 is the system event status register, SYS_STATUS. It contains status bits that indicate the occurrence of different system events or status changes. It is possible to enable particular events as interrupt sources by employing the SYS_ENABLE mask (0x00:3C), so that the setting of the event status bit will generate an interrupt, asserting the hardware IRQ output line.

Reading the SYS_STATUS register returns the state of the status bits. Generally these event status bits are latched so that the event is captured. Such latched bits need to be explicitly cleared by writing '1' to the bit position (writing '0' has no effect).

**REG:00:44 – SYS_STATUS – System Status Register (Octets 0 to 3)**

| Bits  | Field        |
| ----- | ------------ |
| 31:30 | - (reserved) |
| 29    | ARFE         |
| 28    | CPERR        |
| 27    | HPDWARN      |
| 26    | RXSTO        |
| 25    | PLL_HILO     |
| 24    | RCINIT       |
| 23    | SPIRDY       |
| 22    | - (reserved) |
| 21    | RXPTO        |
| 20    | RXOVRR       |
| 19    | VWARN        |
| 18    | CIAERR       |
| 17    | RXFTO        |
| 16    | RXFSL        |
| 15    | RXFCE        |
| 14    | RXFCG        |
| 13    | RXFR         |
| 12    | RXPHE        |
| 11    | RXPHD        |
| 10    | CIADONE      |
| 9     | RXSFDD       |
| 8     | RXPRD        |
| 7     | TXFRS        |
| 6     | TXPHS        |
| 5     | TXPRS        |
| 4     | TXFRB        |
| 3     | AAT          |
| 2     | SPICRCE      |
| 1     | CPLOCK       |
| 0     | IRQS         |

**REG:00:48 – SYS_STATUS – System Status Register (Octets 4 and 5)**

| Bits  | Field        |
| ----- | ------------ |
| 15:13 | - (reserved) |
| 12    | CCA_FAIL     |
| 11    | SPIERR       |
| 10    | SPI_UNF      |
| 9     | SPI_OVF      |
| 8     | CMD_ERR      |
| 7     | AES_ERR      |
| 6     | AES_DONE     |
| 5     | GPIOIRQ      |
| 4     | VT_DET       |
| 3:2   | - (reserved) |
| 1     | RXPREJ       |
| 0     | - (reserved) |

| Field    | Bits                   | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| -------- | ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| -        | reg:00:44 bits:various | Bits marked '-' are reserved and should not be changed.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| IRQS     | reg:00:44 bit:0        | Interrupt Request Status. READ ONLY status flag – it cannot be cleared or overwritten. Whenever a status bit in SYS_STATUS is activated (value 1) and the corresponding bit in SYS_ENABLE is enabled (value 1), then the IRQ interrupt request line will be driven to its active ON level. This IRQS flag reflects the overall status of interrupts. The polarity of the IRQ interrupt request line is controllable via the HIRQ_POL configuration bit in DIAG_TMC (0x0F:24).                                                                                                                                                                                                                                                                                                              |
| CPLOCK   | reg:00:44 bit:1        | Clock PLL Lock. The CPLOCK event status bit indicates that the digital clock PLL has locked. This may be used as an interrupt to indicate that the DW3000 clock is operating at full speed, after which the SPI can be run at its maximum rate. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| SPICRCE  | reg:00:44 bit:2        | SPI CRC Error. When SPI CRC mode is enabled (by the SPI_CRCEN bit in SYS_CFG), the SPICRCE event status bit indicates that a CRC error has been detected during an SPI write. Once set it remains set until cleared by writing a 1 to it. SPI Write CRC Error events are also counted in EVC_SWCE (0x0F:1A). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| AAT      | reg:00:44 bit:3        | Automatic Acknowledge Trigger. This status event bit is set when frame filtering is enabled and a data frame (or MAC command frame) is received (correctly addressed and with a good CRC) with the acknowledgement request bit set in its frame control field. If automatic acknowledgement is enabled (by AUTO_ACK in SYS_CFG), the AAT bit can be used during receive interrupt processing to detect that acknowledgement is in progress. **If automatic acknowledgement is not enabled, then the AAT status bit must be ignored.** Cleared by writing a 1 to it.                                                                                                                                                                                                                        |
| TXFRB    | reg:00:44 bit:4        | Transmit Frame Begins. This event status bit is set at the start of a transmission as the transmitter begins to send preamble. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| TXPRS    | reg:00:44 bit:5        | Transmit Preamble Sent. This event status bit is set at the end of preamble when SFD sending begins. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| TXPHS    | reg:00:44 bit:6        | Transmit PHY Header Sent. This event status bit is set when the PHR has been transmitted. This marks the start of sending the data part of the packet (assuming the frame length is non-zero) at the configured transmit data rate. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| TXFRS    | reg:00:44 bit:7        | Transmit Frame Sent. This event status bit is set at the end of sending the data part of the frame. It is expected that this will be used as the main "Transmit Done" (interrupt) event signalling the completion of frame transmission. (In the case where frame length is zero the TXFRS bit is set soon after the TXPHS event flag.) Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                      |
| RXPRD    | reg:00:44 bit:8        | Receiver Preamble Detected status. This event status bit is set to indicate that the receiver has detected (and confirmed) the presence of the preamble sequence. Preamble reception continues after RXPRD has been set until the SFD is detected (signalled by the RXSFDD event status bit) or an SFD timeout occurs (signalled by RXSTO). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                  |
| RXSFDD   | reg:00:44 bit:9        | Receiver SFD Detected. This event status bit is set to indicate that the receiver has detected the SFD sequence and is moving on to decode the PHR. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| CIADONE  | reg:00:44 bit:10       | CIA processing done. This event status bit is set to indicate the completion by the CIA algorithm of the leading edge detection and its other adjustments of the receive timestamp information. The resultant adjusted message RX timestamp is then available in RX_TIME (0x00:64). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| RXPHD    | reg:00:44 bit:11       | Receiver PHY Header Detect. This event status bit is set to indicate that the receiver has completed the decoding of the PHR. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| RXPHE    | reg:00:44 bit:12       | Receiver PHY Header Error. This event status bit is set to indicate that the receiver has found a non-correctable error in the PHR. The PHR includes a SECDED error check sequence that can correct a single bit error and detect a double bit error. Generally this error means that correct frame reception is not possible, and this event will abort reception. PHR Header Error events are also counted in EVC_PHE (0x0F:04). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                           |
| RXFR     | reg:00:44 bit:13       | Receiver Data Frame Ready. This event status bit is set to indicate the completion of the reception process. It is expected that this will be used as the main "Receive" (interrupt) event signalling the completion of a frame reception. The receive event processing routine should examine RXFCG and RXFCE to determine whether the frame has been received without error (or not), and check the CIADONE event status flag to validate the receive timestamp information. The setting of RXFR is delayed until the CIA adjustments of the timestamp have completed. **Note:** When using packet format configuration 3, this event should be used as the main "Receive" interrupt; RXFCG and RXFCE should not be used in packet format configuration 3. Cleared by writing a 1 to it. |
| RXFCG    | reg:00:44 bit:14       | Receiver FCS Good. This event status bit reflects the result of the frame CRC checking. It is set (or not) at the end of frame reception coincidentally with RXFR. When RXFCG = 1, the CRC check on the received data matches the 2-octet FCS sequence at the end of the frame. RXFR with RXFCG then indicates the correct reception of a valid frame. **Note:** This event should not be used in packet format configuration 3. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                             |
| RXFCE    | reg:00:44 bit:15       | Receiver FCS Error. This event status bit reflects the result of the frame CRC checking. It is set (or not) at the end of frame reception coincidentally with RXFR. When RXFCE = 1, the CRC check on the received data FAILED to match the 2-octet FCS sequence at the end of the frame. RXFCE events are also counted in EVC_FCE (0x0F:0A). **Note:** This event should not be used in packet format configuration 3. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                       |
| RXFSL    | reg:00:44 bit:16       | Receiver Reed Solomon Frame Sync Loss. The RXFSL event status bit is set to indicate that the receiver has found a non-correctable error during Reed Solomon decoding of the data portion of the packet. Generally this means that correct frame reception is not possible, and this event will abort frame reception. Reed Solomon Frame Sync Loss Error events are also counted in EVC_RSE (0x0F:06). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                      |
| RXFTO    | reg:00:44 bit:17       | Receive Frame Wait Timeout. This event status bit is set to indicate that a receive frame wait timeout has occurred. The receive frame wait timeout is enabled by RXWTOE in SYS_CFG (0x00:10), with the timeout set by RX_FWTO (0x00:34). Receive frame wait timeout events are also counted in EVC_FWTO (0x0F:14). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| CIAERR   | reg:00:44 bit:18       | Channel Impulse Response Analyser processing error. The CIA algorithm includes a failsafe watchdog timer that is initialized at the start of each CIR search. This watchdog can be disabled by clearing the CIA_WDEN bit in DIAG_TMC register, which is set by default. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| VWARN    | reg:00:44 bit:19       | Low voltage warning. This event indicates that the IC has detected that the voltage has dropped below the warning threshold of 1.5 V. The role of the low voltage detector is to detect drops in the supply during the higher current TX or RX modes where it could cause performance issues. The VWARN event flag stays set until cleared by writing a 1 to it, or the IC is reset. Low voltage events are also counted in EVC_VWARN (0x0F:2A). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                             |
| RXOVRR   | reg:00:44 bit:20       | Receiver Overrun. This event status bit only applies when double RX buffering is enabled (DIS_DRXB = 0 in SYS_CFG). The RXOVRR event flag is set to indicate that an overrun error has occurred in the receiver. The RXOVRR event status bit stays set until cleared by writing a 1 to it, or the IC is reset. Receiver Overrun events are also counted in EVC_OVR (0x0F:0E). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                |
| RXPTO    | reg:00:44 bit:21       | Preamble detection timeout. This event status bit is set when the preamble detection timeout occurs. The preamble detection timer starts when the receiver is enabled and begins preamble hunt. The preamble detection timeout value is programmed in DRX (0x06:04 – Preamble detection timeout count). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| -        | reg:00:44 bit:22       | Reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| SPIRDY   | reg:00:44 bit:23       | SPI ready for host access. This event status bit is set to indicate that the DW3000 has completed the activities associated with power on and has transitioned from **OFF** or awaking from **SLEEP** (or **DEEPSLEEP**) and is now in the **IDLE_RC** state. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| RCINIT   | reg:00:44 bit:24       | RC INIT. This event status bit is set to indicate that the DW3000 has completed the activities associated with power on and has transitioned from **OFF** or awaking from **SLEEP** (or **DEEPSLEEP**) and is now in the **INIT_RC** state. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| PLL_HILO | reg:00:44 bit:25       | Clock PLL Losing Lock. This event status bit is set to indicate that the system's digital clock PLL is having locking issues. This should not happen in healthy devices operating in their normal range. Its occurrence may indicate a bad configuration, a faulty part or a problem in the power or clock inputs to the device. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                             |
| RXSTO    | reg:00:44 bit:26       | Receive SFD timeout. This event status bit is set when the SFD detection timeout occurs. The SFD detection timer starts running as soon as preamble is detected. If the SFD sequence is not detected before the timeout period expires, the timeout will abort the reception currently in progress. The period of the SFD detection timeout is in DRX (0x06:02 – SFD detection timeout count). By default this has a value of 4096+64. SFD detection timeout events are also counted in EVC_STO (0x0F:10). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                   |
| HPDWARN  | reg:00:44 bit:27       | Half Period Delay Warning. This event status bit relates to the use of delayed transmit and delayed receive functionality. It indicates the delay is more than half a period of the system clock. The HPDWARN event status flag gets set if the time left to actually beginning transmission/reception is more than half a period of the system clock (SYS_TIME) away. Typically when HPDWARN is detected the host controller should abort the delayed TX/RX by issuing a transceiver off command (CMD_TRXOFF). HPDWARN events are counted in EVC_HPW (0x0F:18). Cleared by writing a 1 to it.                                                                                                                                                                                             |
| CPERR    | reg:00:44 bit:28       | Scramble Timestamp Sequence (STS) error. The CPERR event status flag will get set if the STS_TOAST bits are non-zero. CPERR events are counted in EVC_CPQE (0x0F:28 – STS quality error counter). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| ARFE     | reg:00:44 bit:29       | Automatic Frame Filtering rejection. The ARFE event status flag bit is set to indicate when a frame has been rejected in receiver due to it not passing through the frame filtering. Frame Filtering rejection events are also counted in EVC_FFR (0x0F:0C). Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| RXPREJ   | reg:00:48 bit:1        | Receiver Preamble Rejection. This is a low-level event status flag, probably not of interest to the host system. In the DW3000, preamble detection is a two stage process where preamble is initially seen and then has to be confirmed as continuing for a number of symbols before the RXSFDD event status bit actually gets set. If the preamble is not confirmed then the RXSFDD event status bit will not be set, but instead this RXPREJ status will be set. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                           |
| VT_DET   | reg:00:48 bit:4        | Voltage or temperature variation detected.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| GPIOIRQ  | reg:00:48 bit:5        | GPIO interrupt. The GPIOIRQ event status bit is set when an interrupt condition occurs in the GPIO block. The GPIO block may need to be interrogated to determine the source of the interrupt if more than one input line is configured to interrupt. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| AES_DONE | reg:00:48 bit:6        | AES-DMA operation complete. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| AES_ERR  | reg:00:48 bit:7        | AES-DMA error, indicates an AES authentication error or DMA transfer error or memory address conflict. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| CMD_ERR  | reg:00:48 bit:8        | Command error. Indicates that a fast command was programmed and will be ignored. This can happen when the host issues two commands in quick succession; if the device receives the second while the first has not completed, the device will ignore the second command and raise this event. Section 9 – Fast Commands describes the available commands. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                                                     |
| SPI_OVF  | reg:00:48 bit:9        | SPI overflow error. Occurs when data is written into the RX FIFO at too fast a rate and the FIFO overflows, possibly due to an SCLK frequency that is set much too fast.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| SPI_UNF  | reg:00:48 bit:10       | SPI underflow error. Occurs when the data to be read is not available in the TX FIFO possibly due to clocks not being turned on or a very high SCLK frequency.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| SPIERR   | reg:00:48 bit:11       | SPI collision error. If there is a case of a failed SPI transaction caused by internal contention the DW3000 will indicate this by the SPIERR event. The SPI_COLLISION register indicates which internal DW3000 block has conflicted with the host SPI access, and that the last host SPI transaction has not successfully completed.                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| CCA_FAIL | reg:00:48 bit:12       | CCA fail. This event will be set as a result of failure of CMD_CCA_TX to transmit a packet. When CMD_CCA_TX is invoked the device initially enters preamble hunt, and if no preamble is found within the time specified by PRE_TOC, the device will proceed to transmit the packet. However, if preamble is detected, then the device will go back to IDLE and report CCA_FAIL event. Cleared by writing a 1 to it.                                                                                                                                                                                                                                                                                                                                                                        |

---

#### 8.2.2.15 RX_FINFO — Receive Frame Information

| Attribute | Value                |
| --------- | -------------------- |
| Address   | `0x00:4C`            |
| Size      | 4 bytes              |
| Access    | RO (double-buffered) |

Provides information about a received frame, available after `RXFCG` or `RXFR` events.

| Bits    | Field  | Description                                                                                          |
| ------- | ------ | ---------------------------------------------------------------------------------------------------- |
| [31:20] | RXPACC | Preamble accumulation count. Slightly smaller than the TX preamble length due to detection overhead. |
| [19:18] | RXPSR  | Preamble symbol repetitions (decoded with RXNSPL, see Table 21).                                     |
| [17:16] | RXPRF  | Pulse repetition frequency: `00`=16 MHz, `01`=64 MHz.                                                |
| [15]    | RNG    | Ranging bit from the received frame.                                                                 |
| [14]    | —      | Reserved.                                                                                            |
| [13]    | RXBR   | Received data rate: `0`=850 kb/s, `1`=6.8 Mb/s.                                                      |
| [12:11] | RXNSPL | Non-standard preamble length indicator (see Table 21).                                               |
| [10]    | —      | Reserved.                                                                                            |
| [9:0]   | RXFLEN | Received frame length (octets, including 2-byte FCS).                                                |

**Table 21 — RXPSR + RXNSPL → Actual Preamble Length**

| RXNSPL[1:0] | RXPSR[1:0] | Preamble length |
| ----------- | ---------- | --------------- |
| `00`        | `00`       | 16              |
| `00`        | `01`       | 64              |
| `00`        | `10`       | 1024            |
| `00`        | `11`       | 4096            |
| `01`        | `00`       | 32              |
| `10`        | `00`       | 128             |
| `11`        | `00`       | 256             |
| `01`        | `10`       | 1536            |

> **Note:** To copy the received preamble length into a TX configuration, write `TXPSR = (RXNSPL << 2) | RXNSPL` into the TX_FCTRL register.

---

#### 8.2.2.16 RX_TIME — Receive Timestamp

| Attribute | Value                |
| --------- | -------------------- |
| Address   | `0x00:64`            |
| Size      | 16 bytes             |
| Access    | RO (double-buffered) |

> **CRITICAL:** Registers `0x64` through `0x73` **cannot** be read in a single SPI transaction. They must be read in separate transactions.

| Sub-address | Bits   | Field           | Description                           |
| ----------- | ------ | --------------- | ------------------------------------- |
| `0x00:64`   | [31:0] | RX_STAMP[31:0]  | Lower 32 bits of 40-bit RX timestamp. |
| `0x00:68`   | [7:0]  | RX_STAMP[39:32] | Upper 8 bits of 40-bit RX timestamp.  |
| `0x00:70`   | [31:0] | RX_RAWST        | 32-bit raw RX timestamp.              |

- **RX_STAMP:** 40-bit adjusted timestamp, LSB ≈ 15.65 ps (1 / (128 × 499.2 MHz)). Produced by the CIA after `CIADONE`. Represents the time of arrival of the RMARKER.
- **RX_RAWST:** 32-bit raw timestamp captured at the start of the PHR. Resolution is 125 MHz (~8 ns). Available before CIA processing completes.

---

#### 8.2.2.17 TX_TIME — Transmit Timestamp

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x00:74` |
| Size      | 5 bytes   |
| Access    | RO        |

| Sub-address | Bits   | Field           | Description                           |
| ----------- | ------ | --------------- | ------------------------------------- |
| `0x00:74`   | [31:0] | TX_STAMP[31:0]  | Lower 32 bits of 40-bit TX timestamp. |
| `0x00:78`   | [7:0]  | TX_STAMP[39:32] | Upper 8 bits of 40-bit TX timestamp.  |

- **TX_STAMP:** 40-bit timestamp, LSB ≈ 15.65 ps. Latched when the RMARKER of the transmitted frame passes through the RF antenna port. Available after PHR transmission completes (after `TXPHS` event).

---

#### 8.2.2.18 TX_RAWST — Transmit Raw Timestamp

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:00` |
| Size      | 4 bytes   |
| Access    | RO        |

32-bit register. Contains bits [39:8] of the TX_TIME value **prior** to the addition of the antenna delay (TX_ANTD). Useful for calibration purposes.

---

#### 8.2.2.19 TX_ANTD — Transmitter Antenna Delay

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:04` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits   | Field   | Description                             |
| ------ | ------- | --------------------------------------- |
| [15:0] | TX_ANTD | 16-bit transmitter antenna delay value. |

- Automatically added to TX_RAWST to produce the final TX_STAMP.
- **Default:** `0x4015` ≈ 256.74 ns.
- **LSB:** ~15.65 ps (same timescale as TX_STAMP and RX_STAMP).

---

#### 8.2.2.20 ACK_RESP_T — Acknowledgement / Response Time

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:08` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits    | Field   | Description                                                              |
| ------- | ------- | ------------------------------------------------------------------------ |
| [31:24] | ACK_TIM | 8-bit auto-ACK turnaround time, in preamble symbol units.                |
| [23:20] | —       | Reserved.                                                                |
| [19:0]  | W4R_TIM | 20-bit wait-for-response time, in ~1 μs units (128 system clock cycles). |

- **W4R_TIM:** Used with the `W4R` command modifier. Starts counting after the end of transmission. After the timer expires, the receiver is automatically enabled. Maximum ≈ 1.05 seconds.
- **ACK_TIM:** Minimum recommended values: `2` for 850 kb/s, `3` for 6.8 Mb/s. Must be programmed before enabling auto-ACK.

---

#### 8.2.2.21 TX_POWER — Transmitter Power Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:0C` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                 |
| ------- | -------- | --------------------------- |
| [31:24] | STS_PWR  | STS power level.            |
| [23:16] | SHR_PWR  | SHR (preamble) power level. |
| [15:8]  | PHR_PWR  | PHR power level.            |
| [7:0]   | DATA_PWR | Data payload power level.   |

Each power byte format:

| Bits  | Field  | Description                                                        |
| ----- | ------ | ------------------------------------------------------------------ |
| [7:2] | FINE   | Fine gain (6-bit, 0–63 in 0.5 dB steps).                           |
| [1:0] | COARSE | Coarse gain: `00`=Off, `01`=−18 dB, `10`=−12 dB, `11`=0 dB (full). |

- **Default:** `0x82828282` (all segments the same level).
- **CH9:** Maximum COARSE = `10` (−12 dB).
- **CH5:** Recommended COARSE = `10` (−12 dB).
- **BPRF PHR:** PHR is peak-power-limited. If `DATA_PWR = 0xFF`, set `PHR_PWR = 0xFC`.

---

#### 8.2.2.22 CHAN_CTRL — Channel Control

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:14` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                                             |
| ------- | -------- | ------------------------------------------------------- |
| [15:13] | —        | Reserved.                                               |
| [12:8]  | RX_PCODE | 5-bit RX preamble code (1–29).                          |
| [7:3]   | TX_PCODE | 5-bit TX preamble code (1–29).                          |
| [2:1]   | SFD_TYPE | SFD sequence type (see below).                          |
| [0]     | RF_CHAN  | RF channel: `0`=CH5 (6489.6 MHz), `1`=CH9 (7987.2 MHz). |

**Default:** `0x0006`

**SFD_TYPE values:**

| SFD_TYPE | Description                       |
| -------- | --------------------------------- |
| `00`     | IEEE 802.15.4 short (8 symbols)   |
| `01`     | Decawave proprietary (8 symbols)  |
| `10`     | Decawave proprietary (16 symbols) |
| `11`     | IEEE 802.15.4z (8 symbols)        |

**Preamble code ranges:**

| Code range | PRF                         |
| ---------- | --------------------------- |
| 1–8        | 16 MHz                      |
| 9–24       | 64 MHz                      |
| 25–29      | 64 MHz (BPRF/HPRF specific) |

TX_PCODE and RX_PCODE must match between devices for correct reception.

---

#### 8.2.2.23 LE_PEND_01 — Low-Energy Pending Addresses 0 and 1

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:18` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                                          |
| ------- | -------- | ---------------------------------------------------- |
| [31:16] | LE_ADDR1 | Short 16-bit address for low-energy pending entry 1. |
| [15:0]  | LE_ADDR0 | Short 16-bit address for low-energy pending entry 0. |

> **Note:** The corresponding `LE0_PEND` and `LE1_PEND` bits in the `FF_CFG` register must be set to enable these entries.

---

#### 8.2.2.24 LE_PEND_23 — Low-Energy Pending Addresses 2 and 3

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:1C` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                                          |
| ------- | -------- | ---------------------------------------------------- |
| [31:16] | LE_ADDR3 | Short 16-bit address for low-energy pending entry 3. |
| [15:0]  | LE_ADDR2 | Short 16-bit address for low-energy pending entry 2. |

> **Note:** The corresponding `LE2_PEND` and `LE3_PEND` bits in the `FF_CFG` register must be set to enable these entries.

---

#### 8.2.2.25 SPI_COLLISION — SPI Collision Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:20` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits  | Field         | Description                                                |
| ----- | ------------- | ---------------------------------------------------------- |
| [7:5] | —             | Reserved.                                                  |
| [4:0] | SPI_COLLISION | Indicates which DW3000 sub-system caused an SPI collision. |

**SPI_COLLISION bit values:**

| Bit | Value  | Subsystem that caused collision |
| --- | ------ | ------------------------------- |
| 4   | `0x10` | AON (Always-On) block           |
| 3   | `0x08` | CIA / ROM                       |
| 2   | `0x04` | CIA / RAM                       |
| 1   | `0x02` | Digital RX                      |
| 0   | `0x01` | Digital TX                      |

---

#### 8.2.2.26 RDB_STATUS — RX Double-Buffer Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:24` |
| Size      | 1 byte    |
| Access    | RW        |

| Bit | Field    | Description                               |
| --- | -------- | ----------------------------------------- |
| 7   | CP_ERR1  | STS error detected in buffer 1.           |
| 6   | CIADONE1 | CIA processing complete for buffer 1.     |
| 5   | RXFR1    | Frame received in buffer 1.               |
| 4   | RXFCG1   | Frame received with good CRC in buffer 1. |
| 3   | CP_ERR0  | STS error detected in buffer 0.           |
| 2   | CIADONE0 | CIA processing complete for buffer 0.     |
| 1   | RXFR0    | Frame received in buffer 0.               |
| 0   | RXFCG0   | Frame received with good CRC in buffer 0. |

All bits are sticky and cleared by writing `1` to the corresponding bit.

---

#### 8.2.2.27 RDB_DIAG — RX Double-Buffer Diagnostics Mode

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:28` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits  | Field     | Description                                       |
| ----- | --------- | ------------------------------------------------- |
| [7:3] | —         | Reserved.                                         |
| [2:0] | RDB_DMODE | Diagnostic data mode for double-buffer operation. |

**RDB_DMODE values:**

| Value | Mode    | Registers stored         |
| ----- | ------- | ------------------------ |
| `0x1` | Minimal | 7 diagnostic registers   |
| `0x2` | Medium  | 13 diagnostic registers  |
| `0x4` | Full    | All diagnostic registers |

---

#### 8.2.2.28 AES_CFG — AES Engine Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:30` |
| Size      | 2 bytes   |
| Access    | RW        |

| Bits    | Field    | Description                                                                                             |
| ------- | -------- | ------------------------------------------------------------------------------------------------------- |
| [15:13] | —        | Reserved.                                                                                               |
| [12]    | KEY_OTP  | Load key from OTP memory.                                                                               |
| [11]    | CORE_SEL | AES core selection: `0`=GCM, `1`=CCM*.                                                                  |
| [10:8]  | TAG_SIZE | Authentication tag size (see below).                                                                    |
| [7]     | KEY_SRC  | Key source: `0`=AES_KEY register, `1`=indirect via KEY_ADDR.                                            |
| [6]     | KEY_LOAD | Set to load key before AES operation. Must be set each time in CCM* mode, even if the key is unchanged. |
| [5:3]   | KEY_ADDR | Key address for indirect key selection.                                                                 |
| [2:1]   | KEY_SIZE | Key size: `0x0`=128-bit, `0x1`=192-bit, `0x2`=256-bit.                                                  |
| [0]     | MODE     | `0`=Encrypt, `1`=Decrypt.                                                                               |

**TAG_SIZE values:**

| TAG_SIZE | Tag length       |
| -------- | ---------------- |
| `0`      | 0 bytes (no tag) |
| `1`      | 4 bytes          |
| `2`      | 6 bytes          |
| `3`      | 8 bytes          |
| `4`      | 10 bytes         |
| `5`      | 12 bytes         |
| `6`      | 14 bytes         |
| `7`      | 16 bytes         |

> **Note:** `KEY_LOAD` must be set before each AES operation in CCM* mode, even when the key has not changed.

---

#### 8.2.2.29 AES_IV0 — AES Initialisation Vector Word 0

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:34` |
| Size      | 4 bytes   |
| Access    | RW        |

- **GCM mode:** 1st word of the initialisation vector.
- **CCM* mode:** Contains bytes `iv[10]`, `iv[9]`, `iv[8]`, `iv[7]` of the 13-byte nonce (`iv[10]` at LSB).

---

#### 8.2.2.30 AES_IV1 — AES Initialisation Vector Word 1

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:38` |
| Size      | 4 bytes   |
| Access    | RW        |

- **GCM mode:** 2nd word of the initialisation vector.
- **CCM* mode:** Contains bytes `iv[6]`, `iv[5]`, `iv[4]`, `iv[3]` of the 13-byte nonce (`iv[6]` at LSB).

---

#### 8.2.2.31 AES_IV2 — AES Initialisation Vector Word 2

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:3C` |
| Size      | 4 bytes   |
| Access    | RW        |

- **GCM mode:** 3rd word of the initialisation vector.
- **CCM* mode:** Contains bytes `iv[2]`, `iv[1]`, `iv[0]`, don't-care (`iv[2]` at LSB).

---

#### 8.2.2.32 AES_IV3 — AES CCM* Payload Length

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:40` |
| Size      | 2 bytes   |
| Access    | RW        |

AES-CCM* payload length field. Not used in GCM mode.

---

#### 8.2.2.33 AES_IV4 — AES CCM* Nonce Extension

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:42` |
| Size      | 2 bytes   |
| Access    | RW        |

AES-CCM* nonce bytes `iv[12]`, `iv[11]` (`iv[12]` at LSB). Not used in GCM mode.

---

#### 8.2.2.34 DMA_CFG — DMA Configuration

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:44` |
| Size      | 8 bytes   |
| Access    | RW        |

**Octets 0–3 (low word):**

| Bits    | Field      | Description                                        |
| ------- | ---------- | -------------------------------------------------- |
| [31:27] | —          | Reserved.                                          |
| [26]    | CP_END_SEL | Cipher-processing end selection.                   |
| [25:16] | DST_ADDR   | 10-bit destination address within the target port. |
| [15:13] | DST_PORT   | Destination port (see table below).                |
| [12:3]  | SRC_ADDR   | 10-bit source address within the source port.      |
| [2:0]   | SRC_PORT   | Source port (see table below).                     |

**Octets 4–7 (high word):**

| Bits    | Field     | Description            |
| ------- | --------- | ---------------------- |
| [23:17] | —         | Reserved.              |
| [16:7]  | PYLD_SIZE | Payload size in bytes. |
| [6:0]   | HDR_SIZE  | Header size in bytes.  |

**SRC_PORT / DST_PORT values:**

| Value | Port               |
| ----- | ------------------ |
| `0`   | AES scratch RAM    |
| `1`   | RX buffer 0        |
| `2`   | RX buffer 1        |
| `3`   | TX buffer          |
| `4`   | STS key (DST only) |

---

#### 8.2.2.35 AES_START — AES Operation Start

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:4C` |
| Size      | 1 byte    |
| Access    | RW        |

| Bits  | Field     | Description                                                                                               |
| ----- | --------- | --------------------------------------------------------------------------------------------------------- |
| [7:1] | —         | Reserved.                                                                                                 |
| [0]   | AES_START | Write `1` to start the AES operation. Cleared automatically by the AES core when the operation completes. |

---

#### 8.2.2.36 AES_STS — AES Status

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:50` |
| Size      | 4 bytes   |
| Access    | RW        |

| Bits   | Field     | Description                                     |
| ------ | --------- | ----------------------------------------------- |
| [31:6] | —         | Reserved.                                       |
| [5]    | RAM_FULL  | AES scratch RAM is full.                        |
| [4]    | RAM_EMPTY | AES scratch RAM is empty (default `1`).         |
| [3]    | MEM_CONF  | Memory configuration error.                     |
| [2]    | TRANS_ERR | Transfer error.                                 |
| [1]    | AUTH_ERR  | Authentication error (tag mismatch on decrypt). |
| [0]    | AES_DONE  | AES operation complete.                         |

All bits are sticky and cleared by writing `1` to the corresponding bit.

---

#### 8.2.2.37 AES_KEY — AES Key Register

| Attribute | Value     |
| --------- | --------- |
| Address   | `0x01:54` |
| Size      | 16 bytes  |
| Access    | RW        |

128-bit AES key for the GCM / CCM* core, stored as four consecutive 32-bit words:

| Offset  | Bits   | Description                          |
| ------- | ------ | ------------------------------------ |
| `+0x00` | [31:0] | Key word 0 (least significant word). |
| `+0x04` | [31:0] | Key word 1.                          |
| `+0x08` | [31:0] | Key word 2.                          |
| `+0x0C` | [31:0] | Key word 3 (most significant word).  |

> **Note:** Before starting an AES operation, ensure `KEY_LOAD` in `AES_CFG` is set so the key is loaded into the AES core. In CCM* mode this must be done every time, even if the key value has not changed.