#ifndef DW3000_TYPES_TXRX_H
#define DW3000_TYPES_TXRX_H

#include <stdbool.h>
#include <stdint.h>

#include "phy.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 40-bit DW3000 system-time domain (LSB ~15.65 ps), zero-extended to 64. */
typedef uint64_t dw3000_txrx_timestamp_t;

/* DX_TIME / DREF_TIME — 32-bit delayed-time value, units of ~4 ns
   (high 32 bits of system time). LSB is ignored by hardware. */
typedef uint32_t dw3000_txrx_delayed_time_t;

/* RX_FWTO — 20-bit frame wait timeout, units of ~1.0256 us. */
typedef uint32_t dw3000_txrx_fwto_t;

/* TXRX-owned bits of SYS_CFG (0x00:10). HAL merges with PHY/MAC/etc.
   bits owned by their respective headers. */
typedef enum {
    DW3000_TXRX_SYS_CFG_DIS_FCS_TX = 1U << 1,  /* DIS_FCS_TX */
    DW3000_TXRX_SYS_CFG_DIS_FCE    = 1U << 2,  /* DIS_FCE    */
    DW3000_TXRX_SYS_CFG_DIS_DRXB   = 1U << 3,  /* DIS_DRXB   */
    DW3000_TXRX_SYS_CFG_RXWTOE     = 1U << 9,  /* RXWTOE     */
    DW3000_TXRX_SYS_CFG_RXAUTR     = 1U << 10, /* RXAUTR     */
} dw3000_txrx_sys_cfg_flags_t;

/* SYS_STATUS (0x00:44) / SYS_ENABLE (0x00:3C). Both registers share the
   same 48-bit layout; this bitmap describes both. Bits 32–47 correspond
   to octets 4 and 5 of the register. */
typedef uint64_t dw3000_txrx_event_t;

#define DW3000_TXRX_EVENT_IRQS     (UINT64_C(1) << 0)
#define DW3000_TXRX_EVENT_CPLOCK   (UINT64_C(1) << 1)
#define DW3000_TXRX_EVENT_SPICRCE  (UINT64_C(1) << 2)
#define DW3000_TXRX_EVENT_AAT      (UINT64_C(1) << 3)
#define DW3000_TXRX_EVENT_TXFRB    (UINT64_C(1) << 4)
#define DW3000_TXRX_EVENT_TXPRS    (UINT64_C(1) << 5)
#define DW3000_TXRX_EVENT_TXPHS    (UINT64_C(1) << 6)
#define DW3000_TXRX_EVENT_TXFRS    (UINT64_C(1) << 7)
#define DW3000_TXRX_EVENT_RXPRD    (UINT64_C(1) << 8)
#define DW3000_TXRX_EVENT_RXSFDD   (UINT64_C(1) << 9)
#define DW3000_TXRX_EVENT_CIADONE  (UINT64_C(1) << 10)
#define DW3000_TXRX_EVENT_RXPHD    (UINT64_C(1) << 11)
#define DW3000_TXRX_EVENT_RXPHE    (UINT64_C(1) << 12)
#define DW3000_TXRX_EVENT_RXFR     (UINT64_C(1) << 13)
#define DW3000_TXRX_EVENT_RXFCG    (UINT64_C(1) << 14)
#define DW3000_TXRX_EVENT_RXFCE    (UINT64_C(1) << 15)
#define DW3000_TXRX_EVENT_RXFSL    (UINT64_C(1) << 16)
#define DW3000_TXRX_EVENT_RXFTO    (UINT64_C(1) << 17)
#define DW3000_TXRX_EVENT_CIAERR   (UINT64_C(1) << 18)
#define DW3000_TXRX_EVENT_VWARN    (UINT64_C(1) << 19)
#define DW3000_TXRX_EVENT_RXOVRR   (UINT64_C(1) << 20)
#define DW3000_TXRX_EVENT_RXPTO    (UINT64_C(1) << 21)
#define DW3000_TXRX_EVENT_SPIRDY   (UINT64_C(1) << 23)
#define DW3000_TXRX_EVENT_RCINIT   (UINT64_C(1) << 24)
#define DW3000_TXRX_EVENT_PLL_HILO (UINT64_C(1) << 25)
#define DW3000_TXRX_EVENT_RXSTO    (UINT64_C(1) << 26)
#define DW3000_TXRX_EVENT_HPDWARN  (UINT64_C(1) << 27)
#define DW3000_TXRX_EVENT_CPERR    (UINT64_C(1) << 28)
#define DW3000_TXRX_EVENT_ARFE     (UINT64_C(1) << 29)
#define DW3000_TXRX_EVENT_RXPREJ   (UINT64_C(1) << 33)
#define DW3000_TXRX_EVENT_VT_DET   (UINT64_C(1) << 36)
#define DW3000_TXRX_EVENT_GPIOIRQ  (UINT64_C(1) << 37)
#define DW3000_TXRX_EVENT_AES_DONE (UINT64_C(1) << 38)
#define DW3000_TXRX_EVENT_AES_ERR  (UINT64_C(1) << 39)
#define DW3000_TXRX_EVENT_CMD_ERR  (UINT64_C(1) << 40)
#define DW3000_TXRX_EVENT_SPI_OVF  (UINT64_C(1) << 41)
#define DW3000_TXRX_EVENT_SPI_UNF  (UINT64_C(1) << 42)
#define DW3000_TXRX_EVENT_SPIERR   (UINT64_C(1) << 43)
#define DW3000_TXRX_EVENT_CCA_FAIL (UINT64_C(1) << 44)

