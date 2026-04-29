#ifndef DW3000_TYPES_CIA_H
#define DW3000_TYPES_CIA_H

#include <stdbool.h>
#include <stdint.h>

#include "txrx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TX_ANTD (0x01:04) and CIA_CONF.RX_ANTD (0x0E:00, bits [15:0]) — 16-bit
   antenna delay in DW3000 system-time units (~15.65 ps). Programmed from
   OTP-calibrated values; subtracted from the raw timestamp. */
typedef uint16_t dw3000_cia_antenna_delay_t;

/* IP_TS (0x0C:00), STS_TS (0x0C:08), STS1_TS (0x0C:10). Each is an 8-byte
   block holding a 40-bit segment time-of-arrival plus 14-bit signed
   phase-of-arrival. The post-CIA aggregated RX timestamp lives in
   RX_TIME (TXRX layer), not here. */
typedef struct {
    dw3000_txrx_timestamp_t toa; /* 40-bit, ~15.65 ps units */
    int16_t                 poa; /* 14-bit signed           */
} dw3000_cia_path_ts_t;

/* TDOA (0x0C:18) — 41-bit signed difference between IP TOA and STS TOA,
   sign-extended into int64_t. */
typedef int64_t dw3000_cia_tdoa_t;

/* PDOA (0x0C:1E). FP_TH_MD reflects whether the first-path threshold was
   satisfied for the STS receive that produced this PDOA. */
typedef struct {
    int16_t value;    /* PDOA, 14 bits signed */
    bool    fp_th_md; /* FP_TH_MD             */
} dw3000_cia_pdoa_t;

/* CIA_DIAG_0 / CIA_DIAG_1 (0x0C:20, 0x0C:24). CIA_DIAG_0 exposes the
   carrier offset estimate; CIA_DIAG_1 is reserved but surfaced raw so the
   register can be mirrored without inventing semantics. */
typedef struct {
    int16_t  clock_offset; /* COE_PPM, 13-bit signed fixed-point raw value */
    uint32_t reserved;     /* CIA_DIAG_1 raw reserved diagnostic value     */
} dw3000_cia_diag_t;

/* IP_DIAG_* (0x0C:28..58), STS_DIAG_* (0x0C:5C..0x0D:20), STS1_DIAG_*
   (0x0D:38..68) all share the same layout. Only the fields needed for
   first-path power and CIR peak inspection are surfaced; the rest of the
   diagnostic registers stay reachable via reg.h. */
typedef struct {
    uint16_t fp_index;       /* first-path index, Q10.6 or Q9.6      */
    uint32_t cir_pwr;        /* CIR power/channel area, 16/17 bits   */
    uint32_t fp_ampl1;       /* first-path amplitude, sample n       */
    uint32_t fp_ampl2;       /* first-path amplitude, sample n+1     */
    uint32_t fp_ampl3;       /* first-path amplitude, sample n+2     */
    uint32_t peak_amplitude; /* CIR peak amplitude, 21 bits          */
    uint16_t peak_index;     /* CIR peak index                       */
    uint16_t pacc_nosat;     /* accumulated symbol/chip count        */
} dw3000_cia_path_diag_t;

/* CIA_CONF (0x0E:00) bits outside RX_ANTD. HAL merges with RX_ANTD on
   write. */
typedef enum {
    DW3000_CIA_CONF_MINDIAG = 1U << 20, /* minimal diagnostics output */
} dw3000_cia_conf_flags_t;

/* FP_CONF (0x0E:04) — first-path threshold tuning. */
typedef struct {
    uint8_t fp_agreed_th; /* FP_AGREED_TH, 3 bits */
    uint8_t tc_rcfg;      /* TC_RCFG,      8 bits */
} dw3000_cia_fp_conf_t;

/* IP_CONF (0x0E:0C) — preamble CIA tuning. */
typedef struct {
    uint8_t ntm;   /* IP_NTM,   5 bits */
    uint8_t pmult; /* IP_PMULT, 2 bits */
    uint8_t rtm;   /* IP_RTM,   5 bits */
} dw3000_cia_ip_conf_t;

/* STS_CONF_0 (0x0E:12) + STS_CONF_1 (0x0E:16) — STS CIA tuning. */
typedef struct {
    uint8_t  ntm;   /* STS_NTM,    5 bits  */
    uint8_t  pmult; /* STS_PMULT,  2 bits  */
    uint8_t  rtm;   /* reserved by current register map; keep 0 */
    uint16_t mnth;  /* STS_MNTH,   7 bits  */
    bool     cq_en; /* STS_CQ_EN           */
} dw3000_cia_sts_conf_t;

/* CIA_ADJUST (0x0E:1A) — 14-bit unsigned PDoA angle adjustment. */
typedef uint16_t dw3000_cia_adjust_t;

typedef struct {
    dw3000_cia_antenna_delay_t rx_antd;
    dw3000_cia_antenna_delay_t tx_antd;
    dw3000_cia_conf_flags_t    conf_flags;
    dw3000_cia_fp_conf_t       fp_conf;
    dw3000_cia_ip_conf_t       ip_conf;
    dw3000_cia_sts_conf_t      sts_conf;
    dw3000_cia_adjust_t        adjust;
} dw3000_cia_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_CIA_H */
