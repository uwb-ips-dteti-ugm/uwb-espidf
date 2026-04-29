#ifndef DW3000_TYPES_AES_H
#define DW3000_TYPES_AES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AES_KEY (0x01:54) — 128-bit key written directly when KEY_SRC=REGISTER.
   Lower-order octets sit at lower offset addresses. */
typedef struct {
    uint8_t bytes[16];
} dw3000_aes_key_t;

/* AES_IV0..IV4 (0x01:34..42) — 16-byte concatenated initialisation
   vector / nonce. IEEE 802.15.4 CCM* uses the lower 13 bytes as nonce;
   the upper bytes carry the GCM initial counter. */
typedef struct {
    uint8_t bytes[16];
} dw3000_aes_iv_t;

/* AES_CFG.MODE (0x01:30, bit 0). */
typedef enum {
    DW3000_AES_MODE_ENCRYPT = 0x0U,
    DW3000_AES_MODE_DECRYPT = 0x1U,
} dw3000_aes_mode_t;

/* AES_CFG.KEY_SIZE (bits [2:1]). */
typedef enum {
    DW3000_AES_KEY_SIZE_128 = 0x0U,
    DW3000_AES_KEY_SIZE_192 = 0x1U,
    DW3000_AES_KEY_SIZE_256 = 0x2U,
} dw3000_aes_key_size_t;

/* AES_CFG.TAG_SIZE (bits [10:8]) — MIC length in bytes; NONE disables
   authentication. */
typedef enum {
    DW3000_AES_TAG_SIZE_NONE = 0x0U,
    DW3000_AES_TAG_SIZE_4    = 0x1U,
    DW3000_AES_TAG_SIZE_6    = 0x2U,
    DW3000_AES_TAG_SIZE_8    = 0x3U,
    DW3000_AES_TAG_SIZE_10   = 0x4U,
    DW3000_AES_TAG_SIZE_12   = 0x5U,
    DW3000_AES_TAG_SIZE_14   = 0x6U,
    DW3000_AES_TAG_SIZE_16   = 0x7U,
} dw3000_aes_tag_size_t;

/* AES_CFG.CORE_SEL (bit 11). */
typedef enum {
    DW3000_AES_CORE_GCM = 0x0U,
    DW3000_AES_CORE_CCM = 0x1U,
} dw3000_aes_core_t;

/* AES_CFG.KEY_SRC (bit 7). RAM = 16-byte slot in AES_KEY_RAM (0x17),
   indexed by KEY_ADDR. */
typedef enum {
    DW3000_AES_KEY_SRC_REGISTER = 0x0U,
    DW3000_AES_KEY_SRC_RAM      = 0x1U,
} dw3000_aes_key_src_t;

/* AES_CFG bits outside the multi-bit enum fields. KEY_ADDR (bits [5:3])
   is a 3-bit index, kept as uint8_t in the bundle below. */
typedef enum {
    DW3000_AES_CFG_KEY_LOAD = 1U << 6,  /* latch RAM key into engine */
    DW3000_AES_CFG_KEY_OTP  = 1U << 12, /* use OTP-stored key       */
} dw3000_aes_cfg_flags_t;

/* DMA_CFG.SRC_PORT / DST_PORT (3 bits) — on-chip memory selector. */
typedef enum {
    DW3000_AES_PORT_SCRATCH = 0x0U,
    DW3000_AES_PORT_RX_0    = 0x1U,
    DW3000_AES_PORT_RX_1    = 0x2U,
    DW3000_AES_PORT_TX      = 0x3U,
    DW3000_AES_PORT_NONE    = 0x7U,
} dw3000_aes_port_t;

/* DMA_CFG (0x01:44). HDR_SIZE bytes are authenticated only; PYLD_SIZE
   bytes are authenticated and (en|de)crypted. */
typedef struct {
    dw3000_aes_port_t src_port;
    uint16_t          src_addr; /* 10 bits */
    dw3000_aes_port_t dst_port;
    uint16_t          dst_addr;  /* 10 bits */
    uint8_t           hdr_size;  /*  7 bits */
    uint16_t          pyld_size; /* 10 bits */
} dw3000_aes_dma_cfg_t;

/* AES_STS (0x01:50). AES_DONE / AES_ERR also surface in SYS_STATUS as
   the EVENT_AES_DONE / EVENT_AES_ERR bits. Write-1-to-clear. */
typedef enum {
    DW3000_AES_STS_AES_DONE  = 1U << 0,
    DW3000_AES_STS_AUTH_ERR  = 1U << 1,
    DW3000_AES_STS_TRANS_ERR = 1U << 2,
    DW3000_AES_STS_MEM_CONF  = 1U << 3,
    DW3000_AES_STS_RAM_EMPTY = 1U << 4,
    DW3000_AES_STS_AES_ERR   = 1U << 5,
} dw3000_aes_status_t;

/* AES_START (0x01:4C) — write this byte to kick off one operation.
   Self-clearing; poll AES_STS or wait on EVENT_AES_DONE. */
#define DW3000_AES_START_TRIGGER 0x01U

/* AES_KEY_RAM (0x17) holds eight 16-byte slots, indexed by KEY_ADDR. */
#define DW3000_AES_KEY_RAM_SLOTS 8U

typedef struct {
    dw3000_aes_mode_t      mode;
    dw3000_aes_key_size_t  key_size;
    dw3000_aes_tag_size_t  tag_size;
    dw3000_aes_core_t      core;
    dw3000_aes_key_src_t   key_src;
    dw3000_aes_cfg_flags_t flags;
    uint8_t                key_addr; /* 0..7, used when key_src == RAM */
    dw3000_aes_key_t       key;      /* used when key_src == REGISTER  */
    dw3000_aes_iv_t        iv;
    dw3000_aes_dma_cfg_t   dma;
} dw3000_aes_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_AES_H */
