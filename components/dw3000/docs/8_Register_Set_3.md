# Chapter 8 — Register Set (Part 3)

### 8.2.11 Register file: 0x0A – Always-on system control interface

| ID   | Length (octets) | Type | Mnemonic | Description                              |
| ---- | --------------- | ---- | -------- | ---------------------------------------- |
| 0x0A | 23              | -    | AON      | Always on system control interface block |

Register file 0x0A is the Always-On system control block (AON).

The AON block contains a low-power configuration array that remains powered-up as long as power (from the battery, for example) is supplied to the DW3000 via the VDD1 pin. User configurations, from SPI accessible host interface registers, can be automatically saved in the AON memory when the DW3000 enters **SLEEP** or **DEEPSLEEP** states and automatically restored from the AON memory when the DW3000 wakes from sleeping. Additional discussion of these modes may be found in section 2.5.1 – SLEEP and DEEPSLEEP.

This Register file controls the functions that remain on when the IC enters its low-power **SLEEP** or **DEEPSLEEP** states, and configures the activities the DW3000 should take as the IC wakes from these sleep states. An overview of the sub-registers is given by Table 36.

**Table 36: Register file: 0x0A – Always-on system control overview**

| OFFSET in Register 0x0A | Mnemonic    | Description                        |
| ----------------------- | ----------- | ---------------------------------- |
| 0x00                    | AON_DIG_CFG | AON wake up configuration register |
| 0x04                    | AON_CTRL    | AON control register               |
| 0x08                    | AON_RDATA   | AON direct access read data result |
| 0x0C                    | AON_ADDR    | AON direct access address          |
| 0x10                    | AON_WDATA   | AON direct access write data       |
| 0x14                    | AON_CFG     | AON configuration register         |

#### 8.2.11.1 Sub-register 0x0A:00 – AON on wake configuration

| ID    | Length (octets) | Type | Mnemonic    | Description                        |
| ----- | --------------- | ---- | ----------- | ---------------------------------- |
| 0A:00 | 3               | RW   | AON_DIG_CFG | AON wake up configuration register |

Register file: 0x0A – Always-on system control interface, sub-register 0x00 is a 24-bit configuration register that controls what the DW3000 IC does as it wakes up from low-power **SLEEP** or **DEEPSLEEP** states.

**REG:0A:00 – AON_DIG_CFG – AON wake-up configuration register**

| Field       | reg:addr             | Default | Description                                                                                                                                                                                                                                                                  |
| ----------- | -------------------- | ------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ONW_AON_DLD | 0A:00 bit:0          | 0       | On Wake-up download the AON array. When set to 1, configurations from AON memory are restored to user configuration registers. When 0, registers revert to power-on-reset values on wake from **SLEEP** or **DEEPSLEEP**.                                                    |
| ONW_RUN_SAR | 0A:00 bit:1          | 0       | On Wake-up Run the Analog-to-Digital Convertors. Initiates automatic temperature and input battery voltage measurements on wake from **SLEEP** or **DEEPSLEEP**. Note: For DW3000 only voltage can be read after wake up; temperature must be read via SAR_READING register. |
| ONW_GO2IDLE | 0A:00 bit:8          | 0       | On Wake-up go to **IDLE_PLL** state. Device automatically proceeds to **IDLE_PLL** from **IDLE_RC**.                                                                                                                                                                         |
| ONW_GO2RX   | 0A:00 bit:9          | 0       | On Wake-up go to **RX**. Device automatically proceeds from **IDLE_RC** to **IDLE_PLL** and then to **RX**.                                                                                                                                                                  |
| ONW_PGFCAL  | 0A:00 bit:11         | 0       | On Wake-up perform RX calibration. If not set, the host must manually perform RX calibration prior to enabling the receiver.                                                                                                                                                 |
| -           | 0A:00 bits:[various] | -       | Reserved. Should be left unchanged to avoid malfunction.                                                                                                                                                                                                                     |

#### 8.2.11.2 Sub-register 0x0A:04 – AON control register

| ID    | Length (octets) | Type | Mnemonic | Description          |
| ----- | --------------- | ---- | -------- | -------------------- |
| 0A:04 | 1               | RW   | AON_CTRL | AON control register |

Register file: 0x0A – Always-on system control interface, sub-register 0x04 is an 8-bit control register. The bits cause direct activity within the AON block with respect to the stored AON memory, acting like commands that are automatically cleared when the activity is taken.

**REG:0A:04 – AON_CTRL – AON Control Register**

| Field        | reg:addr    | Default | Description                                                                                                                                                                                                                                                                          |
| ------------ | ----------- | ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| RESTORE      | 0A:04 bit:0 | 0       | Copies user configurations from AON memory to the host interface register set. Auto-clears when executed.                                                                                                                                                                            |
| SAVE         | 0A:04 bit:1 | 0       | Copies user configurations from the host interface register set into the AON memory, then uploads AON block configurations. Auto-clears when executed.                                                                                                                               |
| CFG_UPLOAD   | 0A:04 bit:2 | 0       | Uploads the AON block configurations (from Sub-register 0x0A:14 – AON configuration register) into the AON configuration registers. Used to enter **SLEEP** state. Not self-clearing unless going to sleep — must be explicitly cleared immediately after use if not going to sleep. |
| DCA_READ     | 0A:04 bit:3 | 0       | Direct AON memory access read. When set (and DCA_ENAB is set), commands a direct read of the low-power configuration array. Address specified in AON_ADDR; result returned in AON_RDATA. Must be explicitly cleared after use.                                                       |
| DCA_WRITE    | 0A:04 bit:4 | 0       | Direct AON memory access write. When set (and DCA_ENAB is set), commands a direct write to the low-power configuration array. Address in AON_ADDR; data in AON_WDATA.                                                                                                                |
| DCA_WRITE_HI | 0A:04 bit:5 | 0       | Direct AON memory write for address > 0xFF. Must be set together with DCA_WRITE when address >= 0x100.                                                                                                                                                                               |
| -            | 0A:04 bit:6 | 0       | Reserved. Should always be written zero.                                                                                                                                                                                                                                             |
| DCA_ENAB     | 0A:04 bit:7 | 0       | Direct AON memory access enable. Must be set to 1 to enable DCA_READ. Must be reset to 0 to allow automatic saving/restoring of user configurations to/from AON memory during **SLEEP** and **DEEPSLEEP**.                                                                           |

#### 8.2.11.3 Sub-register 0x0A:08 – AON read data

| ID    | Length (octets) | Type | Mnemonic  | Description                        |
| ----- | --------------- | ---- | --------- | ---------------------------------- |
| 0A:08 | 1               | RW   | AON_RDATA | AON direct access read data result |

Register file: 0x0A – Always-on system control interface, sub-register 0x08 is an 8-bit register used to return the result of a direct access read of a location in the AON memory array. The location to read from is specified by AON_ADDR and the read is initiated using the DCA_READ control bit in AON_CTRL register.

**Reading from a specified address within AON memory (Figure 28 procedural flow):**

1. Write Address to AON_ADDR (Reg 0A:0C)
2. Set DCA_ENAB by writing 0x80 to AON_CTRL (Reg 0A:04)
3. Set DCA_READ by writing 0x88 to AON_CTRL (Reg 0A:04)
4. Read result data from AON_RDATA (Reg 0A:08)
5. Repeat steps 1–4 for additional reads if needed
6. Clear DCA_ENAB and DCA_READ by writing 0x00 to AON_CTRL (Reg 0A:04)

#### 8.2.11.4 Sub-register 0x0A:0C – AON memory address

| ID    | Length (octets) | Type | Mnemonic | Description               |
| ----- | --------------- | ---- | -------- | ------------------------- |
| 0A:0C | 2               | RW   | AON_ADDR | AON direct access address |

Register file: 0x0A – Always-on system control interface, sub-register 0x0C is a 16-bit register used to specify the address (9-bits) for a direct access read of the AON memory array. The read is initiated using DCA_READ in AON_CTRL and the result is returned in AON_RDATA.

#### 8.2.11.5 Sub-register 0x0A:10 – AON data to write

| ID    | Length (octets) | Type | Mnemonic  | Description                  |
| ----- | --------------- | ---- | --------- | ---------------------------- |
| 0A:10 | 1               | RW   | AON_WDATA | AON direct access write data |

Register file: 0x0A – Always-on system control interface, sub-register 0x10 is an 8-bit register used to provide the data for a direct write access of the AON memory array, to the address specified by AON_ADDR. The write is initiated using the DCA_WRITE control bit in AON_CTRL register.

**Writing to a specified address within AON memory (Figure 29 procedural flow):**

1. Write Data to AON_WDATA (Reg 0A:10)
2. Write 9 low bits of Address to AON_ADDR (Reg 0A:0C)
3. If Address >= 0x100: set DCA_WRITE_HI and DCA_WRITE by writing 0x30 to AON_CTRL (Reg 0A:04)
   If Address < 0x100: set DCA_WRITE by writing 0x10 to AON_CTRL (Reg 0A:04)
