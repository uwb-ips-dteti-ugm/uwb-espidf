#ifndef DW3000_TYPES_OTP_H
#define DW3000_TYPES_OTP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* OTP_ADDR (0x0B:04) — 11-bit word address into the 2 Kword OTP. */
typedef uint16_t dw3000_otp_addr_t;

#define DW3000_OTP_ADDR_MASK 0x07FFU

/* OTP_WDATA (0x0B:00), OTP_RDATA (0x0B:10), OTP_SRDATA (0x0B:14) — all
   carry one 32-bit word per transaction. */
typedef uint32_t dw3000_otp_word_t;

/* OTP_CFG (0x0B:08) — manual access plus the *_KICK bits that copy a
   factory-calibrated block from OTP straight into its destination
   registers (DGC tuning, LDO trim, bias trim, OPS table). */
typedef enum {
    DW3000_OTP_CFG_MAN       = 1U << 0,  /* manual mode (debug)     */
    DW3000_OTP_CFG_READ      = 1U << 1,  /* trigger read at OTP_ADDR*/
    DW3000_OTP_CFG_WRITE     = 1U << 2,  /* trigger program         */
    DW3000_OTP_CFG_WRITE_MR  = 1U << 3,  /* write mode register     */
    DW3000_OTP_CFG_DGC_KICK  = 1U << 6,  /* DGC tuning load         */
    DW3000_OTP_CFG_LDO_KICK  = 1U << 7,  /* LDO trim load           */
    DW3000_OTP_CFG_BIAS_KICK = 1U << 8,  /* bias trim load          */
    DW3000_OTP_CFG_OPS_KICK  = 1U << 10, /* OPS table load          */
    DW3000_OTP_CFG_DGC_SEL   = 1U << 13, /* 0 = ch5, 1 = ch9        */
} dw3000_otp_cfg_flags_t;

/* OTP_CFG.OPS_SEL (bits [12:11]) — selects which OPS block to load on
   OPS_KICK. Values are unshifted field values. */
typedef enum {
    DW3000_OTP_OPS_SEL_LONG  = 0x0U,
    DW3000_OTP_OPS_SEL_SHORT = 0x2U,
} dw3000_otp_ops_sel_t;

#define DW3000_OTP_OPS_SEL_SHIFT 11U
#define DW3000_OTP_OPS_SEL_MASK  (0x3U << DW3000_OTP_OPS_SEL_SHIFT)

/* OTP_STAT (0x0B:0C) — programming status. VPP_OK must be high before
   asserting OTP_CFG.WRITE; PROG_DONE rises after a successful program. */
typedef enum {
    DW3000_OTP_STAT_PROG_DONE = 1U << 0,
    DW3000_OTP_STAT_VPP_OK    = 1U << 1,
} dw3000_otp_status_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_OTP_H */
