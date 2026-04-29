#ifndef DW3000_TYPES_PHY_H
#define DW3000_TYPES_PHY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CHAN_CTRL.RF_CHAN (1 bit). */
typedef enum {
    DW3000_PHY_CHANNEL_5 = 0x0U,
    DW3000_PHY_CHANNEL_9 = 0x1U,
} dw3000_phy_channel_t;

/* RX_FINFO.RXPRF (2 bits). PRF is implied by preamble code on TX, but the
   chosen value must be consistent across both ends of the link. */
typedef enum {
    DW3000_PHY_PRF_16_MHZ = 0x0U,
    DW3000_PHY_PRF_64_MHZ = 0x1U,
} dw3000_phy_prf_t;

/* TX_FCTRL.TXBR (1 bit). Also reflected in RX_FINFO.RXBR. */
typedef enum {
    DW3000_PHY_DATA_RATE_850K = 0x0U,
    DW3000_PHY_DATA_RATE_6M81 = 0x1U,
} dw3000_phy_data_rate_t;

/* TX_FCTRL.TXPSR (4 bits, Table 20). FINE_PLEN-only sub-symbol-count
   lengths are not represented here; use FINE_PLEN directly when needed. */
typedef enum {
    DW3000_PHY_PREAMBLE_LEN_64   = 0x1U,
    DW3000_PHY_PREAMBLE_LEN_1024 = 0x2U,
    DW3000_PHY_PREAMBLE_LEN_4096 = 0x3U,
    DW3000_PHY_PREAMBLE_LEN_32   = 0x4U,
    DW3000_PHY_PREAMBLE_LEN_128  = 0x5U,
    DW3000_PHY_PREAMBLE_LEN_1536 = 0x6U,
    DW3000_PHY_PREAMBLE_LEN_256  = 0x9U,
    DW3000_PHY_PREAMBLE_LEN_2048 = 0xAU,
    DW3000_PHY_PREAMBLE_LEN_512  = 0xDU,
} dw3000_phy_preamble_length_t;

/* CHAN_CTRL.SFD_TYPE (2 bits). */
typedef enum {
    DW3000_PHY_SFD_TYPE_IEEE_802154_SHORT = 0x0U,
    DW3000_PHY_SFD_TYPE_DECAWAVE_8        = 0x1U,
    DW3000_PHY_SFD_TYPE_DECAWAVE_16       = 0x2U,
    DW3000_PHY_SFD_TYPE_IEEE_802154Z      = 0x3U,
} dw3000_phy_sfd_type_t;

/* DTUNE0.PAC (2 bits). Pick PAC per preamble length per Table 12. */
typedef enum {
    DW3000_PHY_PAC_SIZE_8  = 0x0U,
    DW3000_PHY_PAC_SIZE_16 = 0x1U,
    DW3000_PHY_PAC_SIZE_32 = 0x2U,
    DW3000_PHY_PAC_SIZE_4  = 0x3U,
} dw3000_phy_pac_size_t;

/* SYS_CFG.PHR_MODE (1 bit). Standard caps payload at 127 octets;
   extended caps at 1023 octets. Both ends must match. */
typedef enum {
    DW3000_PHY_PHR_MODE_STANDARD = 0x0U,
    DW3000_PHY_PHR_MODE_EXTENDED = 0x1U,
} dw3000_phy_phr_mode_t;

/* SYS_CFG.PHR_6M8 (1 bit). Only meaningful at the 6.81 Mb/s data rate. */
typedef enum {
    DW3000_PHY_PHR_RATE_850K = 0x0U,
    DW3000_PHY_PHR_RATE_6M81 = 0x1U,
} dw3000_phy_phr_rate_t;

typedef struct {
    dw3000_phy_channel_t         channel;
    dw3000_phy_prf_t             prf;
    dw3000_phy_data_rate_t       data_rate;
    dw3000_phy_preamble_length_t preamble_length;
    dw3000_phy_sfd_type_t        sfd_type;
    dw3000_phy_pac_size_t        pac_size;
    dw3000_phy_phr_mode_t        phr_mode;
    dw3000_phy_phr_rate_t        phr_rate;
    uint8_t                      tx_preamble_code; /* 1..29, see CHAN_CTRL */
    uint8_t                      rx_preamble_code; /* 1..29, see CHAN_CTRL */
} dw3000_phy_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_PHY_H */
