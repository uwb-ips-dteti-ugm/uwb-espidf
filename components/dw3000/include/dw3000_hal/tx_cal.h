#ifndef DW3000_HAL_TX_CAL_H
#define DW3000_HAL_TX_CAL_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/tx_cal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_TX_CAL_READY_POLL_DELAY_US 100U

dw3000_error_t dw3000_hal_tx_cal_validate_config(
    const dw3000_tx_cal_config_t* config
);

dw3000_error_t dw3000_hal_tx_cal_read_sar_status(
    dw3000_device_t*                device,
    dw3000_tx_cal_sar_status_t*     status
);

dw3000_error_t dw3000_hal_tx_cal_write_sar_ctrl(
    dw3000_device_t*             device,
    dw3000_tx_cal_sar_ctrl_t     ctrl
);

dw3000_error_t dw3000_hal_tx_cal_start_sar(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_cal_read_sar(
    dw3000_device_t*              device,
    dw3000_tx_cal_sar_reading_t*  reading
);

dw3000_error_t dw3000_hal_tx_cal_read_sar_wake(
    dw3000_device_t*            device,
    dw3000_tx_cal_sar_wake_t*   reading
);

dw3000_error_t dw3000_hal_tx_cal_read_pgc_ctrl(
    dw3000_device_t*             device,
    dw3000_tx_cal_pgc_ctrl_t*    ctrl,
    uint8_t*                     tmeas
);

dw3000_error_t dw3000_hal_tx_cal_write_pgc_ctrl(
    dw3000_device_t*            device,
    dw3000_tx_cal_pgc_ctrl_t    ctrl,
    uint8_t                     tmeas
);

dw3000_error_t dw3000_hal_tx_cal_start_pgc_count(
    dw3000_device_t* device,
    uint8_t          tmeas
);

dw3000_error_t dw3000_hal_tx_cal_start_pgc_autocal(
    dw3000_device_t* device,
    uint8_t          tmeas
);

dw3000_error_t dw3000_hal_tx_cal_read_pgc_status(
    dw3000_device_t*             device,
    dw3000_tx_cal_pgc_status_t*  status
);

bool dw3000_hal_tx_cal_pgc_autocal_done(
    const dw3000_tx_cal_pgc_status_t* status
);

dw3000_error_t dw3000_hal_tx_cal_wait_pgc_autocal_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
);

dw3000_error_t dw3000_hal_tx_cal_read_pg_test(
    dw3000_device_t*          device,
    dw3000_tx_cal_pg_test_t*  value
);

dw3000_error_t dw3000_hal_tx_cal_write_pg_test(
    dw3000_device_t*         device,
    dw3000_tx_cal_pg_test_t  value
);

dw3000_error_t dw3000_hal_tx_cal_read_pg_target(
    dw3000_device_t*            device,
    dw3000_tx_cal_pg_target_t*  target
);

dw3000_error_t dw3000_hal_tx_cal_write_pg_target(
    dw3000_device_t*           device,
    dw3000_tx_cal_pg_target_t  target
);

dw3000_error_t dw3000_hal_tx_cal_configure(
    dw3000_device_t*               device,
    const dw3000_tx_cal_config_t*  config
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_TX_CAL_H */
