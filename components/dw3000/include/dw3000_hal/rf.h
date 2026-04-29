#ifndef DW3000_HAL_RF_H
#define DW3000_HAL_RF_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/rf.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t dw3000_hal_rf_force_tx_value(
    dw3000_phy_channel_t channel
);

dw3000_rf_tx_ctrl_1_t dw3000_hal_rf_tx_ctrl_1_value(
    dw3000_phy_channel_t channel
);

dw3000_rf_tx_ctrl_2_t dw3000_hal_rf_tx_ctrl_2_value(
    dw3000_phy_channel_t channel
);

dw3000_error_t dw3000_hal_rf_read_enable(
    dw3000_device_t* device,
    uint32_t*        value
);

dw3000_error_t dw3000_hal_rf_write_enable(
    dw3000_device_t* device,
    uint32_t         value
);

dw3000_error_t dw3000_hal_rf_read_ctrl_mask(
    dw3000_device_t* device,
    uint32_t*        value
);

dw3000_error_t dw3000_hal_rf_write_ctrl_mask(
    dw3000_device_t* device,
    uint32_t         value
);

dw3000_error_t dw3000_hal_rf_force_tx_path(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
);

dw3000_error_t dw3000_hal_rf_clear_forced_path(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rf_read_switch(
    dw3000_device_t*      device,
    dw3000_rf_switch_t*   value
);

dw3000_error_t dw3000_hal_rf_write_switch(
    dw3000_device_t*     device,
    dw3000_rf_switch_t   value
);

dw3000_error_t dw3000_hal_rf_read_tx_ctrl_1(
    dw3000_device_t*          device,
    dw3000_rf_tx_ctrl_1_t*    value
);

dw3000_error_t dw3000_hal_rf_write_tx_ctrl_1(
    dw3000_device_t*         device,
    dw3000_rf_tx_ctrl_1_t    value
);

dw3000_error_t dw3000_hal_rf_read_tx_ctrl_2(
    dw3000_device_t*          device,
    dw3000_rf_tx_ctrl_2_t*    value
);

dw3000_error_t dw3000_hal_rf_write_tx_ctrl_2(
    dw3000_device_t*         device,
    dw3000_rf_tx_ctrl_2_t    value
);

dw3000_error_t dw3000_hal_rf_configure_tx_channel(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
);

dw3000_error_t dw3000_hal_rf_read_tx_test(
    dw3000_device_t*       device,
    dw3000_rf_tx_test_t*   value
);

dw3000_error_t dw3000_hal_rf_write_tx_test(
    dw3000_device_t*      device,
    dw3000_rf_tx_test_t   value
);

dw3000_error_t dw3000_hal_rf_read_sar_test(
    dw3000_device_t*        device,
    dw3000_rf_sar_test_t*   value
);

dw3000_error_t dw3000_hal_rf_write_sar_test(
    dw3000_device_t*       device,
    dw3000_rf_sar_test_t   value
);

dw3000_error_t dw3000_hal_rf_set_sar_read_enable(
    dw3000_device_t* device,
    bool             enable
);

dw3000_error_t dw3000_hal_rf_read_ldo_tune(
    dw3000_device_t*       device,
    dw3000_rf_ldo_tune_t*  value
);

dw3000_error_t dw3000_hal_rf_write_ldo_tune(
    dw3000_device_t*             device,
    const dw3000_rf_ldo_tune_t*  value
);

dw3000_error_t dw3000_hal_rf_read_ldo_ctrl(
    dw3000_device_t*       device,
    dw3000_rf_ldo_ctrl_t*  value
);

dw3000_error_t dw3000_hal_rf_write_ldo_ctrl(
    dw3000_device_t*      device,
    dw3000_rf_ldo_ctrl_t  value
);

dw3000_error_t dw3000_hal_rf_read_ldo_rload(
    dw3000_device_t*        device,
    dw3000_rf_ldo_rload_t*  value
);

dw3000_error_t dw3000_hal_rf_write_ldo_rload(
    dw3000_device_t*       device,
    dw3000_rf_ldo_rload_t  value
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_RF_H */
