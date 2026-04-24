# 9 Fast Commands

> **Source:** DW3000 Family User Manual, Version 1.1 (© Decawave Ltd 2019, revised 28 May 2021)
> **Pages:** 238-242

This section lists and describes the single-octet commands used initiate specific IC activities. For details of format for the fast command SPI transaction please refer to section 2.3 – The SPI interface. Table 46 lists the supported commands and their hex codes, and each command is described in separate sub-sections following this.

**Table 46: List of supported fast commands**

| Command ID      | Code | Brief Description                                                                                                |
| --------------- | ---- | ---------------------------------------------------------------------------------------------------------------- |
| CMD_TXRXOFF     | 0x0  | Puts the device into IDLE state and clears any events.                                                           |
| CMD_TX          | 0x1  | Immediate start of transmission                                                                                  |
| CMD_RX          | 0x2  | Enable RX immediately                                                                                            |
| CMD_DTX         | 0x3  | Delayed TX w.r.t. DX_TIME                                                                                        |
| CMD_DRX         | 0x4  | Delayed RX w.r.t. DX_TIME                                                                                        |
| CMD_DTX_TS      | 0x5  | Delayed TX w.r.t. TX timestamp + DX_TIME                                                                         |
| CMD_DRX_TS      | 0x6  | Delayed RX w.r.t. TX timestamp + DX_TIME                                                                         |
| CMD_DTX_RS      | 0x7  | Delayed TX w.r.t. RX timestamp + DX_TIME                                                                         |
| CMD_DRX_RS      | 0x8  | Delayed RX w.r.t. RX timestamp + DX_TIME                                                                         |
| CMD_DTX_REF     | 0x9  | Delayed TX w.r.t. DREF_TIME + DX_TIME                                                                            |
| CMD_DRX_REF     | 0xA  | Delayed RX w.r.t. DREF_TIME + DX_TIME                                                                            |
| CMD_CCA_TX      | 0xB  | TX if no preamble detected                                                                                       |
| CMD_TX_W4R      | 0xC  | Start TX immediately, then when TX is done, enable the receiver                                                  |
| CMD_DTX_W4R     | 0xD  | Delayed TX w.r.t. DX_TIME, then enable receiver                                                                  |
| CMD_DTX_TS_W4R  | 0xE  | Delayed TX w.r.t. TX timestamp + DX_TIME, then enable receiver                                                   |
| CMD_DTX_RS_W4R  | 0xF  | Delayed TX w.r.t. RX timestamp + DX_TIME, then enable receiver                                                   |
| CMD_DTX_REF_W4R | 0x10 | Delayed TX w.r.t. DREF_TIME + DX_TIME, then enable receiver                                                      |
| CMD_CCA_TX_W4R  | 0x11 | TX packet if no preamble detected, then enable receiver                                                          |
| CMD_CLR_IRQS    | 0x12 | Clear all interrupt events                                                                                       |
| CMD_DB_TOGGLE   | 0x13 | Toggle double buffer pointer / notify the device that the host has finished processing the received buffer/data. |

---

## 9.1 CMD_TXRXOFF

This command puts the device into **IDLE_PLL** state, it will turn off the transmitter or receiver if the device is actively transmitting or receiving, thus any active transmission or reception is aborted.

This command must be issued only when device is in **IDLE_PLL**, **TX** or **RX** state, otherwise the device will need a restart. It should not be issued when device is in **INIT_RC** or **IDLE_RC** states.

**Note:**

**If host has configured the device into TX test mode: e.g. continuous frame mode (when** TX_PSTM **bit is set), issuing TRXOFF will not put the device into IDLE state.**

---

## 9.2 CMD_TX

This is a start TX command. It commands the DW3000 to start transmission. The transmission will start immediately once the TX blocks are powered up. In general it would be expected that the user has a prepared frame in the transmit buffer and has configured the desired transmit mode and set the frame length. For general discussion of transmission see section 3 Message transmission.

---

## 9.3 CMD_RX