4. Set DCA_ENAB by writing 0x80 to AON_CTRL (Reg 0A:04)
5. Repeat steps 1–4 for additional writes if needed
6. Clear all bits by writing 0x00 to AON_CTRL (Reg 0A:04)

#### 8.2.11.6 Sub-register 0x0A:14 – AON configuration

| ID    | Length (octets) | Type | Mnemonic | Description                |
| ----- | --------------- | ---- | -------- | -------------------------- |
| 0A:14 | 1               | RW   | AON_CFG  | AON configuration register |

Register file: 0x0A – Always-on system control interface, sub-register 0x14 is an 8-bit configuration register for the always on block. The fields are interpreted inside the AON block, which can only happen after they are loaded via the CFG_UPLOAD command in AON_CTRL register.

**REG:0A:14 – AON_CFG – AON configuration register (default: 0b00001100)**

| Field      | reg:addr       | Default | Description                                                                                                                                                                                                                                                                                                                                      |
| ---------- | -------------- | ------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| SLEEP_EN   | 0A:14 bit:0    | 0       | Sleep enable. Must be set and configuration uploaded to AON (CFG_UPLOAD) to enter **SLEEP** or **DEEPSLEEP**. The SLEEP or DEEPSLEEP state can also be entered via ATX2SLP or ARX2SLP controls in Sub-register 0x11:08 – Sequencing control. If WAKE_CNT bit is disabled the device will enter **DEEPSLEEP**, otherwise it will enter **SLEEP**. |
| WAKE_CNT   | 0A:14 bit:1    | 0       | Wake when sleep counter elapses. Enables the sleep counter to bring the DW3000 out of **SLEEP** into operational mode. Default 0 (disabled). Setting to 1 means the sleep counter will wake the DW3000 from **SLEEP**.                                                                                                                           |
| BROUT_EN   | 0A:14 bit:2    | 1       | Set to 1 to enable the BROWNOUT detector during **SLEEP** or **DEEPSLEEP** (~6 µA). See also VWARN event in Sub-register 0x00:44 – System event status register.                                                                                                                                                                                 |
| WAKE_CSN   | 0A:14 bit:3    | 1       | Wake using SPI access (SPICSn). Default 1, enabling SPICSn as a wake-up signal. Setting to 0 means SPICSn cannot wake the DW3000 from **SLEEP** or **DEEPSLEEP**.                                                                                                                                                                                |
| WAKE_WUP   | 0A:14 bit:4    | 1       | Wake using **WAKEUP** pin. Default 1, enabling the WAKEUP line as a wake-up signal. Setting to 0 means the WAKEUP line cannot wake the DW3000 from **SLEEP** or **DEEPSLEEP**.                                                                                                                                                                   |
| PRES_SLEEP | 0A:14 bit:5    | 0       | Preserve Sleep. Determines what the DW3000 does after a wake-up event. When set to 1, the ATX2SLP/ARX2SLP sleep controls and SLEEP_EN are not cleared upon wake-up, allowing the device to be more easily (or automatically) returned to sleep.                                                                                                  |
| -          | 0A:14 bits:7,6 | -       | Reserved.                                                                                                                                                                                                                                                                                                                                        |

**Note:** Three mechanisms to wake the DW3000:
- Using the **WAKEUP** line when WAKE_WUP = 1
- Using **SPICSn** when WAKE_CSN = 1
- Using the sleep timer when WAKE_CNT = 1 and the sleep counter is enabled via the WAKE_CNT bit in AON_CFG
- Full chip reset

If none of these wake up mechanisms are enabled and the DW3000 is put into **DEEPSLEEP** state, a reset must be performed with RSTn pin or by removing power to the device.

**8.2.11.6.1 Configuration of sleep counter**

The sleep counter time (SLEEP_TIM) is 16-bits wide, representing the upper 16-bits of a 28-bit counter (the low order bit equals 4096 counts). For example, if the low power oscillator frequency is 20000 Hz, programming SLEEP_TIM with a value of 24 yields a sleep time of 24 × 4096 ÷ 20000 ≈ 4.92 seconds.

SLEEP_TIM is located in AON memory at address 0x102 (low 8-bits) and 0x103 (high 8-bits). To write data to this space see §8.2.11.5 above.

**Note:** When programming sleep counter time, the host must delay enabling the sleep counter by at least 32 µs following the programming of the timer duration for the newly programmed value to apply. Otherwise the counter will use the previously programmed value.

---

### 8.2.12 Register file: 0x0B – OTP memory interface

| ID   | Length (octets) | Type | Mnemonic | Description                            |
| ---- | --------------- | ---- | -------- | -------------------------------------- |
| 0x0B | 23              | -    | OTP_IF   | One Time Programmable memory interface |

Register file 0x0B is the OTP memory interface. This allows read access to parameters stored in the OTP memory, and it is also the interface via which parameters are programmed into the OTP memory. An overview of the sub-registers is given by Table 37.

**Note:** Programming OTP memory is a one-time only activity; any values programmed in error cannot be corrected. Also, please take care when programming OTP memory to only write to the designated areas – programming elsewhere may permanently damage the DW3000's ability to function normally. The OTP memory is programmed 32-bits at a time; programming of a single 8-bit, 16-bit or 24-bit word is not supported.

**Table 37: Register file: 0x0B – OTP memory interface overview**

| OFFSET in Register 0x0B | Mnemonic   | Description                         |
| ----------------------- | ---------- | ----------------------------------- |
| 0x00                    | OTP_WDATA  | OTP write data                      |
| 0x04                    | OTP_ADDR   | OTP address                         |
| 0x08                    | OTP_CFG    | OTP configuration                   |
| 0x0C                    | OTP_STAT   | OTP status                          |
| 0x10                    | OTP_RDATA  | OTP read data                       |
| 0x14                    | OTP_SRDATA | OTP Special Register (SR) read data |

#### 8.2.12.1 Sub-register 0x0B:00 – OTP data to program

| ID    | Length (octets) | Type | Mnemonic  | Description                                 |
| ----- | --------------- | ---- | --------- | ------------------------------------------- |
| 0B:00 | 4               | RW   | OTP_WDATA | OTP data to program to a particular address |

Register file: 0x0B – OTP memory interface, sub-register 0x00 is a 32-bit register. This register is used to configure the OTP memory block for memory writing operations and also to store the data value to be programmed into an OTP location. Writing to OTP memory is an involved procedure; for details please refer to section 7.3.2 – Programming a value into OTP memory.

#### 8.2.12.2 Sub-register 0x0B:04 – OTP programming address

| ID    | Length (octets) | Type | Mnemonic | Description                              |
| ----- | --------------- | ---- | -------- | ---------------------------------------- |
| 0B:04 | 2               | RW   | OTP_ADDR | OTP address to which to program the data |

Register file: 0x0B – OTP memory interface, sub-register 0x04 is a 16-bit register used to select the address within the OTP memory block that is being accessed (for read or write). The OTP_ADDR register contains the following fields:

**REG:2D:04 – OTP_ADDR – OTP address**

| Field    | reg:addr         | Description                                                                                                                                       |
| -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| OTP_ADDR | 0B:04 bits:10–0  | This 7-bit field specifies the address within OTP memory that will be accessed for read or write. See section 7.3 – Using the on-chip OTP memory. |
| -        | 0B:04 bits:15–11 | Reserved.                                                                                                                                         |

#### 8.2.12.3 Sub-register 0x0B:08 – OTP configuration

| ID    | Length (octets) | Type | Mnemonic | Description                |
| ----- | --------------- | ---- | -------- | -------------------------- |
| 0B:08 | 2               | RW   | OTP_CFG  | OTP configuration register |

Register file: 0x0B – OTP memory interface, sub-register 0x08 is a 16-bit register used to select and load special receiver operational parameter sets. See §8.2.12.7 – Receiver operating parameter sets for details.

**REG:0B:08 – OTP_SF – OTP configuration**

