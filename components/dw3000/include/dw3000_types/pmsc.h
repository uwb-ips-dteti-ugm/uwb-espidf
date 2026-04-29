#ifndef DW3000_TYPES_PMSC_H
#define DW3000_TYPES_PMSC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SOFT_RST (0x11:00). Each bit gates one subsystem: 1 = running,
   0 = held in reset. Power-on default is 0x01FF. Pulse a block by
   clearing its bit and setting it back. */
typedef enum {
    DW3000_PMSC_SOFT_RST_ARM  = 1U << 0,
    DW3000_PMSC_SOFT_RST_PRGN = 1U << 1,
    DW3000_PMSC_SOFT_RST_CIA  = 1U << 2,
    DW3000_PMSC_SOFT_RST_BIST = 1U << 3,
    DW3000_PMSC_SOFT_RST_RX   = 1U << 4,
    DW3000_PMSC_SOFT_RST_TX   = 1U << 5,
    DW3000_PMSC_SOFT_RST_HIF  = 1U << 6,
    DW3000_PMSC_SOFT_RST_PMSC = 1U << 7,
    DW3000_PMSC_SOFT_RST_GPIO = 1U << 8,
} dw3000_pmsc_soft_rst_t;

#define DW3000_PMSC_SOFT_RST_MASK     0x01FFU
#define DW3000_PMSC_SOFT_RST_ALL_RUN  0x01FFU
#define DW3000_PMSC_SOFT_RST_ALL_HOLD 0x0000U

/* CLK_CTRL.SYS_CLK (bits [1:0]) / RX_CLK ([3:2]) / TX_CLK ([5:4]) —
   clock-source selector. SYS_CLK values are source selectors; RX/TX values
   other than AUTO force the PLL-derived clock on for that block. */
typedef enum {
    DW3000_PMSC_CLK_SRC_AUTO             = 0x0U,
    DW3000_PMSC_CLK_SRC_FAST_RC_DIV4     = 0x1U, /* SYS_CLK only */
    DW3000_PMSC_CLK_SRC_PLL              = 0x2U, /* SYS_CLK only */
    DW3000_PMSC_CLK_SRC_FAST_RC          = 0x3U, /* SYS_CLK only */
    DW3000_PMSC_CLK_SRC_FORCE_PLL_LOW    = 0x1U, /* RX/TX_CLK only */
    DW3000_PMSC_CLK_SRC_FORCE_PLL_MEDIUM = 0x2U, /* RX/TX_CLK only */
    DW3000_PMSC_CLK_SRC_FORCE_PLL_HIGH   = 0x3U, /* RX/TX_CLK only */
} dw3000_pmsc_clk_src_t;

/* CLK_CTRL (0x11:04) bits outside the SYS/RX/TX_CLK selectors. */
typedef enum {
    DW3000_PMSC_CLK_ACC_CLK_EN   = 1U << 6,
    DW3000_PMSC_CLK_CIA_CLK_EN   = 1U << 8,
    DW3000_PMSC_CLK_SAR_CLK_EN   = 1U << 10,
    DW3000_PMSC_CLK_ACC_MCLK_EN  = 1U << 15,
    DW3000_PMSC_CLK_GPIO_CLK_EN  = 1U << 16,
    DW3000_PMSC_CLK_GPIO_DCLK_EN = 1U << 18,
    DW3000_PMSC_CLK_GPIO_DRST_N  = 1U << 19,
    DW3000_PMSC_CLK_LP_CLK_EN    = 1U << 23,
} dw3000_pmsc_clk_flags_t;

typedef struct {
    dw3000_pmsc_clk_src_t   sys_clk;
    dw3000_pmsc_clk_src_t   rx_clk;
    dw3000_pmsc_clk_src_t   tx_clk;
    dw3000_pmsc_clk_flags_t flags;
} dw3000_pmsc_clk_ctrl_t;

/* SEQ_CTRL (0x11:08) — power/state-machine sequencer. ATXSLP / ARXSLP
   require AON sleep configuration to be valid before they fire. */
typedef enum {
    DW3000_PMSC_SEQ_AINIT2IDLE = 1U << 8,  /* auto IDLE_RC -> IDLE_PLL      */
    DW3000_PMSC_SEQ_ATX2SLP    = 1U << 11, /* auto sleep after TX           */
    DW3000_PMSC_SEQ_ARX2SLP    = 1U << 12, /* auto sleep after RX           */
    DW3000_PMSC_SEQ_PLL_SYNC   = 1U << 15, /* 1 GHz clock for SYNC modes    */
    DW3000_PMSC_SEQ_CIARUNE    = 1U << 17, /* run CIA automatically         */
    DW3000_PMSC_SEQ_FORCE2INIT = 1U << 23, /* force FSM back to IDLE_RC     */
    DW3000_PMSC_SEQ_ATXSLP     = DW3000_PMSC_SEQ_ATX2SLP,
    DW3000_PMSC_SEQ_ARXSLP     = DW3000_PMSC_SEQ_ARX2SLP,
} dw3000_pmsc_seq_ctrl_flags_t;

/* TXFSEQ (0x11:12) — TX fine-sequencer timing word. Decawave-tuned;
   typically programmed from a known-good constant rather than authored. */
typedef uint32_t dw3000_pmsc_txfseq_t;

#define DW3000_PMSC_TXFSEQ_FINE_ENABLED  0x04D28874UL
#define DW3000_PMSC_TXFSEQ_FINE_DISABLED 0x00D20874UL

/* LED_CTRL (0x11:16). BLINK_TIM is in units of 14 ms (LP-osc derived);
   BLNKNOW_* bits are write-1 to force a one-shot blink. */
typedef enum {
    DW3000_PMSC_LED_BLNKEN       = 1U << 8,
    DW3000_PMSC_LED_BLNKNOW_RXOK = 1U << 16,
    DW3000_PMSC_LED_BLNKNOW_SFD  = 1U << 17,
    DW3000_PMSC_LED_BLNKNOW_RX   = 1U << 18,
    DW3000_PMSC_LED_BLNKNOW_TX   = 1U << 19,
} dw3000_pmsc_led_flags_t;

#define DW3000_PMSC_LED_BLINK_TIM_DEFAULT 0x20U

typedef struct {
    uint8_t                 blink_tim; /* 8 bits, ~14 ms units */
    dw3000_pmsc_led_flags_t flags;
} dw3000_pmsc_led_ctrl_t;

/* RX_SNIFF (0x11:1A) is owned by txrx.h as dw3000_txrx_sniff_t. */

/* BIAS_CTRL (0x11:1F) — analog bias-current trim. Programmed from
   OTP-stored calibration; not a user-tunable knob. */
typedef uint16_t dw3000_pmsc_bias_ctrl_t;

#define DW3000_PMSC_BIAS_CTRL_MASK 0x1FFFU

typedef struct {
    dw3000_pmsc_clk_ctrl_t       clk_ctrl;
    dw3000_pmsc_seq_ctrl_flags_t seq_flags;
    dw3000_pmsc_txfseq_t         txfseq;
    dw3000_pmsc_led_ctrl_t       led_ctrl;
    dw3000_pmsc_bias_ctrl_t      bias_ctrl;
} dw3000_pmsc_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_PMSC_H */
