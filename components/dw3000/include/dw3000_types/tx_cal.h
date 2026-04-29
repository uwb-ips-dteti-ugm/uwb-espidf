#ifndef DW3000_TYPES_TX_CAL_H
#define DW3000_TYPES_TX_CAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SAR_CTRL (0x08:00) — start one SAR ADC conversion. Self-clearing;
   poll SAR_STATUS or read SAR_READING after the bit drops. */
typedef enum {
    DW3000_TX_CAL_SAR_START = 1U << 0,
} dw3000_tx_cal_sar_ctrl_t;

/* SAR_STATUS (0x08:04) — conversion-complete flag. */
typedef enum {
    DW3000_TX_CAL_SAR_DONE = 1U << 0,
} dw3000_tx_cal_sar_status_t;

/* SAR_READING (0x08:08) — latched ADC measurements after a manually
   triggered conversion. Convert to volts/celsius using the OTP-stored
   factory references (VBAT in cell 0x08, VTEMP in cell 0x09). */
typedef struct {
    uint8_t vbat;
    uint8_t vtemp;
} dw3000_tx_cal_sar_reading_t;

/* SAR_WAKE_RD (0x08:0C) — same fields, sampled automatically on wake
   when AON_DIG_CFG.ONW_RUN_SAR is asserted before sleep. */
typedef dw3000_tx_cal_sar_reading_t dw3000_tx_cal_sar_wake_t;

/* PGC_CTRL (0x08:10) — pulse-generator auto-calibration controls.
   START kicks one measurement; AUTOCAL_EN re-runs the loop on every
   transmission. */
typedef enum {
    DW3000_TX_CAL_PGC_START      = 1U << 0,
    DW3000_TX_CAL_PGC_AUTOCAL_EN = 1U << 1,
} dw3000_tx_cal_pgc_ctrl_t;

#define DW3000_TX_CAL_PGC_TMEAS_MAX 0x0FU

/* PGC_STATUS (0x08:14) — flags above the 12-bit delay result. */
typedef enum {
    DW3000_TX_CAL_PGC_AUTOCAL_DONE = 1U << 12,
} dw3000_tx_cal_pgc_status_flags_t;

#define DW3000_TX_CAL_PGC_DELAY_MASK 0x0FFFU

typedef struct {
    uint16_t                         delay; /* bits [11:0] */
    dw3000_tx_cal_pgc_status_flags_t flags;
} dw3000_tx_cal_pgc_status_t;

/* PG_TEST (0x08:18) — pulse-generator test mode. Bench / characterisation
   use only. */
typedef uint16_t dw3000_tx_cal_pg_test_t;

#define DW3000_TX_CAL_PG_TEST_NORMAL 0x0000U
#define DW3000_TX_CAL_PG_TEST_CW     0x000FU

/* PG_CAL_TARGET (0x08:1C) — 12-bit target delay the PGC autocal loop
   converges towards. Channel-dependent; sourced from OTP. */
typedef uint16_t dw3000_tx_cal_pg_target_t;

#define DW3000_TX_CAL_PG_TARGET_MASK 0x0FFFU

typedef struct {
    dw3000_tx_cal_pgc_ctrl_t  pgc_ctrl;
    dw3000_tx_cal_pg_target_t pg_target;
} dw3000_tx_cal_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_TX_CAL_H */
