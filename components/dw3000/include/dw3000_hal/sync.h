#ifndef DW3000_HAL_SYNC_H
#define DW3000_HAL_SYNC_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/calib.h"
#include "dw3000_types/sync.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_SYNC_DEFAULT_OSTS_WAIT DW3000_SYNC_OSTS_WAIT_RECOMMENDED

bool dw3000_hal_sync_osts_wait_is_valid(
    dw3000_sync_osts_wait_t wait
);

dw3000_error_t dw3000_hal_sync_validate_config(
    const dw3000_sync_config_t* config
);

dw3000_error_t dw3000_hal_sync_read_ec_ctrl(
    dw3000_device_t* device,
    uint32_t*        value
);

dw3000_error_t dw3000_hal_sync_write_ec_ctrl(
    dw3000_device_t* device,
    uint32_t         value
);

dw3000_error_t dw3000_hal_sync_read_config(
    dw3000_device_t*        device,
    dw3000_sync_config_t*   config
);

dw3000_error_t dw3000_hal_sync_configure(
    dw3000_device_t*              device,
    const dw3000_sync_config_t*   config
);

dw3000_error_t dw3000_hal_sync_configure_current(
    dw3000_device_t* device
);

dw3000_error_t dw3000_hal_sync_set_flags(
    dw3000_device_t*                device,
    dw3000_sync_ec_ctrl_flags_t     flags
);

dw3000_error_t dw3000_hal_sync_clear_flags(
    dw3000_device_t*                device,
    dw3000_sync_ec_ctrl_flags_t     flags
);

dw3000_error_t dw3000_hal_sync_set_osts_wait(
    dw3000_device_t*           device,
    dw3000_sync_osts_wait_t    wait
);

dw3000_error_t dw3000_hal_sync_arm_ostr(
    dw3000_device_t*           device,
    dw3000_sync_osts_wait_t    wait
);

dw3000_error_t dw3000_hal_sync_disarm_ostr(
    dw3000_device_t* device
);

dw3000_error_t dw3000_hal_sync_set_pll_sync_clock(
    dw3000_device_t* device,
    bool             enable
);

dw3000_error_t dw3000_hal_sync_prepare_ostr(
    dw3000_device_t*           device,
    dw3000_sync_osts_wait_t    wait
);

dw3000_error_t dw3000_hal_sync_clear_rx_cal_status(
    dw3000_device_t* device
);

dw3000_error_t dw3000_hal_sync_read_rx_cal_status(
    dw3000_device_t*               device,
    dw3000_calib_rx_cal_status_t*  status
);

dw3000_error_t dw3000_hal_sync_read_rx_cal_result(
    dw3000_device_t*              device,
    dw3000_calib_rx_cal_result_t* result
);

bool dw3000_hal_sync_rx_cal_result_is_valid(
    const dw3000_calib_rx_cal_result_t* result
);

dw3000_error_t dw3000_hal_sync_run_rx_calibration(
    dw3000_device_t*              device,
    uint32_t                      timeout_us,
    dw3000_calib_rx_cal_result_t* result
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_SYNC_H */