| Field        | reg:addr           | Default | Description                                                                                                                                                                                                                       |
| ------------ | ------------------ | ------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| OTP_MAN      | 0B:08 bit:0        | 0       | Enable manual control over OTP interface. See DW3000 APIs and section 7.3.                                                                                                                                                        |
| OTP_READ     | 0B:08 bit:1        | 0       | OTP read enable. See DW3000 APIs and section 7.3.                                                                                                                                                                                 |
| OTP_WRITE    | 0B:08 bit:2        | 0       | OTP write enable. See DW3000 APIs and section 7.3.                                                                                                                                                                                |
| OTP_WRITE_MR | 0B:08 bit:3        | 0       | OTP write mode. See DW3000 APIs and section 7.3.                                                                                                                                                                                  |
| DGC_KICK     | 0B:08 bit:6        | 0       | Initiates loading of the RX_TUNE_CAL parameter from OTP address 0x20–0x34 into the register RX_TUNE. Either channel 5 or channel 9 RX tune calibration data is loaded depending on the configuration of DGC_SEL.                  |
| LDO_KICK     | 0B:08 bit:7        | 0       | Initiates loading of the LDOTUNE_CAL parameter from OTP address 0x4 into Sub-register 0x07:40 – LDO voltage tune. See the section Waking from sleep for more details.                                                             |
| BIAS_KICK    | 0B:08 bit:8        | 0       | Initiates loading of the BIASTUNE_CAL parameter from OTP address 0xA into the BIAS_CTRL register. See the section Waking from sleep for more details.                                                                             |
| OPS_KICK     | 0B:08 bit:10       | 0       | Initiates a load of the operating parameter set selected by the OPS_SEL configuration.                                                                                                                                            |
| OPS_SEL      | 0B:08 bits:12,11   | 0       | Operating parameter set selection for OPS_KICK: 00 = Select parameter set 0 (Long), 01 = reserved, 10 = Select Default parameter set 2 (Short), 11 = Reserved (do not select). See §8.2.12.7 – Receiver operating parameter sets. |
| DGC_SEL      | 0B:08 bit:13       | 0       | RX_TUNE parameter set selection for DGC_KICK. If 0, channel 5 parameter set is selected; if 1, channel 9 parameter set is selected.                                                                                               |
| -            | 0B:08 bits:various | -       | Reserved. **N.B.: Any change in these bits may cause the DW3000 to malfunction.**                                                                                                                                                 |

#### 8.2.12.4 Sub-register 0x0B:0C – OTP programming status

| ID    | Length (octets) | Type | Mnemonic | Description                            |
| ----- | --------------- | ---- | -------- | -------------------------------------- |
| 0B:0C | 1               | RW   | OTP_STAT | OTP memory programming status register |

Register file: 0x0B – OTP memory interface, sub-register 0x0C is a 16-bit register used to give status information about the progress of the OTP programming activity.

**REG:0B:0C – OTP_STAT – OTP status**

| Field         | reg:addr           | Description                                                                                                                                               |
| ------------- | ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| OTP_PROG_DONE | 0B:0C bit:0        | OTP Programming Done. Indicates that programming of the 32-bit word from OTP_WDATA to the address specified by OTP_ADDR has completed. See section 7.3.2. |
| OTP_VPP_OK    | 0B:0C bit:1        | OTP Programming Voltage OK. Indicates the VPP level is sufficient for programming the OTP memory. See section 7.3.                                        |
| -             | 0B:0C bits:various | Reserved.                                                                                                                                                 |

#### 8.2.12.5 Sub-register 0x0B:10 – OTP data read from given address

| ID    | Length (octets) | Type | Mnemonic  | Description                      |
| ----- | --------------- | ---- | --------- | -------------------------------- |
| 0B:10 | 4               | R    | OTP_RDATA | OTP data read from given address |

Register file: 0x0B – OTP memory interface, sub-register 0x10 is a 32-bit register. The data value read from an OTP location will appear here after invoking the OTP read function. For details of the OTP memory map and procedures to read OTP memory, please refer to section 7.3 – Using the on-chip OTP memory.

#### 8.2.12.6 Sub-register 0x0B:14 – OTP special register

| ID    | Length (octets) | Type | Mnemonic   | Description                         |
| ----- | --------------- | ---- | ---------- | ----------------------------------- |
| 0B:14 | 4               | RW   | OTP_SRDATA | OTP Special Register (SR) read data |

Register file: 0x0B – OTP memory interface, sub-register 0x14 is a 32-bit register. The data value stored in the OTP SR (0x60) location will appear here after power up. For details of the OTP memory map and procedures to read OTP memory, please refer to section 7.3 – Using the on-chip OTP memory.

#### 8.2.12.7 Receiver operating parameter sets

The DW3000 receiver has the capability of operating with specific parameter sets that relate to how it acquires the preamble signal and decodes the data. Three distinct operating parameter sets are defined within the IC for selection by the host system designer depending on system characteristics.

**Table 38: Receiver operating parameter sets**

| Set                  | Description                                                                                                                                                        |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 10 – Default / Short | This is the default operating parameter set. Designed to give good performance for very short preambles (e.g. length 64 preamble). Not optimum for long preambles. |
| 00 – Long            | Designed to give good performance for long (>= 256) preambles (size including the SFD and STS length).                                                             |
| 01 – reserved        | Reserved.                                                                                                                                                          |

For most applications the default operating parameter set is the best choice.

---

### 8.2.13 Register files: 0x0C, 0x0D, 0x0E – CIA Interface

| ID          | Length (octets) | Type | Mnemonic | Description                                       |
| ----------- | --------------- | ---- | -------- | ------------------------------------------------- |
| 0x0C – 0x0E | -               | -    | CIA_IF   | Channel Impulse Response Analyser (CIA) Interface |

Register file 0x0C is the CIA control and status interface. The Channel Impulse Response Analyser (CIA) function is responsible for analysing the accumulator data (available through Register file: 0x15 – Accumulator CIR memory). Depending on the exact configuration, there are up to 3 channel impulse response (CIR) estimates in the accumulator memory: one based on the preamble code and two based on the STS sequence.

The CIA identifies the first path in any given CIR estimate. It can then use this to calculate the RX timestamp written to Sub-register 0x00:64 – Receive time stamp. An overview of the sub-registers is given by Table 39. 0x0C and 0x0D are CIA outputs; 0x0E is the CIA configuration control. The control and configuration of the STS is achieved via CP_CFG and SYS_CFG. Please also refer to §6 – Secure ranging / timestamping for details of the STS operation.

**Table 39: Register files: 0x0C, 0x0D, 0x0E – CIA Interface overview**

| Register file | OFFSET in Register | Mnemonic     | Description                                          |
| ------------- | ------------------ | ------------ | ---------------------------------------------------- |
| 0x0C          | 0x00               | IP_TS        | Preamble sequence receive time stamp and status      |
| 0x0C          | 0x08               | STS_TS       | STS receive time stamp and status                    |
| 0x0C          | 0x10               | STS1_TS      | 2nd STS receive time stamp and status                |
| 0x0C          | 0x18               | TDOA         | The TDoA between the two CIRs                        |
| 0x0C          | 0x1E               | PDOA         | The PDoA between the two CIRs                        |
| 0x0C          | 0x20               | CIA_DIAG_0   | CIA Diagnostic 0                                     |
| 0x0C          | 0x28               | IP_DIAG_0    | Preamble Diagnostic 0 – peak                         |
| 0x0C          | 0x2C               | IP_DIAG_1    | Preamble Diagnostic 1 – power indication             |
| 0x0C          | 0x30               | IP_DIAG_2    | Preamble Diagnostic 2 – magnitude @ FP + 1           |
| 0x0C          | 0x34               | IP_DIAG_3    | Preamble Diagnostic 3 – magnitude @ FP + 2           |
| 0x0C          | 0x38               | IP_DIAG_4    | Preamble Diagnostic 4 – magnitude @ FP + 3           |
| 0x0C          | 0x48               | IP_DIAG_8    | Preamble Diagnostic 8 – first path                   |
| 0x0C          | 0x58               | IP_DIAG_12   | Preamble Diagnostic 12 – symbols accumulated         |
| 0x0C          | 0x5C               | STS_DIAG_0   | STS Diagnostic 0 – STS CIR peak                      |
| 0x0C          | 0x60               | STS_DIAG_1   | STS 0 Diagnostic 1 – STS CIR power indication        |
| 0x0C          | 0x64               | STS_DIAG_2   | STS 0 Diagnostic 2 – STS CIR magnitude @ FP + 1      |
| 0x0C          | 0x68               | STS_DIAG_3   | STS 0 Diagnostic 3 – STS CIR magnitude @ FP + 2      |
| 0x0D          | 0x00               | STS_DIAG_4   | STS 0 Diagnostic 4 – STS CIR magnitude @ FP + 3      |
| 0x0D          | 0x10               | STS_DIAG_8   | STS 0 Diagnostic 8 – STS CIR first path              |
| 0x0D          | 0x20               | STS_DIAG_12  | STS 0 Diagnostic 12 – STS CIR accumulated STS length |
| 0x0D          | 0x38               | STS1_DIAG_0  | STS 1 Diagnostic 0 – STS CIR peak                    |
| 0x0D          | 0x3C               | STS1_DIAG_1  | STS 1 Diagnostic 1 – STS 1 power indication          |
| 0x0D          | 0x40               | STS1_DIAG_2  | STS 1 Diagnostic 2 – STS 1 magnitude @ FP + 1        |
| 0x0D          | 0x44               | STS1_DIAG_3  | STS 1 Diagnostic 3 – STS 1 magnitude @ FP + 2        |
| 0x0D          | 0x48               | STS1_DIAG_4  | STS 1 Diagnostic 4 – STS 1 magnitude @ FP + 3        |
| 0x0D          | 0x58               | STS1_DIAG_8  | STS 1 Diagnostic 8 – STS 1 first path                |
| 0x0D          | 0x68               | STS1_DIAG_12 | STS 1 Diagnostic 12 – STS 1 accumulated STS length   |
| 0x0E          | 0x00               | CIA_CONF     | CIA general configuration                            |
| 0x0E          | 0x04               | FP_CONF      | First path temp adjustment and thresholds            |
| 0x0E          | 0x0C               | IP_CONF      | Preamble Config 0 – CIA preamble configuration       |
| 0x0E          | 0x12               | STS_CONF_0   | STS Config 0 – CIA STS configuration                 |
| 0x0E          | 0x16               | STS_CONF_1   | STS Config 1 – CIA STS configuration                 |
| 0x0E          | 0x1A               | CIA_ADJUST   | User adjustment to the PDoA                          |

