#ifndef DW3000_TYPES_STS_H
#define DW3000_TYPES_STS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* STS_KEY (0x02:0C) — 128-bit AES key for STS generation. Lower-order
   octets sit at lower offset addresses. */
typedef struct {
    uint8_t bytes[16];
} dw3000_sts_key_t;

/* STS_IV (0x02:1C) — 128-bit AES initialisation vector / counter. */
typedef struct {
    uint8_t bytes[16];
} dw3000_sts_iv_t;

/* STS_STS.ACC_QUAL (0x02:08, bits [11:0]) — accumulation quality. Check
   before accepting an STS-derived RX timestamp. */
typedef uint16_t dw3000_sts_acc_qual_t;

/* SYS_CFG.CP_SPC (0x00:10, bits [13:12]) — STS placement within packet. */
typedef enum {
    DW3000_STS_PACKET_CFG_SP0 = 0x0U, /* No STS                                        */
    DW3000_STS_PACKET_CFG_SP1 = 0x1U, /* STS between SFD and PHR                       */
    DW3000_STS_PACKET_CFG_SP2 = 0x2U, /* STS after data (data length must be non-zero) */
    DW3000_STS_PACKET_CFG_SP3 = 0x3U, /* STS after SFD, no PHR/data                    */
} dw3000_sts_packet_cfg_t;

/* SYS_CFG.PDOA_MODE (0x00:10, bits [17:16]). Mode 3 requires CP_SPC=SP3. */
typedef enum {
    DW3000_STS_PDOA_MODE_DISABLED = 0x0U,
    DW3000_STS_PDOA_MODE_1        = 0x1U,
    DW3000_STS_PDOA_MODE_3        = 0x3U,
} dw3000_sts_pdoa_mode_t;

/* STS-owned bits of SYS_CFG (0x00:10). HAL merges with PHY/MAC/TXRX
   bits owned by their respective headers. */
typedef enum {
    DW3000_STS_SYS_CFG_CIA_STS = 1U << 8,  /* CIA processes STS CIR    */
    DW3000_STS_SYS_CFG_CP_SDC  = 1U << 15, /* Super Deterministic Code */
} dw3000_sts_sys_cfg_flags_t;

/* STS_CTRL (0x02:04). Both bits self-clearing. Set CP_SPC before
   asserting LOAD_IV. */
typedef enum {
    DW3000_STS_CTRL_LOAD_IV  = 1U << 0,
    DW3000_STS_CTRL_RST_LAST = 1U << 1,
} dw3000_sts_ctrl_flags_t;

typedef struct {
    uint8_t                    cps_len; /* STS_CFG.CPS_LEN, 8 bits, blocks of 8 */
    dw3000_sts_key_t           key;
    dw3000_sts_iv_t            iv;
    dw3000_sts_packet_cfg_t    packet_cfg;
    dw3000_sts_pdoa_mode_t     pdoa_mode;
    dw3000_sts_sys_cfg_flags_t sys_cfg_flags;
} dw3000_sts_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_STS_H */
