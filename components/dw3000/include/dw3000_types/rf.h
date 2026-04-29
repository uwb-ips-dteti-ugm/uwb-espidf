#ifndef DW3000_TYPES_RF_H
#define DW3000_TYPES_RF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RF_ENABLE (0x07:00) — block enables for the analog TX/RX paths.
   Normally driven automatically by the PMSC sequencer; only manipulated
   directly for continuous-wave or spectrum testing. */
typedef enum {
    DW3000_RF_ENABLE_TX_EN     = 1U << 9,
    DW3000_RF_ENABLE_TX_EN_BUF = 1U << 10,
    DW3000_RF_ENABLE_RX_EN     = 1U << 12,
    DW3000_RF_ENABLE_RXFE_EN   = 1U << 13,
    DW3000_RF_ENABLE_RX_LFO_EN = 1U << 14,
    DW3000_RF_ENABLE_RX_PLL_EN = 1U << 15,
} dw3000_rf_enable_t;

/* RF_CTRL_MASK (0x07:04) — gates which RF_ENABLE bits the sequencer is
   allowed to toggle. Same bit layout as RF_ENABLE. */
typedef dw3000_rf_enable_t dw3000_rf_ctrl_mask_t;

/* RF_SWITCH (0x07:14) — antenna / RF-switch routing. Layout is
   per-board; values come from the application's antenna design. */
typedef uint32_t dw3000_rf_switch_t;

/* RF_TX_CTRL_1 (0x07:1A) — 8-bit channel-specific TX gain word.
   Per the user manual: 0x0E for channel 5, 0x34 for channel 9. */
typedef uint8_t dw3000_rf_tx_ctrl_1_t;

/* RF_TX_CTRL_2 (0x07:1C) — 32-bit channel-specific TX configuration
   word, from the channel-configuration table in the user manual. */
typedef uint32_t dw3000_rf_tx_ctrl_2_t;

/* TX_TEST (0x07:28) — TX test-pattern selector. Compliance / spectrum
   testing only. */
typedef uint8_t dw3000_rf_tx_test_t;

/* SAR_TEST (0x07:34) — SAR ADC test-mode selector. */
typedef uint8_t dw3000_rf_sar_test_t;

/* LDO_TUNE (0x07:40) — 64-bit opaque LDO trim word, loaded from OTP
   via OTP_CFG.LDO_KICK. Not user-authored. */
typedef struct {
    uint8_t bytes[8];
} dw3000_rf_ldo_tune_t;

/* LDO_CTRL (0x07:48) — per-LDO enable bitmap. Each bit gates one
   on-chip regulator; normally driven by the PMSC sequencer. */
typedef enum {
    DW3000_RF_LDO_VDDIF1  = 1U << 0,
    DW3000_RF_LDO_VDDMS1  = 1U << 1,
    DW3000_RF_LDO_VDDMS2  = 1U << 2,
    DW3000_RF_LDO_VDDMS3  = 1U << 3,
    DW3000_RF_LDO_VDDIF2  = 1U << 4,
    DW3000_RF_LDO_VDDPLL  = 1U << 5,
    DW3000_RF_LDO_VDDLNA  = 1U << 6,
    DW3000_RF_LDO_VDDPDIF = 1U << 7,
    DW3000_RF_LDO_VDDPDIM = 1U << 8,
    DW3000_RF_LDO_VDDLNA2 = 1U << 9,
} dw3000_rf_ldo_ctrl_t;

/* LDO_RLOAD (0x07:51) — LDO load-resistance trim, sourced from OTP. */
typedef uint8_t dw3000_rf_ldo_rload_t;

typedef struct {
    dw3000_rf_tx_ctrl_1_t tx_ctrl_1;
    dw3000_rf_tx_ctrl_2_t tx_ctrl_2;
    dw3000_rf_switch_t    switch_cfg;
    dw3000_rf_ldo_tune_t  ldo_tune;
    dw3000_rf_ldo_ctrl_t  ldo_ctrl;
    dw3000_rf_ldo_rload_t ldo_rload;
} dw3000_rf_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_RF_H */