#### 8.2.13.1 Sub-register 0x0C:00 – Preamble receive time stamp and status

| ID    | Length (octets) | Type | Mnemonic | Description                            |
| ----- | --------------- | ---- | -------- | -------------------------------------- |
| 0C:00 | 8               | RO   | IP_TS    | Preamble receive time stamp and status |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x00 of register file 0x0C is an 8-octet register reporting the 40-bit preamble Time of Arrival estimate along with status diagnostic octet relating to it.

**REG:0C:00 – IP_TS – Preamble receive time stamp and status (Octets 0 to 3)**

| Bits | Field                                | Description                 |
| ---- | ------------------------------------ | --------------------------- |
| 31:0 | IP_TOA [low 32 bits of 40-bit value] | Low 32 bits of preamble TOA |

**REG:0C:04 – IP_TS – Preamble receive time stamp and status (Octets 4 to 7)**

| Bits  | Field                      | Description                        |
| ----- | -------------------------- | ---------------------------------- |
| 7:0   | IP_TOA [high 8 bits of 40] | High 8 bits of preamble TOA        |
| 21:8  | IP_POA                     | Phase of arrival from preamble CIR |
| 23:22 | -                          | Unused/reserved                    |
| 31:24 | IP_TOAST                   | Preamble Time of Arrival status    |

| Field    | reg:addr         | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| -------- | ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_TOA   | 0C:00 bits:39–0  | Preamble sequence Time of Arrival estimate. 40-bit (5-octet) fully adjusted time of reception estimated by the CIA algorithm using the preamble sequence CIR. LSB ≈ 15.65 ps (1/(128×499.2×10⁶) s). Available when CIADONE is set. Also written to RX_STAMP field in Sub-register 0x00:64; may be overwritten by STS_TOA if computed by the CIA algorithm.                                                                                                                                                                                            |
| IP_POA   | 0C:04 bits:21–8  | Phase of arrival from the preamble CIR. Adjusted by carrier recovery correction when the CIR is finalized. Used when implementing a two-chip PDoA system.                                                                                                                                                                                                                                                                                                                                                                                             |
| -        | 0C:04 bits:23–22 | Unused/reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| IP_TOAST | 0C:04 bits:31–24 | Preamble sequence Time of Arrival status indicator. Low order 5-bits are error flags indicating conditions where IP_TOA is not reliable and should not be used. Status word: [31] Reserved, [30] Reserved, [29] Reserved, [28] First path too close to the end to be plausible, [27] Coarse first path estimate too close to end to be plausible, [26] CIR too weak to get any estimate, [25] Noise threshold had to be artificially lowered to find any first path, [24] No strong rising edge on the first path so estimate is vulnerable to noise. |

#### 8.2.13.2 Sub-register 0x0C:08 – STS receive time stamp and status

| ID    | Length (octets) | Type | Mnemonic | Description                       |
| ----- | --------------- | ---- | -------- | --------------------------------- |
| 0C:08 | 8               | RO   | STS_TS   | STS receive time stamp and status |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x08 of register file 0x0C is an 8-octet register reporting the 40-bit STS based Time of Arrival estimate along with status diagnostic octet relating to it.

**REG:0C:08 – STS_TS – STS receive time stamp and status (Octets 0 to 3)**

| Bits | Field                                 | Description            |
| ---- | ------------------------------------- | ---------------------- |
| 31:0 | STS_TOA [low 32 bits of 40-bit value] | Low 32 bits of STS TOA |

**REG:0C:0C – STS_TS – STS receive time stamp and status (Octets 4 to 7)**

| Bits  | Field                       | Description                   |
| ----- | --------------------------- | ----------------------------- |
| 7:0   | STS_TOA [high 8 bits of 40] | High 8 bits of STS TOA        |
| 21:8  | STS_POA                     | Phase of arrival from STS CIR |
| 22    | -                           | Unused/reserved               |
| 31:23 | STS_TOAST                   | STS Time of Arrival status    |

| Field     | reg:addr         | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| --------- | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| STS_TOA   | 0C:08 bits:39–0  | STS Time of Arrival estimate. 40-bit field estimated by CIA using the STS sequence CIR. LSB ≈ 15.65 ps. Available when CIADONE is set. Also written to RX_STAMP, overwriting any existing IP_TOA value (STS CP_TOA takes precedence over IP_TOA).                                                                                                                                                                                                                                                                                                                                                                          |
| STS_POA   | 0C:0C bits:21–8  | Phase of arrival from the STS CIR, adjusted by carrier recovery correction when the CIR estimate is complete. Used in a two-chip PDoA system.                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| –         | 0C:0C bit:22     | Unused/reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| STS_TOAST | 0C:08 bits:31–23 | STS sequence Time of Arrival status. Non-zero means the CIR has failed one or more confidence tests; STS_TOA and STS_POA are not reliable and should not be used. Status bits from STS analysis: [31] STS_PGR_EN test fail, [30] STS_SS_EN test fail, [29] STS_CQ_EN test fail, [28] First path too close to the end to be plausible, [27] Coarse first path estimate too close to end to be plausible, [26] CIR too weak to get any estimate, [25] Noise threshold had to be artificially lowered to find any first path, [24] No strong rising edge on the first path so estimate is vulnerable to noise, [23] Reserved. |

#### 8.2.13.3 Sub-register 0x0C:10 – STS second RX time stamp and status

| ID    | Length (octets) | Type | Mnemonic | Description                                                              |
| ----- | --------------- | ---- | -------- | ------------------------------------------------------------------------ |
| 0C:10 | 8               | RO   | STS1_TS  | STS second receive time stamp and status (**valid in PDoA Mode 3 only**) |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x10 of register file 0x0C is an 8-octet register reporting the 40-bit STS second Time of Arrival estimate along with status diagnostic octet. This is only valid when operating in PDoA Mode 3.

**REG:0C:10 – STS1_TS – STS second receive time stamp and status (Octets 0 to 3)**

| Bits | Field                                  | Description             |
| ---- | -------------------------------------- | ----------------------- |
| 31:0 | STS1_TOA [low 32 bits of 40-bit value] | Low 32 bits of STS1 TOA |

**REG:0C:14 – STS1_TS – STS second receive time stamp and status (Octets 4 to 7)**

| Bits  | Field                        | Description                    |
| ----- | ---------------------------- | ------------------------------ |
| 7:0   | STS1_TOA [high 8 bits of 40] | High 8 bits of STS1 TOA        |
| 21:8  | STS1_POA                     | Phase of arrival from STS1 CIR |
| 22    | –                            | Unused/reserved                |
| 31:23 | STS1_TOAST                   | STS1 Time of Arrival status    |

| Field      | reg:addr         | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| ---------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| STS1_TOA   | 0C:10 bits:39–0  | STS second Time of Arrival estimate. 40-bit field. LSB ≈ 15.65 ps. Available when CIADONE is set.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| STS1_POA   | 0C:14 bits:21–8  | Phase of arrival from the STS based CIR estimate, adjusted by carrier recovery correction. Used in two-chip PDoA system.                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| –          | 0C:14 bit:22     | Unused/reserved.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| STS1_TOAST | 0C:14 bits:31–23 | STS second Time of Arrival status. Non-zero means the CIR has failed one or more confidence tests; STS1_TOA and STS1_POA are not reliable. Status bits: [31] STS_PGR_EN test fail, [30] STS_SS_EN test fail, [29] STS_CQ_EN test fail, [28] First path too close to the end to be plausible, [27] Coarse first path estimate too close to end to be plausible, [26] CIR too weak to get any estimate, [25] Noise threshold had to be artificially lowered to find any first path, [24] No strong rising edge on the first path so estimate is vulnerable to noise, [23] Reserved. |

#### 8.2.13.4 Sub-register 0x0C:18 – Time difference of arrival result