This command enables the receiver. It commands the DW3000 to turn on its receiver and begin looking for the configured preamble sequence. It is assumed that the all necessary configurations have been made before turning on the receiver. For general discussion of reception see section 4 Message reception.

---

## 9.4 CMD_DTX

This command instructs the device to do a delayed transmission. This control works in conjunction with the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time. When the user wants to control the time of sending of a packet, the send time is programmed into DX_TIME, and then this command issued.

When delayed sending is used the DW3000 precisely controls the transmission start time so that the internal TX timestamp occurs at the point when SYS_TIME is equal to the DX_TIME value. The actual time of TX then is calculable as DX_TIME plus the TX antenna delay.

### 9.4.1 Delayed TX notes

CMD_DTX has a number of uses:

- It can be used to give precise control of the transmission time of a response message, which would allow a receiver that knows this response time to only turn on at the correct time to receive the response, thus saving power.

- In symmetric double-sided two-way ranging, the RX to TX response times at either end should be the same so that their differences in local clocks correctly cancel out. This may be ensured by setting DX_TIME to a value that is a fixed delta added to the RX time-stamp.

- In two-way ranging the TX timestamp of the final message exchange needs to be communicated to the receiving end to allow the round-trip delay to be calculated. Using CMD_DTX allows this time to be predicted, pre-calculated and embedded into the final message itself. This may save the need for an additional message interchange which will give a power saving, and save time too.

- Embedding the TX time in this way may also reduce the number of messages in a wireless clock synchronisation scheme.

**Note:**

**If host issues a delayed transmit command after the specified TX time has "passed", i.e. the time (in DX_TIME) is more than half period away from the current system time, a HPDWARN event will be set (please see full description of HPDWARN event). Otherwise the device should enter the TX delay wait state (PMSC_STATE = 0xA), and remain in this state until it starts to transmit the packet.**

**Due to an errata in the DW3000, there is a case when neither the HPDWARN event gets set nor does the packet get transmitted. This happens when the time at which the transmitter needs to be enabled to send occurs within the analog front-end power up time preceding the current time. To avoid this bug reliably, the delayed send time must be set far enough in advance; it should be at least equal or more than the current time + preamble length + SFD length + 20 µs, or the time should be passed which will give rise to HPDWARN.**

**The host can check for this issue by reading the PMSC_STATE. When this bug occurs, the PMSC_STATE will be "TX" but TX_STATE will be "IDLE", the TXFRS event will never be set, see state descriptions in 8.2.14.19 Sub-register 0x0F:30 – System state. The host should abort the transmission in this case. This check and recovery is implemented in the published DW3000 dwt-starttx() API.**

---

## 9.5 CMD_DRX

This command instructs the device to turn on the receiver with a delay. This command works in conjunction with the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time. When the user wants to control the time of turning on the receiver, the turn on time is programmed into DX_TIME, and then this command executed. The DW3000 then precisely controls the RX turn on time so that it is ready to receive the first symbol of preamble at the specified DX_TIME start time. In cases when the received time can be known precisely, for example when a response is expected at a well-defined time, employing CMD_DRX will give a power saving as it allows the IC to remain idle until the moment it is required to act for the reception.

---

## 9.6 CMD_DTX_TS

This command will do a delayed transmission with respect to the last transmission timestamp. The new TX timestamp will be the combination of the previous TX timestamp and the delay programmed in the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time (i.e. TX_TSnew = TX_TSold + DX_TIME). See also notes in delayed TX section 9.4.1.

---

## 9.7 CMD_DRX_TS

This command will instruct the device to turn on its receiver with respect to the last transmission timestamp. The receiver turn on time will be the combination of the previous TX timestamp and the delay programmed in the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time.

---

## 9.8 CMD_DTX_RS

This command will do a delayed transmission with respect to the last receive timestamp. The new TX timestamp will be the combination of the previous RX timestamp (RX_TIME) and the delay programmed in the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time (i.e. TX_Tsnew = RX_Tsold + DX_TIME). See also notes in delayed TX section 9.4.1.