/* FINT_STAT (0x1F:00). Reduced 8-bit aggregate of SYS_STATUS events;
   read-only — clear by writing into the underlying SYS_STATUS bits. */
typedef enum {
    DW3000_TXRX_FINT_TXOK      = 1U << 0,
    DW3000_TXRX_FINT_CCA_FAIL  = 1U << 1,
    DW3000_TXRX_FINT_RXTSERR   = 1U << 2,
    DW3000_TXRX_FINT_RXOK      = 1U << 3,
    DW3000_TXRX_FINT_RXERR     = 1U << 4,
    DW3000_TXRX_FINT_RXTO      = 1U << 5,
    DW3000_TXRX_FINT_SYS_EVENT = 1U << 6,
    DW3000_TXRX_FINT_SYS_PANIC = 1U << 7,
} dw3000_txrx_fint_t;

/* TX_FCTRL (0x00:24) per-frame fields. PHY-derived fields (TXBR, TXPSR)
   come from dw3000_phy_config_t and are merged in by the HAL. */
typedef struct {
    uint16_t tx_flen;     /* TXFLEN, 10 bits — payload + 2-byte CRC unless DIS_FCS_TX */
    bool     ranging;     /* TR bit                                                   */
    uint16_t tx_b_offset; /* TXB_OFFSET, 10 bits                                      */
    uint8_t  fine_plen;   /* FINE_PLEN, 0 = use PHY preamble length                   */
} dw3000_txrx_tx_frame_t;

/* RX_FINFO (0x00:4C) decoded fields. RXPSR/RXNSPL combine into preamble
   length per Table 21 — kept raw so consumers can decode if needed. */
typedef struct {
    uint16_t               rx_flen;   /* RXFLEN, 10 bits, includes 2-byte FCS */
    bool                   ranging;   /* RNG bit from received PHR            */
    dw3000_phy_data_rate_t data_rate; /* RXBR                                 */
    dw3000_phy_prf_t       prf;       /* RXPRF                                */
    uint8_t                rx_psr;    /* RXPSR, 2 bits                        */
    uint8_t                rx_nspl;   /* RXNSPL, 2 bits                       */
    uint16_t               rx_pacc;   /* RXPACC, 12 bits                      */
} dw3000_txrx_rx_finfo_t;

/* RDB_STATUS (0x01:24) — per-buffer flags for double-buffered RX. */
typedef enum {
    DW3000_TXRX_RDB_RXFCG0   = 1U << 0,
    DW3000_TXRX_RDB_RXFR0    = 1U << 1,
    DW3000_TXRX_RDB_CIADONE0 = 1U << 2,
    DW3000_TXRX_RDB_CP_ERR0  = 1U << 3,
    DW3000_TXRX_RDB_RXFCG1   = 1U << 4,
    DW3000_TXRX_RDB_RXFR1    = 1U << 5,
    DW3000_TXRX_RDB_CIADONE1 = 1U << 6,
    DW3000_TXRX_RDB_CP_ERR1  = 1U << 7,
} dw3000_txrx_rdb_status_t;

/* RDB_DIAG (0x01:28) RDB_DMODE field — selects how much diagnostic data
   the double-buffer hardware logs per frame. */
typedef enum {
    DW3000_TXRX_RDB_DMODE_MINIMAL = 0x1U,
    DW3000_TXRX_RDB_DMODE_MEDIUM  = 0x2U,
    DW3000_TXRX_RDB_DMODE_FULL    = 0x4U,
} dw3000_txrx_rdb_dmode_t;

/* RX_SNIFF (0x11:1A) — low-power preamble-hunt duty cycling. on=0
   disables SNIFF mode; otherwise minimum on=2 for valid detection. */
typedef struct {
    uint8_t on;  /* SNIFF_ON,  4 bits, PAC units    */
    uint8_t off; /* SNIFF_OFF, 8 bits, ~1 us units  */
} dw3000_txrx_sniff_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_TXRX_H */