| ID    | Length (octets) | Type | Mnemonic | Description               |
| ----- | --------------- | ---- | -------- | ------------------------- |
| 0C:18 | 6               | RO   | TDOA     | The TDoA between two CIRs |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x18 of register file 0x0C is a 41-bit TDoA between two CIR TOAs. Bit 40 is a sign bit. If PDoA Mode 1 is selected (see PDOA_MODE), this TDoA refers to the TDoA between IP_TOA and STS_TOA. In PDoA Mode 3 this is the TDoA between STS_TOA and STS1_TOA. All three CIRs are estimates of the same (or very similar) channel so the TDoA should be small. Its value can help when determining how reliable the PDoA/ToA estimate is. TDoA is described in 4.2 – TDoA and PDoA support.

#### 8.2.13.5 Sub-register 0x0C:1E – Phase difference of arrival result

| ID    | Length (octets) | Type | Mnemonic | Description                               |
| ----- | --------------- | ---- | -------- | ----------------------------------------- |
| 0C:1E | 2               | RO   | PDOA     | PDoA and First Path (FP) threshold status |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x1E of register file 0x0C is a 2-octet register reporting the phase difference of arrival. Valid only when operating in one of the PDoA modes (PDOA_MODE).

**REG:0C:1E – PDOA – PDoA result**

| Field    | reg:addr        | Description                                                                                                                              |
| -------- | --------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| PDOA     | 0C:1E bits:13–0 | Phase difference result. Further described in 4.2 TDoA and PDoA support.                                                                 |
| FP_TH_MD | 0C:1E bit:14    | First path threshold test mode. 0 → absolute difference of two first paths is ≤ FP_AGREED_TH; 1 → absolute difference is > FP_AGREED_TH. |
| –        | 0C:1E bit:15    | Unused/reserved.                                                                                                                         |

#### 8.2.13.6 Sub-register 0x0C:20 – CIA diagnostic 0

| ID    | Length (octets) | Type | Mnemonic   | Description      |
| ----- | --------------- | ---- | ---------- | ---------------- |
| 0C:20 | 4               | RO   | CIA_DIAG_0 | CIA diagnostic 0 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x20 of register file 0x0C is a 4-octet register reporting CIA diagnostics after CIR analysis.

**REG:0C:20 – CIA_DIAG_0 – CIA diagnostic 0**

| Field   | reg:addr         | Description                                                                                                                                                                                                                                               |
| ------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| COE_PPM | 0C:20 bits:12–0  | Clock offset estimate. 13-bit field reporting an estimate of the clock offset of the remote transmitting device using an s[-15:-26] fixed point. Available when CIADONE is set. To get PPM: divide the integer 13-bit value by 2^26 and multiply by 10^6. |
| –       | 0C:20 bits:31–13 | Unused/reserved.                                                                                                                                                                                                                                          |

#### 8.2.13.7 Sub-register 0x0C:24 – Reserved

| ID    | Length (octets) | Type | Mnemonic   | Description              |
| ----- | --------------- | ---- | ---------- | ------------------------ |
| 0C:24 | 4               | RO   | CIA_DIAG_1 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x24 of register file 0x0C is a 4-octet reserved register.

#### 8.2.13.8 Sub-register 0x0C:28 – Preamble diagnostic 0

| ID    | Length (octets) | Type | Mnemonic  | Description                  |
| ----- | --------------- | ---- | --------- | ---------------------------- |
| 0C:28 | 4               | RO   | IP_DIAG_0 | Preamble diagnostic 0 – peak |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x28 of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:28 – IP_DIAG_0 – Preamble diagnostic 0 – peak**

| Field    | reg:addr         | Description                                                                                                                                                          |
| -------- | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_PEAKA | 0C:28 bits:20–0  | 21-bit field reporting the amplitude of the sample in the CIR accumulated using the preamble sequence that has the largest amplitude. Available when CIADONE is set. |
| IP_PEAKI | 0C:28 bits:30–21 | 10-bit field reporting the index of the sample in the CIR accumulated using the preamble sequence that has the largest amplitude. Available when CIADONE is set.     |
| -        | 0C:28 bit:31     | Reserved.                                                                                                                                                            |

#### 8.2.13.9 Sub-register 0x0C:2C – Preamble diagnostic 1

| ID    | Length (octets) | Type | Mnemonic  | Description                              |
| ----- | --------------- | ---- | --------- | ---------------------------------------- |
| 0C:2C | 4               | RO   | IP_DIAG_1 | Preamble diagnostic 1 – power indication |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x2C of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:2C – IP_DIAG_1 – Preamble diagnostic 1 – power indication**

| Field    | reg:addr         | Description                                                                                                                                                                      |
| -------- | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_CAREA | 0C:2C bits:16–0  | 17-bit field reporting the channel area in the CIR accumulated using the preamble sequence. Gives a measure of the receive power of the preamble. Available when CIADONE is set. |
| -        | 0C:2C bits:31–17 | Reserved.                                                                                                                                                                        |

#### 8.2.13.10 Sub-register 0x0C:30 – Preamble diagnostic 2

| ID    | Length (octets) | Type | Mnemonic  | Description                                |
| ----- | --------------- | ---- | --------- | ------------------------------------------ |
| 0C:30 | 4               | RO   | IP_DIAG_2 | Preamble diagnostic 2 – magnitude @ FP + 1 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x30 of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:30 – IP_DIAG_2 – Preamble diagnostic 2 – magnitude @ FP + 1**

| Field   | reg:addr         | Description                                                                                                                                                                                                  |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| IP_FP1M | 0C:30 bits:21–0  | 22-bit field reporting the magnitude of the sample at the first index immediately after the estimated first path position in the CIR accumulated using the preamble sequence. Available when CIADONE is set. |
| -       | 0C:30 bits:31–22 | Reserved.                                                                                                                                                                                                    |

#### 8.2.13.11 Sub-register 0x0C:34 – Preamble diagnostic 3

| ID    | Length (octets) | Type | Mnemonic  | Description                                |
| ----- | --------------- | ---- | --------- | ------------------------------------------ |
| 0C:34 | 4               | RO   | IP_DIAG_3 | Preamble diagnostic 3 – magnitude @ FP + 2 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x34 of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:34 – IP_DIAG_3 – Preamble diagnostic 3 – magnitude @ FP + 2**

| Field   | reg:addr         | Description                                                                                                                                                                                   |
| ------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_FP2M | 0C:34 bits:21–0  | 22-bit field reporting the magnitude of the sample at second index after the estimated first path position in the CIR accumulated using the preamble sequence. Available when CIADONE is set. |
| -       | 0C:34 bits:31–22 | Reserved.                                                                                                                                                                                     |

#### 8.2.13.12 Sub-register 0x0C:38 – Preamble diagnostic 4

| ID    | Length (octets) | Type | Mnemonic  | Description                                |
| ----- | --------------- | ---- | --------- | ------------------------------------------ |
| 0C:38 | 4               | RO   | IP_DIAG_4 | Preamble Diagnostic 4 – magnitude @ FP + 3 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x38 of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:38 – IP_DIAG_4 – Preamble diagnostic 4 – magnitude @ FP + 3**

| Field   | reg:addr         | Description                                                                                                                                                                                  |
| ------- | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_FP3M | 0C:38 bits:21–0  | 22-bit field reporting the magnitude of the sample at third index after the estimated first path position in the CIR accumulated using the preamble sequence. Available when CIADONE is set. |
| -       | 0C:38 bits:31–22 | Reserved.                                                                                                                                                                                    |

#### 8.2.13.13 Sub-register 0x0C:3C – Reserved

| ID    | Length (octets) | Type | Mnemonic     | Description              |
| ----- | --------------- | ---- | ------------ | ------------------------ |
| 0C:3C | 12              | RO   | IP_DIAG_RES1 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x3C of register file 0x0C is a 12-octet reserved register.

#### 8.2.13.14 Sub-register 0x0C:48 – Preamble diagnostic 8

| ID    | Length (octets) | Type | Mnemonic  | Description                               |
| ----- | --------------- | ---- | --------- | ----------------------------------------- |
| 0C:48 | 4               | RO   | IP_DIAG_8 | Preamble diagnostic 8 – first path result |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x48 of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:48 – IP_DIAG_8 – Preamble diagnostic 8 – first path**

| Field | reg:addr         | Description                                                                                                                                                                                                                                                                                                               |
| ----- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_FP | 0C:48 bits:15–0  | 16-bit field reporting the estimated first path location within the CIR accumulated using the preamble sequence. [10.6] fixed point format (10-bit unsigned integer, 6-bit fractional). Index relative to start of preamble CIR within the accumulator (beginning at accumulator index 0). Available when CIADONE is set. |
| -     | 0C:48 bits:31–16 | Reserved.                                                                                                                                                                                                                                                                                                                 |

#### 8.2.13.15 Sub-register 0x0C:4C – Reserved

