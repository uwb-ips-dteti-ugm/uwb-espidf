#ifndef DW3000_TYPES_CALIB_H
#define DW3000_TYPES_CALIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TX_POWER (0x01:0C) — four independent 8-bit power-control bytes,
   one per frame section. Each byte splits into 3-bit coarse gain
   ([7:5], ~3 dB steps) and 5-bit fine gain ([4:0], ~0.5 dB steps);
   typically programmed as raw bytes sourced from OTP cal cells. */
typedef struct {
    uint8_t data; /* DATA section          */
    uint8_t phr;  /* PHR  section          */
    uint8_t shr;  /* SHR (preamble) section*/
    uint8_t sts;  /* STS section           */
} dw3000_calib_tx_power_t;

/* XTAL (0x09:14) — 7-bit crystal trim (~1.5 ppm/step), bit 7 reserved.
   Sourced from OTP cell holding the factory-measured trim. */
typedef uint8_t dw3000_calib_xtal_trim_t;

/* TX_ANTD (0x01:04) and CIA_CONF.RX_ANTD reuse the
   dw3000_cia_antenna_delay_t typedef from cia.h. */

/* RX_CAL (0x04:0C) — RX I/Q calibration controls. Set CAL_EN to start
   a measurement; result lands in RX_CAL_RESI / RX_CAL_RESQ once
   RX_CAL_STS.DONE rises. */
typedef enum {
    DW3000_CALIB_RX_CAL_EN   = 1U << 0,
    DW3000_CALIB_RX_CAL_MODE = 1U << 4,
} dw3000_calib_rx_cal_flags_t;

/* RX_CAL_STS (0x04:20). */
typedef enum {
    DW3000_CALIB_RX_CAL_DONE = 1U << 0,
} dw3000_calib_rx_cal_status_t;

/* RX_CAL_RESI (0x04:14) and RX_CAL_RESQ (0x04:1C) — 32-bit per-channel
   calibration measurement. */
typedef struct {
    uint32_t i;
    uint32_t q;
} dw3000_calib_rx_cal_result_t;

/* Well-known OTP cell addresses holding factory calibration data. The
   HAL reads these via OTP_ADDR/OTP_RDATA during init and either copies
   them into the runtime registers above or into LDO/BIAS/DGC/OPS via
   the OTP_CFG.*_KICK bits. Only cells whose meaning is stable across
   silicon revisions are listed; per-channel TX-power and antenna-delay
   cells live at variable offsets and are looked up by the HAL against
   the part revision in cell 0x1F. */
typedef enum {
    DW3000_CALIB_OTP_EUI_LO   = 0x00U,
    DW3000_CALIB_OTP_EUI_HI   = 0x01U,
    DW3000_CALIB_OTP_PART_ID  = 0x06U,
    DW3000_CALIB_OTP_LOT_ID   = 0x07U,
    DW3000_CALIB_OTP_VBAT     = 0x08U, /* factory VBAT  reading */
    DW3000_CALIB_OTP_VTEMP    = 0x09U, /* factory VTEMP reading */
    DW3000_CALIB_OTP_REVISION = 0x1FU,
} dw3000_calib_otp_cell_t;

typedef struct {
    dw3000_calib_tx_power_t  tx_power;
    dw3000_calib_xtal_trim_t xtal_trim;
} dw3000_calib_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_CALIB_H */
