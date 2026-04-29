#ifndef DW3000_TYPES_RX_TUNE_H
#define DW3000_TYPES_RX_TUNE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DTUNE0 (0x06:00) PAC field comes from dw3000_phy_pac_size_t in phy.h.
   The remaining bits are HAL-managed constants. */

/* RX_SFD_TOC (0x06:02) — SFD detection timeout, in preamble-symbol
   units. Counted from preamble lock; expiry raises EVENT_RXSTO. */
typedef uint16_t dw3000_rx_tune_sfd_toc_t;

/* PRE_TOC (0x06:04) — preamble detection timeout, in PAC units. Expiry
   raises EVENT_RXPTO. Set to 0 to disable. */
typedef uint16_t dw3000_rx_tune_pre_toc_t;

/* DTUNE3 (0x06:0C) — RX cancellation-loop tuning. Channel-specific
   constant from the user manual (typical: 0xAF5F35CC for ch5). */
typedef uint32_t dw3000_rx_tune_dtune3_t;

/* DTUNE_5 (0x06:14) — Decawave-tuned RX configuration word. */
typedef uint32_t dw3000_rx_tune_dtune5_t;

/* DRX_CAR_INT (0x06:29) — 21-bit signed carrier integrator output,
   sign-extended into int32_t. Convert to ppm using the channel/data-rate
   factor from the user manual. Read-only. */
typedef int32_t dw3000_rx_tune_carrier_int_t;

/* DGC_CFG (0x03:18) — Digital Gain Control configuration. The bulk of
   DGC tuning is loaded from OTP via OTP_CFG.DGC_KICK; this register
   holds the gating bits. */
typedef enum {
    DW3000_RX_TUNE_DGC_RX_TUNE_EN = 1U << 0,  /* enable DGC tuning      */
    DW3000_RX_TUNE_DGC_THR_64     = 1U << 12, /* threshold for 64 MHz PRF */
} dw3000_rx_tune_dgc_cfg_t;

/* DGC_DBG (0x03:60) — read-only DGC debug. DGC_DECISION (the selected
   gain-table index, 3 bits) sits at bits [30:28]. */
typedef uint32_t dw3000_rx_tune_dgc_dbg_t;

typedef struct {
    dw3000_rx_tune_sfd_toc_t sfd_toc;
    dw3000_rx_tune_pre_toc_t pre_toc;
    dw3000_rx_tune_dtune3_t  dtune3;
    dw3000_rx_tune_dtune5_t  dtune5;
    dw3000_rx_tune_dgc_cfg_t dgc_cfg;
} dw3000_rx_tune_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_RX_TUNE_H */
