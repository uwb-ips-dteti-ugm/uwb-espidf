#ifndef DW3000_TYPES_PHY_H
#define DW3000_TYPES_PHY_H

#include <stdbool.h>
#include <stdint.h>

#include "address.h"
#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DW3000_PHY_PREAMBLE_LEN_32   = 32,
    DW3000_PHY_PREAMBLE_LEN_64   = 64,
    DW3000_PHY_PREAMBLE_LEN_128  = 128,
    DW3000_PHY_PREAMBLE_LEN_256  = 256,
    DW3000_PHY_PREAMBLE_LEN_512  = 512,
    DW3000_PHY_PREAMBLE_LEN_1024 = 1024,
    DW3000_PHY_PREAMBLE_LEN_1536 = 1536,
    DW3000_PHY_PREAMBLE_LEN_2048 = 2048,
    DW3000_PHY_PREAMBLE_LEN_4096 = 4096,
} dw3000_phy_preamble_length_t;

typedef enum {
    DW3000_SFD_TYPE_IEEE_802154_SHORT = 0,
    DW3000_SFD_TYPE_DECAWAVE_8        = 1,
    DW3000_SFD_TYPE_DECAWAVE_16       = 2,
    DW3000_SFD_TYPE_IEEE_802154Z      = 3,
} dw3000_phy_sfd_type_t;

typedef struct {
    uint8_t coarse;
    uint8_t fine;
} dw3000_phy_tx_gain_t;

typedef struct {
    dw3000_phy_tx_gain_t shr;
    dw3000_phy_tx_gain_t phr;
    dw3000_phy_tx_gain_t data;
    dw3000_phy_tx_gain_t sts;
} dw3000_phy_tx_power_t;

typedef struct {
    uint16_t tx_antenna_delay;
    uint16_t rx_antenna_delay;
} dw3000_phy_antenna_delay_t;

typedef struct {
    dw3000_channel_t channel;
    dw3000_prf_t prf;
    dw3000_data_rate_t data_rate;
    dw3000_phy_preamble_length_t preamble_length;
    dw3000_phy_sfd_type_t sfd_type;
    uint8_t tx_preamble_code;
    uint8_t rx_preamble_code;
    uint16_t sfd_timeout;
    uint16_t preamble_timeout;
    bool extended_phr;
    bool phr_rate_6m8;
    dw3000_phy_tx_power_t tx_power;
    dw3000_phy_antenna_delay_t antenna_delay;
} dw3000_phy_config_t;

typedef struct {
    dw3000_phy_config_t phy;
    dw3000_address_t address;
} dw3000_phy_radio_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_PHY_H */
