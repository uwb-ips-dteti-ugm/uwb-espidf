#ifndef DW3000_TYPES_PLL_H
#define DW3000_TYPES_PLL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PLL_CFG (0x09:00) — 16-bit PLL configuration word. Channel-dependent;
   values are taken verbatim from the PLL configuration table in the
   DW3000 user manual (one row per RF channel). The HAL picks the row
   based on dw3000_phy_channel_t. */
typedef uint16_t dw3000_pll_cfg_t;

/* PLL_CC (0x09:04) — PLL coarse calibration code, 8 bits. Either
   programmed from a previously-stored value or written by the
   PLL_CAL.CAL_EN auto-calibration sequence. */
typedef uint8_t dw3000_pll_coarse_code_t;

/* PLL_CAL (0x09:08) — calibration controls. CAL_EN kicks the PLL
   coarse-code search; USE_OLD bypasses the search and applies the
   value already in PLL_CC (faster wake from sleep). */
typedef enum {
    DW3000_PLL_CAL_USE_OLD = 1U << 0,
    DW3000_PLL_CAL_EN      = 1U << 8,
} dw3000_pll_cal_flags_t;

/* XTAL (0x09:14) — see dw3000_calib_xtal_trim_t in calib.h. */

/* PLL lock and PLL_HILO surface as DW3000_TXRX_EVENT_CPLOCK /
   DW3000_TXRX_EVENT_PLL_HILO in txrx.h. */

typedef struct {
    dw3000_pll_cfg_t         cfg;
    dw3000_pll_coarse_code_t coarse_code;
    dw3000_pll_cal_flags_t   cal_flags;
} dw3000_pll_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_PLL_H */
