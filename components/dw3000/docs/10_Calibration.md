# 10 Calibration

> **Source:** DW3000 Family User Manual, Version 1.1 (© Decawave Ltd 2019, revised 28 May 2021)
> **Pages:** 243-245

The operating characteristics and performance of the DW3000 is dependent on the IC itself and on its external circuitry and on its operating environment. To give optimum performance it is necessary to calibrate the IC to account for factors which affect its operation.

Some calibration parameters are dependent solely on process variations that occur within the silicon of the IC during its manufacture. These are typically measured during IC production test and the required calibration parameters are written to the OTP memory of the DW3000. The host system software can then use these values during DW3000 configuration to optimise the DW3000 performance.

Some calibration parameters are dependent on circuit elements external to the IC. These can only be determined during the manufacture of the product e.g. a module into which the DW3000 is soldered. These parameters are typically measured during manufacture of the product and the required calibration parameters are stored in an area of the DW3000's OTP memory which has been allocated for test calibration parameters. The host system software will use this calibration data during DW3000 configuration to optimise the DW3000 performance.

Some calibration parameters may vary according to the operational environment of the DW3000. For example some parameters may need to be changed if there are large variations in the ambient temperature (e.g. moving from a warm area into a cold store). In such circumstances in order to optimise the DW3000 performance the host system software can monitor the voltage and temperature using DW3000 and adjust configuration accordingly.

Elements of the DW3000 that may be subject to calibration are:

- Crystal trimming – the DW3000 contains trimming capacitors that can fine tune the operating frequency of its crystal oscillator.
- Transmitter output power and spectrum – the DW3000 output spectrum is tuneable to meet regional spectral regulations and maximise the output power to achieve the greatest operating range.
- Antenna delay – the DW3000 antenna delay may be fine-tuned to give best possible ranging or location accuracy.
- IC calibration – PLL calibration over temperature.

The sub-sections below detail the calibration of these DW3000 parameters.

---

## 10.1 IC calibration – crystal oscillator trim

DW3000 is specified to operate with clock offsets between the transmitting and receiving nodes of up to ±20 ppm. The receiver sensitivity of DW3000 can be improved by reducing the relative offset in clocks between the transmitting and receiving nodes. One way to reduce the offset is to use temperature compensated crystal oscillators (TCXO) in both transmitting and receiving nodes or just in one side of the link. Using TCXOs increases system cost and current consumption so generally they are only used in fixed anchor scenarios. Where crystals are used as the clock reference, DW3000 provides a facility to trim the oscillator frequency by switching in internal capacitor banks in parallel with the external loading capacitors associated with the chosen crystal. This trimming can be used to reduce the crystal initial frequency error and to compensate for temperature and aging drift, if required.

The amount of trimming is programmable through Sub-register 0x09:14 – Crystal trim register.

### 10.1.1 Calibration method

Please see API and example code [3].

---

## 10.2 IC calibration – transmit power and spectrum

DW3000 provides registers to adjust the transmit power and spectrum bandwidth to meet regulatory limits. Decawave provides an application note (APS312 [6]) which describes this in detail. The following registers are provided for control of the transmit power and bandwidth:

| Parameter      | Register |
| -------------- | -------- |
| Transmit Power | TX_POWER |
| Bandwidth      | PG_DELAY |

### 10.2.1 Calibration method

Please see API and example code [3], and Application Note APS312 [6].

---

## 10.3 IC calibration – antenna delay

In order to measure range accurately, precise calculation of timestamps is required. To do this, a delay called the antenna delay must be known. The DW3000 allows this delay to be calibrated and provides the facility to compensate for delays introduced by PCB, external components, antenna and internal DW3000 delays.

To calibrate the antenna delay, range is measured at a known distance using two DW3000 systems. Antenna delay is adjusted until the known distance and reported range agree. The antenna delay can be stored in OTP memory.

There is a Transmitter Antenna Delay and a Receiver Antenna Delay. The Transmitter Antenna Delay is used to account for the delay between the internal digital timestamp of the RMARKER (as shown in Figure 13) inside the DW3000 and the time the RMARKER is transmitted from the antenna. The Receiver Antenna Delay is used to account for the delay between the time of arrival of the RMARKER at the antenna and the internal digital timestamp of the RMARKER inside the DW3000.

![Figure 30: Transmit and Receive Antenna Delay](image/DW3000%20User%20Manual_Page_245.jpg)

_Figure 30: Transmit and Receive Antenna Delay_

### 10.3.1 Calibration method

See API and examples [3], and Application Note APS014.

---

## 10.4 IC Calibration – PLL calibration over temperature

For proper device operation, when using channel 9, the PLL will need to be re-calibrated if the device temperature changes by 20° C, using the steps below:

- firstly the device state needs to be changed to **IDLE_RC** state, by clearing AINIT2IDLE configuration bit in SEQ_CTRL and setting the FORCE2INIT bit
- then setting system clocks to FAST_RC, by setting the SYSCLKS field to 0x3
- then clearing the FORCE2INIT bit
- and restoring system clocks to Auto mode, by setting the SYSCLKS field to 0x0
- finally the host should set the CAL_EN bit in PLL_CAL register and the AINIT2IDLE configuration bit in SEQ_CTRL register to cause the IC will enable the PLL and wait for it to lock before entering the **IDLE_PLL** state

This only applies to channel 9. When using channel 5 the PLL does not need to be re-calibrated if temperature changes.
