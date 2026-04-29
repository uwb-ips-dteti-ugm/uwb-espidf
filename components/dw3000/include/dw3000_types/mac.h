#ifndef DW3000_TYPES_MAC_H
#define DW3000_TYPES_MAC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t dw3000_mac_eui_t;
typedef uint16_t dw3000_mac_pan_id_t;
typedef uint16_t dw3000_mac_short_addr_t;

/* Mirrors PANADR (0x00:0C): low half PAN ID, high half short address. */
typedef struct {
    dw3000_mac_pan_id_t     pan_id;
    dw3000_mac_short_addr_t short_addr;
} dw3000_mac_panadr_t;

/* IEEE 802.15.4 frame type — bits [2:0] of the Frame Control field. */
typedef enum {
    DW3000_MAC_FRAME_TYPE_BEACON       = 0x0U,
    DW3000_MAC_FRAME_TYPE_DATA         = 0x1U,
    DW3000_MAC_FRAME_TYPE_ACK          = 0x2U,
    DW3000_MAC_FRAME_TYPE_MAC_CMD      = 0x3U,
    DW3000_MAC_FRAME_TYPE_RESERVED     = 0x4U,
    DW3000_MAC_FRAME_TYPE_MULTIPURPOSE = 0x5U,
    DW3000_MAC_FRAME_TYPE_FRAGMENT     = 0x6U,
    DW3000_MAC_FRAME_TYPE_EXTENDED     = 0x7U,
} dw3000_mac_frame_type_t;

/* FF_CFG (0x00:14) — frame filter rules. ORed together; the resulting
   uint16_t is written directly to FF_CFG. FFEN in SYS_CFG must also be
   set for any of these to take effect. */
typedef enum {
    DW3000_MAC_FF_ALLOW_BEACON       = 1U << 0, /* FFAB     */
    DW3000_MAC_FF_ALLOW_DATA         = 1U << 1, /* FFAD     */
    DW3000_MAC_FF_ALLOW_ACK          = 1U << 2, /* FFAA     */
    DW3000_MAC_FF_ALLOW_MAC_CMD      = 1U << 3, /* FFAM     */
    DW3000_MAC_FF_ALLOW_RESERVED     = 1U << 4, /* FFAR     */
    DW3000_MAC_FF_ALLOW_MULTIPURPOSE = 1U << 5, /* FFAMULTI */
    DW3000_MAC_FF_ALLOW_FRAGMENT     = 1U << 6, /* FFAF     */
    DW3000_MAC_FF_ALLOW_EXTENDED     = 1U << 7, /* FFAE     */
    DW3000_MAC_FF_BEHAVE_PAN_COORD   = 1U << 8, /* FFBC     */
    DW3000_MAC_FF_IMPLICIT_BROADCAST = 1U << 9, /* FFIB     */
    DW3000_MAC_FF_LE0_PEND           = 1U << 10,
    DW3000_MAC_FF_LE1_PEND           = 1U << 11,
    DW3000_MAC_FF_LE2_PEND           = 1U << 12,
    DW3000_MAC_FF_LE3_PEND           = 1U << 13,
    DW3000_MAC_FF_SHORT_SRC_PEND_ACK = 1U << 14, /* SSADRAPE */
    DW3000_MAC_FF_LONG_SRC_PEND_ACK  = 1U << 15, /* LSADRAPE */
} dw3000_mac_ff_flags_t;

/* MAC-owned bits of SYS_CFG (0x00:10). HAL merges with PHY/RX/STS/SPI
   bits owned by their respective headers. */
typedef enum {
    DW3000_MAC_SYS_CFG_FF_ENABLE = 1U << 0,  /* FFEN     */
    DW3000_MAC_SYS_CFG_AUTO_ACK  = 1U << 11, /* AUTO_ACK */
    DW3000_MAC_SYS_CFG_FAST_AAT  = 1U << 18, /* FAST_AAT */
} dw3000_mac_sys_cfg_flags_t;

/* ACK_RESP_T (0x01:08). Auto-ACK turnaround in preamble symbols;
   wait-for-response time in ~1 us units (128 sysclk cycles). */
typedef struct {
    uint8_t  ack_tim; /* 8-bit  */
    uint32_t w4r_tim; /* 20-bit */
} dw3000_mac_ack_resp_t;

/* LE_PEND_01 (0x01:18) and LE_PEND_23 (0x01:1C) — four short addresses
   indexed by the matching LEx_PEND bit in dw3000_mac_ff_flags_t. */
typedef struct {
    dw3000_mac_short_addr_t addr[4];
} dw3000_mac_le_pend_t;

typedef struct {
    dw3000_mac_eui_t           eui;
    dw3000_mac_panadr_t        panadr;
    dw3000_mac_ff_flags_t      ff_flags;
    dw3000_mac_sys_cfg_flags_t sys_cfg_flags;
    dw3000_mac_ack_resp_t      ack_resp;
    dw3000_mac_le_pend_t       le_pend;
} dw3000_mac_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_MAC_H */