---

## 9.9 CMD_DRX_RS

This command will instruct the device to turn on its receiver with respect to the last receive timestamp. The receiver turn on time will be the combination of the previous RX timestamp and the delay programmed in the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time.

---

## 9.10 CMD_DTX_REF

This command will do a delayed transmission with respect to the time programmed into the DREF_TIME register. The new TX timestamp will be the combination of the DREF_TIME and the delay programmed in the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time (i.e. TX_Tsnew = DREF + DX_TIME). See also notes in delayed TX section 9.4.1.

---

## 9.11 CMD_DRX_REF

This command will instruct the device to turn on its receiver with respect to the time programmed into the DREF_TIME register. The receiver turn on time will be the combination of the DREF_TIME and the delay programmed in the DX_TIME value specified by Sub-register 0x00:2C – Delayed send or receive time.

---

## 9.12 CMD_CCA_TX

This command instructs the device to perform a pseudo CCA before starting transmission. Once this command is issued the device will turn on the receiver and failing to detect preamble within the time programmed in the preamble detection timeout register (Sub-register 0x06:04 – Preamble detection timeout count) it will start the transmission of the packet. If the preamble detection occurs, the transmission will be aborted and preamble detection signalled (RXPRD).

---

## 9.13 CMD_TX_W4R

Similarly to the start TX command above. It initially commands the DW3000 to start transmission, and then once the transmission is complete the device will enable its receiver. Optionally receiver can be enabled with a delay programmed into W4R_TIM in Sub-register 0x01:08 – Acknowledgement time and response time.

---

## 9.14 CMD_DTX_W4R

Similarly to the delayed TX command above. It initially commands the DW3000 to start delayed transmission (as described in CMD_DTX), and then once the transmission is complete the device will enable its receiver. Optionally receiver can be enabled with a delay programmed into W4R_TIM in Sub-register 0x01:08 – Acknowledgement time and response time. See also notes in delayed TX section 9.4.1.

---

## 9.15 CMD_DTX_TS_W4R

Similarly to the delayed TX command above. It initially commands the DW3000 to start delayed transmission (as described in CMD_DTX_TS), and then once the transmission is complete the device will enable its receiver. Optionally receiver can be enabled with a delay programmed into W4R_TIM in Sub-register 0x01:08 – Acknowledgement time and response time. See also notes in delayed TX section 9.4.1.

---

## 9.16 CMD_DTX_RS_W4R

Similarly to the delayed TX command above. It initially commands the DW3000 to start delayed transmission (as described in CMD_DTX_RS), and then once the transmission is complete the device will enable its receiver. Optionally receiver can be enabled with a delay programmed into W4R_TIM in Sub-register 0x01:08 – Acknowledgement time and response time. See also notes in delayed TX section 9.4.1.

---

## 9.17 CMD_DTX_REF_W4R

Similarly to the delayed TX command above. It initially commands the DW3000 to start delayed transmission (as described in CMD_DTX_REF), and then once the transmission is complete the device will enable its receiver. Optionally receiver can be enabled with a delay programmed into W4R_TIM in Sub-register 0x01:08 – Acknowledgement time and response time. See also notes in delayed TX section 9.4.1.

---

## 9.18 CMD_CCA_TX_W4R

Similarly to the pseudo CCA TX command above. It initially commands the DW3000 to start transmission (as described in CMD_CCA_TX), and then once the transmission is complete the device will enable its receiver. Optionally receiver can be enabled with a delay programmed into W4R_TIM in Sub-register 0x01:08 – Acknowledgement time and response time.

---

## 9.19 CMD_CLR_IRQS

This command will clear all of the status events and will clear the interrupt if set.

---

## 9.20 CMD_DB_TOGGLE

This command is only applicable when the device is using double buffering (the DIS_DRXB is set to 0). It will notify the device that the host has finished with the last RX buffer and the buffer is free for the device to use for another RX packet reception.