| ID    | Length (octets) | Type | Mnemonic     | Description              |
| ----- | --------------- | ---- | ------------ | ------------------------ |
| 0C:4C | 12              | RO   | IP_DIAG_RES2 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x4C of register file 0x0C is a 12-octet reserved register.

#### 8.2.13.16 Sub-register 0x0C:58 – Preamble diagnostic 12

| ID    | Length (octets) | Type | Mnemonic   | Description                         |
| ----- | --------------- | ---- | ---------- | ----------------------------------- |
| 0C:58 | 4               | RO   | IP_DIAG_12 | Diagnostic 12 – symbols accumulated |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x58 of register file 0x0C is a 4-octet register reporting diagnostics relating to the preamble sequence.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:58 – IP_DIAG_12 – Preamble diagnostic 12 – symbols accumulated**

| Field   | reg:addr         | Description                                                                                                                          |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| IP_NACC | 0C:58 bits:11–0  | 12-bit field reporting the number of preamble sequence symbols accumulated to form the preamble CIR. Available after CIADONE is set. |
| -       | 0C:58 bits:31–12 | Reserved.                                                                                                                            |

#### 8.2.13.17 Sub-register 0x0C:5C – STS diagnostic 0

| ID    | Length (octets) | Type | Mnemonic   | Description                               |
| ----- | --------------- | ---- | ---------- | ----------------------------------------- |
| 0C:5C | 4               | RO   | STS_DIAG_0 | STS Diagnostic 0 – STS CIA peak amplitude |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x5C of register file 0x0C is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:5C – STS_DIAG_0 – STS diagnostic 0 – STS CIA peak amplitude**

| Field    | reg:addr         | Description                                                                                                                                            |
| -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| CP_PEAKA | 0C:5C bits:20–0  | 21-bit field reporting the amplitude of the sample in the CIR accumulated using the STS that has the largest amplitude. Available when CIADONE is set. |
| CP_PEAKI | 0C:5C bits:29–21 | 9-bit field reporting the index of the sample in the CIR accumulated using the STS that has the largest amplitude. Available when CIADONE is set.      |
| -        | 0C:5C bits:31,30 | Reserved.                                                                                                                                              |

#### 8.2.13.18 Sub-register 0x0C:60 – STS 0 diagnostic 1

| ID    | Length (octets) | Type | Mnemonic   | Description                             |
| ----- | --------------- | ---- | ---------- | --------------------------------------- |
| 0C:60 | 4               | RO   | STS_DIAG_1 | STS Diagnostic 1 – STS power indication |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x60 of register file 0x0C is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:60 – STS_DIAG_1 – STS diagnostic 1 – STS power indication**

| Field    | reg:addr         | Description                                                                                                                                                   |
| -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_CAREA | 0C:60 bits:15–0  | 16-bit field reporting the channel area in the CIR accumulated using the STS. Gives a measure of the receive power of the STS. Available when CIADONE is set. |
| -        | 0C:60 bits:31–16 | Reserved.                                                                                                                                                     |

#### 8.2.13.19 Sub-register 0x0C:64 – STS 0 diagnostic 2

| ID    | Length (octets) | Type | Mnemonic   | Description                               |
| ----- | --------------- | ---- | ---------- | ----------------------------------------- |
| 0C:64 | 4               | RO   | STS_DIAG_2 | STS Diagnostic 2 – STS magnitude @ FP + 1 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x64 of register file 0x0C is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:64 – STS_DIAG_2 – STS diagnostic 2 – STS magnitude @ FP + 1**

| Field   | reg:addr         | Description                                                                                                                                                                                    |
| ------- | ---------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_FP1M | 0C:64 bits:21–0  | 22-bit field reporting the magnitude of the sample at the first index immediately after the estimated first path position in the CIR accumulated using the STS. Available when CIADONE is set. |
| -       | 0C:64 bits:31–22 | Reserved.                                                                                                                                                                                      |

#### 8.2.13.20 Sub-register 0x0C:68 – STS 0 diagnostic 3

| ID    | Length (octets) | Type | Mnemonic   | Description                               |
| ----- | --------------- | ---- | ---------- | ----------------------------------------- |
| 0C:68 | 4               | RO   | STS_DIAG_3 | STS Diagnostic 3 – STS magnitude @ FP + 2 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x68 of register file 0x0C is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0C:68 – STS_DIAG_3 – STS diagnostic 3 – STS magnitude @ FP + 2**

| Field   | reg:addr         | Description                                                                                                                                                                     |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_FP2M | 0C:68 bits:21–0  | 22-bit field reporting the magnitude of the sample at second index after the estimated first path position in the CIR accumulated using the STS. Available when CIADONE is set. |
| -       | 0C:68 bits:31–22 | Reserved.                                                                                                                                                                       |

#### 8.2.13.21 Sub-register 0x0D:00 – STS 0 diagnostic 4

| ID    | Length (octets) | Type | Mnemonic   | Description                               |
| ----- | --------------- | ---- | ---------- | ----------------------------------------- |
| 0D:00 | 4               | RO   | STS_DIAG_4 | STS diagnostic 4 – STS magnitude @ FP + 3 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x00 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:00 – STS_DIAG_4 – STS diagnostic 4 – STS magnitude @ FP + 3**

| Field   | reg:addr         | Description                                                                                                                                                                    |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| CP_FP3M | 0D:00 bits:21–0  | 22-bit field reporting the magnitude of the sample at third index after the estimated first path position in the CIR accumulated using the STS. Available when CIADONE is set. |
| -       | 0D:00 bits:31–22 | Reserved.                                                                                                                                                                      |

#### 8.2.13.22 Sub-register 0x0D:04 – Reserved

| ID    | Length (octets) | Type | Mnemonic      | Description              |
| ----- | --------------- | ---- | ------------- | ------------------------ |
| 0D:04 | 12              | RO   | STS_DIAG_RES1 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x04 of register file 0x0D is a 12-octet reserved register.

#### 8.2.13.23 Sub-register 0x0D:10 – STS 0 diagnostic 8

| ID    | Length (octets) | Type | Mnemonic   | Description                       |
| ----- | --------------- | ---- | ---------- | --------------------------------- |
| 0D:10 | 4               | RO   | STS_DIAG_8 | STS Diagnostic 8 – STS first path |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x10 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:10 – STS_DIAG_8 – STS diagnostic 8 – STS first path**

| Field | reg:addr         | Description                                                                                                                                                                                                                                                                                             |
| ----- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_FP | 0D:10 bits:14–0  | 15-bit field reporting the estimated first path location within the CIR accumulated using the STS. [9.6] fixed point format (9-bit unsigned integer, 6-bit fractional). Index relative to start of STS CIR within the accumulator (beginning at accumulator index 1024). Available when CIADONE is set. |
| -     | 0D:10 bits:31–15 | Reserved.                                                                                                                                                                                                                                                                                               |

#### 8.2.13.24 Sub-register 0x0D:14 – Reserved

| ID    | Length (octets) | Type | Mnemonic      | Description              |
| ----- | --------------- | ---- | ------------- | ------------------------ |
| 0D:14 | 12              | RO   | STS_DIAG_RES2 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x14 of register file 0x0D is a 12-octet reserved register.

#### 8.2.13.25 Sub-register 0x0D:20 – STS 0 diagnostic 12

| ID    | Length (octets) | Type | Mnemonic    | Description                                |
| ----- | --------------- | ---- | ----------- | ------------------------------------------ |
| 0D:20 | 4               | RO   | STS_DIAG_12 | STS diagnostic 12 – accumulated STS length |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x20 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:20 – STS_DIAG_12 – STS diagnostic 12 – accumulated STS length**

| Field   | reg:addr         | Description                                                                                                                             |
| ------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| CP_NACC | 0D:20 bits:10–0  | 11-bit field reporting the length of STS accumulated in units of 512 chips (~1 µs) to form the STS CIR. Available after CIADONE is set. |
| -       | 0D:20 bits:31–11 | Reserved.                                                                                                                               |

#### 8.2.13.26 Sub-register 0x0D:24 – Reserved

| ID    | Length (octets) | Type | Mnemonic      | Description              |
| ----- | --------------- | ---- | ------------- | ------------------------ |
| 0D:24 | 20              | RO   | STS_DIAG_RES3 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x24 of register file 0x0D is a 20-octet reserved register.

#### 8.2.13.27 Sub-register 0x0D:38 – STS 1 diagnostic 0

| ID    | Length (octets) | Type | Mnemonic    | Description                         |
| ----- | --------------- | ---- | ----------- | ----------------------------------- |
| 0D:38 | 4               | RO   | STS1_DIAG_0 | STS 1 diagnostic 0 – peak amplitude |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x38 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS where it is present and being used to measure the PDOA. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:38 – STS1_DIAG_0 – STS 1 diagnostic 0 – peak amplitude**

