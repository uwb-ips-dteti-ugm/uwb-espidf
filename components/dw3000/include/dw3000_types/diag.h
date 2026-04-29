#ifndef DW3000_TYPES_DIAG_H
#define DW3000_TYPES_DIAG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EVC_CTRL (0x0F:00). EVC_CLR is self-clearing — write 1 to zero all
   event counters in this file. */
typedef enum {
    DW3000_DIAG_EVC_EN  = 1U << 0, /* enable counter accumulation */
    DW3000_DIAG_EVC_CLR = 1U << 1, /* clear all counters          */
} dw3000_diag_evc_ctrl_t;

/* Snapshot of all DIG_DIAG event counters (0x0F:04..0x0F:2A). All counters
   saturate at their bit width and freeze when EVC_EN=0. */
typedef struct {
    uint16_t phe;   /* EVC_PHE   PHR errors                12-bit */
    uint16_t rse;   /* EVC_RSE   Reed–Solomon decode errs  12-bit */
    uint16_t fcg;   /* EVC_FCG   frames received good FCS  12-bit */
    uint16_t fce;   /* EVC_FCE   frames received bad FCS   12-bit */
    uint8_t  ffr;   /* EVC_FFR   frame filter rejections    8-bit */
    uint8_t  ovr;   /* EVC_OVR   RX overruns                8-bit */
    uint16_t sto;   /* EVC_STO   SFD timeouts              12-bit */
    uint16_t pto;   /* EVC_PTO   preamble timeouts         12-bit */
    uint8_t  fwto;  /* EVC_FWTO  RX frame wait timeouts     8-bit */
    uint16_t txfs;  /* EVC_TXFS  TX frames sent            12-bit */
    uint8_t  hpw;   /* EVC_HPW   half-period warnings       8-bit */
    uint8_t  swce;  /* EVC_SWCE  SPI write CRC errors       8-bit */
    uint8_t  cpqe;  /* EVC_CPQE  CIA preamble quality errs  8-bit */
    uint8_t  vwarn; /* EVC_VWARN low-voltage warnings       8-bit */
} dw3000_diag_event_counters_t;

/* DIAG_TMC (0x0F:24) — diagnostic test-mode control. Most bits are
   reserved / Decawave internal; only the well-documented controls are
   surfaced. Treat the remainder as 0 unless the manual says otherwise. */
typedef enum {
    DW3000_DIAG_TMC_TX_PSTM = 1U << 4, /* continuous TX spectrum test    */
} dw3000_diag_tmc_flags_t;

/* SYS_STATE (0x0F:30) — current FSM states. Read-only. Three separate
   state machines packed into one 32-bit word. */
typedef struct {
    uint8_t tx_state;   /* bits [3:0]   */
    uint8_t rx_state;   /* bits [13:8]  */
    uint8_t pmsc_state; /* bits [19:16] */
} dw3000_diag_sys_state_t;

/* SPI_MODE (0x0F:2C) — observed SPI mode (CPOL/CPHA), bits [1:0]. */
typedef uint8_t dw3000_diag_spi_mode_t;

/* FCMD_STAT (0x0F:3C) — last fast command opcode received. 5 bits. */
typedef uint8_t dw3000_diag_fcmd_stat_t;

/* CTR_DBG (0x0F:48) — current value of the internal timestamp counter,
   sampled when read; for debugging the system-time domain. */
typedef uint32_t dw3000_diag_ctr_dbg_t;

/* SPICRCINIT (0x0F:4C) — 8-bit seed for the SPI write-CRC engine. Must
   match the host-side initial value. */
typedef uint8_t dw3000_diag_spicrc_init_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_DIAG_H */
