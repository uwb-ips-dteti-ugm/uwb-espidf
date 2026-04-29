#ifndef DW3000_HAL_PHY_H
#define DW3000_HAL_PHY_H

#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/rx_tune.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_error_t dw3000_hal_phy_validate_config(const dw3000_phy_config_t* config);

dw3000_error_t dw3000_hal_phy_validate_rx_tune(
    const dw3000_rx_tune_config_t* config
);

uint16_t dw3000_hal_phy_preamble_symbols(
    dw3000_phy_preamble_length_t preamble_length
);

uint8_t dw3000_hal_phy_pac_symbols(dw3000_phy_pac_size_t pac_size);

uint8_t dw3000_hal_phy_sfd_symbols(dw3000_phy_sfd_type_t sfd_type);

uint16_t dw3000_hal_phy_sfd_timeout(const dw3000_phy_config_t* config);

dw3000_error_t dw3000_hal_phy_configure(
    dw3000_device_t*              device,
    const dw3000_phy_config_t*    phy_config,
    const dw3000_rx_tune_config_t* rx_tune_config
);

dw3000_error_t dw3000_hal_phy_configure_current(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_PHY_H */