| Field    | reg:addr         | Description                                                                                                                                            |
| -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| CP_PEAKA | 0D:38 bits:20–0  | 21-bit field reporting the amplitude of the sample in the CIR accumulated using the STS that has the largest amplitude. Available when CIADONE is set. |
| CP_PEAKI | 0D:38 bits:29–21 | 9-bit field reporting the index of the sample in the CIR accumulated using the STS that has the largest amplitude. Available when CIADONE is set.      |
| -        | 0D:38 bits:31,30 | Reserved.                                                                                                                                              |

#### 8.2.13.28 Sub-register 0x0D:3C – STS 1 diagnostic 1

| ID    | Length (octets) | Type | Mnemonic    | Description                                 |
| ----- | --------------- | ---- | ----------- | ------------------------------------------- |
| 0D:3C | 4               | RO   | STS1_DIAG_1 | STS 1 diagnostic 1 – STS 1 power indication |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x3C of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit CIA_CONF has been cleared to zero.

**REG:0D:3C – STS1_DIAG_1 – STS 1 diagnostic 1 – STS 1 power indication**

| Field    | reg:addr         | Description                                                                                                                                                   |
| -------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_CAREA | 0D:3C bits:15–0  | 16-bit field reporting the channel area in the CIR accumulated using the STS. Gives a measure of the receive power of the STS. Available when CIADONE is set. |
| -        | 0D:3C bits:31–16 | Reserved.                                                                                                                                                     |

#### 8.2.13.29 Sub-register 0x0D:40 – STS 1 diagnostic 2

| ID    | Length (octets) | Type | Mnemonic    | Description                                   |
| ----- | --------------- | ---- | ----------- | --------------------------------------------- |
| 0D:40 | 4               | RO   | STS1_DIAG_2 | STS 1 diagnostic 2 – STS 1 magnitude @ FP + 1 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x40 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:40 – STS1_DIAG_2 – STS 1 diagnostic 2 – STS 1 magnitude @ FP + 1**

| Field   | reg:addr         | Description                                                                                                                                                                                    |
| ------- | ---------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_FP1M | 0D:40 bits:21–0  | 22-bit field reporting the magnitude of the sample at the first index immediately after the estimated first path position in the CIR accumulated using the STS. Available when CIADONE is set. |
| -       | 0D:40 bits:31–22 | Reserved.                                                                                                                                                                                      |

#### 8.2.13.30 Sub-register 0x0D:44 – STS 1 diagnostic 3

| ID    | Length (octets) | Type | Mnemonic    | Description                                   |
| ----- | --------------- | ---- | ----------- | --------------------------------------------- |
| 0D:44 | 4               | RO   | STS1_DIAG_3 | STS 1 diagnostic 3 – STS 1 magnitude @ FP + 2 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x44 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:44 – STS1_DIAG_3 – STS 1 diagnostic 3 – STS 1 magnitude @ FP + 2**

| Field   | reg:addr         | Description                                                                                                                                                                     |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_FP2M | 0D:44 bits:21–0  | 22-bit field reporting the magnitude of the sample at second index after the estimated first path position in the CIR accumulated using the STS. Available when CIADONE is set. |
| -       | 0D:44 bits:31–22 | Reserved.                                                                                                                                                                       |

#### 8.2.13.31 Sub-register 0x0D:48 – STS 1 diagnostic 4

| ID    | Length (octets) | Type | Mnemonic    | Description                                   |
| ----- | --------------- | ---- | ----------- | --------------------------------------------- |
| 0D:48 | 4               | RO   | STS1_DIAG_4 | STS 1 diagnostic 4 – STS 1 magnitude @ FP + 3 |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x48 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:48 – STS1_DIAG_4 – STS 1 diagnostic 4 – STS 1 magnitude @ FP + 3**

| Field   | reg:addr         | Description                                                                                                                                                                    |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| CP_FP3M | 0D:48 bits:21–0  | 22-bit field reporting the magnitude of the sample at third index after the estimated first path position in the CIR accumulated using the STS. Available when CIADONE is set. |
| -       | 0D:48 bits:31–22 | Reserved.                                                                                                                                                                      |

#### 8.2.13.32 Sub-register 0x0D:4C – Reserved

| ID    | Length (octets) | Type | Mnemonic       | Description              |
| ----- | --------------- | ---- | -------------- | ------------------------ |
| 0D:4C | 12              | RO   | STS1_DIAG_RES1 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x4C of register file 0x0D is a 12-octet reserved register.

#### 8.2.13.33 Sub-register 0x0D:58 – STS 1 diagnostic 8

| ID    | Length (octets) | Type | Mnemonic    | Description                           |
| ----- | --------------- | ---- | ----------- | ------------------------------------- |
| 0D:58 | 4               | RO   | STS1_DIAG_8 | STS 1 diagnostic 8 – STS 1 first path |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x58 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit in CIA_CONF has been cleared to zero.

**REG:0D:58 – STS1_DIAG_8 – STS 1 diagnostic 8 – STS 1 first path**

| Field | reg:addr         | Description                                                                                                                                                                                                                                                                                                                                                    |
| ----- | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CP_FP | 0D:58 bits:14–0  | 15-bit field reporting the estimated first path location within the CIR accumulated using the STS where it is present and being used to measure the PDOA. [9.6] fixed point format (9-bit unsigned integer, 6-bit fractional). Index relative to start of STS CIR within the accumulator (beginning at accumulator index 1024). Available when CIADONE is set. |
| -     | 0D:58 bits:31–15 | Reserved.                                                                                                                                                                                                                                                                                                                                                      |

#### 8.2.13.34 Sub-register 0x0D:5C – Reserved

| ID    | Length (octets) | Type | Mnemonic       | Description              |
| ----- | --------------- | ---- | -------------- | ------------------------ |
| 0D:5C | 12              | RO   | STS1_DIAG_RES2 | Reserved diagnostic data |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x5C of register file 0x0D is a 12-octet reserved register.

#### 8.2.13.35 Sub-register 0x0D:68 – STS 1 diagnostic 12

| ID    | Length (octets) | Type | Mnemonic     | Description                                        |
| ----- | --------------- | ---- | ------------ | -------------------------------------------------- |
| 0D:68 | 4               | RO   | STS1_DIAG_12 | STS 1 diagnostic 12 – STS 1 accumulated STS length |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x68 of register file 0x0D is a 4-octet register reporting diagnostics relating to the STS. Valid only when PDOA mode 3 is used.

**Please note:** This diagnostics register will not be updated unless the MINDIAG configuration bit CIA_CONF has been cleared to zero.

**REG:0D:68 – STS1_DIAG_12 – STS 1 diagnostic 12 – STS 1 accumulated STS length**

| Field   | reg:addr         | Description                                                                                                                             |
| ------- | ---------------- | --------------------------------------------------------------------------------------------------------------------------------------- |
| CP_NACC | 0D:68 bits:10–0  | 11-bit field reporting the length of STS accumulated in units of 512 chips (~1 µs) to form the STS CIR. Available after CIADONE is set. |
| -       | 0D:68 bits:31–11 | Reserved.                                                                                                                               |

#### 8.2.13.36 Sub-register 0x0E:00 – RX antenna delay and CIA diagnostic enable

| ID    | Length (octets) | Type | Mnemonic | Description                                |
| ----- | --------------- | ---- | -------- | ------------------------------------------ |
| 0E:00 | 4               | RW   | CIA_CONF | RX antenna delay and CIA diagnostic enable |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x00 of register file 0x0E is a 4-octet configuration register.

**REG:0E:00 – CIA_CONF – RX antenna delay and CIA diagnostic enable (default: 0x00104015)**

| Field   | reg:addr         | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| ------- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| RXANTD  | 0E:00 bits:15–0  | 16-bit field configuring the receive antenna delay. Used to account for the delay between the arrival of the RMARKER at the antenna and the time the RMARKER is detected and time-stamped by the internal digital RX circuitry. Units are the same as system time: 1/(499.2 MHz × 128), LSB ≈ 15.65 ps. Default value 0x4015 ≈ 256.74 ns. The value is subtracted by the CIA algorithm from the raw timestamp RX_RAWST, along with other updates and adjustments, to generate the fully adjusted RX_STAMP value in Sub-register 0x00:64. See §10.3 – IC calibration – antenna delay for calibration details. |
| MINDIAG | 0E:00 bit:20     | Minimum Diagnostics. Default 1. When 1, the CIA does NOT update any of the preamble CIA diagnostic registers (IP_DIAG_0 – IP_DIAG_12), STS CIA diagnostic registers (STS_DIAG_0 – STS_DIAG_12), or second STS CIA diagnostic registers (STS1_DIAG_0 – STS1_DIAG_12, when PDoA Mode 3 is used). Clear to 0 to generate extended diagnostics; **note:** this delays assertion of CIADONE and RXFR event status flag bits, causing ranging exchanges to take a little longer and consume a little more energy.                                                                                                  |
| -       | 0E:00 bits:31–21 | Reserved. **N.B.: Any change in these bits may cause the DW3000 to malfunction.**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |

#### 8.2.13.37 Sub-register 0x0E:04 – First path temperature adjustment

| ID    | Length (octets) | Type | Mnemonic | Description                               |
| ----- | --------------- | ---- | -------- | ----------------------------------------- |
| 0E:04 | 4               | RW   | FP_CONF  | First path temp adjustment and thresholds |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x04 of register file 0x0E is a 4-octet register containing first path temperature adjustments and threshold values.

**REG:0E:04 – FP_CONF – First path temp adjustment and thresholds (default: 0x00F00480)**

| Field        | reg:addr           | Description                                                                                                                                                                                                    |
| ------------ | ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| FP_AGREED_TH | 0E:04 bits:10–8    | Threshold to use when performing the FP_AGREE test. The LSB is 1/(998.4 MHz) ≈ 1 ns.                                                                                                                           |
| CAL_TEMP     | 0E:04 bits:18–11   | Temperature at which the device was calibrated. Units are from the on-chip temperature sensor, as read from SAR_LTEMP in SAR_READING register.                                                                 |
| TC_RXDLY_EN  | 0E:04 bit:20       | Temperature compensation for RX antenna delay. When set to 1, the device will compensate for temperature differences from the calibration temperature (CAL_TEMP). Default corresponds to absolute zero Kelvin. |
| -            | 0E:04 bits:various | Reserved. **N.B.: Any change in these bits may cause the DW3000 to malfunction.**                                                                                                                              |

#### 8.2.13.38 Sub-register 0x0E:0C – CIA preamble configuration

| ID    | Length (octets) | Type | Mnemonic | Description                                  |
| ----- | --------------- | ---- | -------- | -------------------------------------------- |
| 0E:0C | 4               | RW   | IP_CONF  | Preamble config – CIA preamble configuration |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x0C of register file 0x0E is a 4-octet configuration register relating to the CIA configuration for the preamble sequence CIR analysis.

**REG:0E:0C – IP_CONF0 – CIA preamble configuration (default: 0x00100014B)**

| Field    | reg:addr           | Description                                                                                                                                                                                                                                                                                                                                                                                           |
| -------- | ------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IP_NTM   | 0E:0C bits:4–0     | Preamble Noise Threshold Multiplier. 5-bit field. Factor by which the measured noise level in the preamble generated CIR is multiplied to set the threshold for the CIA algorithm's first path search. Sets the compromise between falsely triggering on noise peaks and missing real attenuated (NLOS) first paths. Where NLOS performance is more important, a lower value (e.g. 12) could be used. |
| IP_PMULT | 0E:0C bits:6–5     | Preamble Peak Multiplier. 2-bit field. Factor by which the peak value of estimated noise is increased to set the threshold for first path searching. Values: 0 → 1.00×, 1 → 1.25×, 2 → 1.50× (default), 3 → 1.75×.                                                                                                                                                                                    |
| IP_RTM   | 0E:0C bits:20–16   | Preamble replica threshold multiplier. 5-bit field. Tunes the choice of replica avoidance threshold. The default value of 4 is not expected to need modification.                                                                                                                                                                                                                                     |
| -        | 0E:0C bits:various | Reserved. **N.B.: Any change in these bits may cause the DW3000 to malfunction.**                                                                                                                                                                                                                                                                                                                     |

#### 8.2.13.39 Sub-register 0x0E:12 – CIA STS configuration 0

| ID    | Length (octets) | Type | Mnemonic   | Description                              |
| ----- | --------------- | ---- | ---------- | ---------------------------------------- |
| 0E:12 | 4               | RW   | STS_CONF_0 | STS Config 0 – CIA configuration for STS |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x12 of register file 0x0E is a 4-octet configuration register relating to the CIA configuration for the STS based CIR analysis.

**REG:0E:12 – STS_CONF_0 – CIA configuration for STS (default: 0x000C0174)**

| Field     | reg:addr           | Description                                                                                                                                                                                                                                                                                                                                                                                                        |
| --------- | ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| STS_NTM   | 0E:12 bits:4–0     | STS Noise Threshold Multiplier. 5-bit field. Factor by which the measured noise level in the STS generated CIR is multiplied to set the threshold for CIA first path search. Sets compromise between falsely triggering on noise peaks and missing real attenuated (NLOS) first paths.                                                                                                                             |
| STS_PMULT | 0E:12 bits:6–5     | STS Peak Multiplier. 2-bit field. Factor by which the peak value of estimated noise in the STS generated CIR is increased for first path searching threshold. Values: 0 → 0.00× (default), 1 → 1.25×, 2 → 1.50×, 3 → 1.75×. A value of 0 allows the first path to be in the region of the CIR used for gathering noise statistics, which helps when the channel is severely NLOS with a large delay spread.        |
| STS_MNTH  | 0E:12 bits:22–16   | STS Minimum Threshold. 7-bit field. Sets a lower bound on the threshold for first path searching. The default STS_MNTH value is 12, suitable for default 64 MHz PRF and STS length of 64 (CPS_LEN = 7). Recommended values by PRF / STS length (PDoA Off/1 vs PDoA Mode 3): 64 MHz / 64 chips → 12 / 0; 64 MHz / 128 chips → 17 / 12; 64 MHz / 256 chips → 24 / 17. Values are scaled by √2 as STS length doubles. |
| -         | 0E:12 bits:various | Reserved. **N.B.: Any change in these bits field may cause the DW3000 to malfunction.**                                                                                                                                                                                                                                                                                                                            |

#### 8.2.13.40 Sub-register 0x0E:16 – CIA STS configuration 1

| ID    | Length (octets) | Type | Mnemonic   | Description                              |
| ----- | --------------- | ---- | ---------- | ---------------------------------------- |
| 0E:16 | 4               | RW   | STS_CONF_1 | STS Config 1 – CIA configuration for STS |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x16 of register file 0x0E is a 4-octet configuration register relating to the CIA configuration for the STS based CIR analysis.

**REG:0E:16 – STS_CONF_1 – CIA configuration for STS (default: 0xF00B5B77B)**

| Field        | reg:addr        | Description                                                                                                                                                                                                                                                                                                               |
| ------------ | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| RES_B0       | 0E:16 bits:7–0  | Reserved tuning value. Should be changed from default 0x7B to 0x9B for optimised performance.                                                                                                                                                                                                                             |
| FP_AGREED_EN | 0E:16 bit:28    | Checks that the two ToA estimates are within allowed tolerances. Tolerance set in FP_AGREED_TH. If the test fails, the appropriate bit in the STS_TOAST status indicator will be set to indicate that the STS_TOA value is not reliable and should not be used.                                                           |
| STS_CQ_EN    | 0E:16 bit:29    | Checks how consistent the impulse response stays during the accumulation of the STS. Since the RF channel should remain fairly constant over the sequence, this check helps ensure the integrity of the STS based RX timestamp. If the test fails, the appropriate bit in STS_TOAST will be set.                          |
| STS_SS_EN    | 0E:16 bit:30    | Compares the sampling statistics of the STS reception to those of the earlier reception of the preamble sequence. Since both relate to the same transmitter over the same channel, preamble and STS sequence statistics should match (within tolerance). If the test fails, the appropriate bit in STS_TOAST will be set. |
| STS_PGR_EN   | 0E:16 bit:31    | Tests the growth rate of the STS based CIR to the earlier growth rate of the preamble based CIR. Since both are estimating the channel impulse response of the same channel, they should grow at the same rate (within tolerance). If the test fails, the appropriate bit in STS_TOAST will be set.                       |
| -            | 0E:16 bits:27–0 | Reserved. **N.B.: Any change in these bits field may cause the DW3000 to malfunction.**                                                                                                                                                                                                                                   |

#### 8.2.13.41 Sub-register 0x0E:1A – CIA adjustment

| ID    | Length (octets) | Type | Mnemonic   | Description                             |
| ----- | --------------- | ---- | ---------- | --------------------------------------- |
| 0E:1A | 2               | RW   | CIA_ADJUST | User adjustments to the CIA calculation |

Register files: 0x0C, 0x0D, 0x0E – CIA Interface, sub-register 0x1A of register file 0x0E is a 2-octet configuration register relating to the CIA configuration for the STS based CIR analysis.

**REG:0E:1A – PDOA_ADJ – User adjustments to the PDoA**

| Field    | reg:addr         | Description                                                                                                                                                                                                                                                                                        |
| -------- | ---------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| PDOA_ADJ | 0E:1A bits:13–0  | Adjustment value to account for non-balanced antenna circuits. The LSB is 2⁻¹¹ radians. This angle is simply added to the PDoA that is estimated from the CIR. Interpreted by the device as an unsigned integer. A value between 0 and 12868 represents an offset in the 0 to 2π radians interval. |
| -        | 0E:1A bits:15–14 | Reserved. **N.B.: Any change in these bits field may cause the DW3000 to malfunction.**                                                                                                                                                                                                            |
