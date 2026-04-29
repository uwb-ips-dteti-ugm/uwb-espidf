#ifndef DW3000_HAL_AON_H
#define DW3000_HAL_AON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/aon.h"
#include "dw3000_types/calib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_AON_CFG_UPLOAD_SETTLE_US       1U
#define DW3000_HAL_AON_SLEEP_TIMER_LOAD_DELAY_US  32U
#define DW3000_HAL_AON_WAKE_TIMEOUT_US            10000U
#define DW3000_HAL_AON_PLL_LOCK_TIMEOUT_US        10000U
#define DW3000_HAL_AON_RX_CAL_TIMEOUT_US          10000U

typedef struct {
    uint32_t spi_ready_timeout_us;
    uint32_t pll_lock_timeout_us;
    uint32_t rx_cal_timeout_us;
    bool     reload_otp_ldo_bias;
    bool     run_rx_calibration;
    bool     enter_idle_pll;
} dw3000_hal_aon_wake_options_t;

void dw3000_hal_aon_default_wake_options(
    dw3000_hal_aon_wake_options_t* options
);

bool dw3000_hal_aon_addr_is_valid(dw3000_aon_addr_t addr);

dw3000_error_t dw3000_hal_aon_validate_dig_cfg(dw3000_aon_dig_cfg_t dig_cfg);

dw3000_error_t dw3000_hal_aon_validate_cfg(dw3000_aon_cfg_flags_t cfg);

dw3000_error_t dw3000_hal_aon_validate_config(
    const dw3000_aon_config_t* config
);

dw3000_error_t dw3000_hal_aon_sleep_time_from_us(
    uint32_t                    sleep_us,
    uint32_t                    lp_osc_hz,
    dw3000_aon_sleep_time_t*    sleep_time
);

dw3000_error_t dw3000_hal_aon_read_dig_cfg(
    dw3000_device_t*        device,
    dw3000_aon_dig_cfg_t*   dig_cfg
);

dw3000_error_t dw3000_hal_aon_set_dig_cfg(
    dw3000_device_t*       device,
    dw3000_aon_dig_cfg_t   dig_cfg
);

dw3000_error_t dw3000_hal_aon_read_cfg(
    dw3000_device_t*           device,
    dw3000_aon_cfg_flags_t*    cfg
);

dw3000_error_t dw3000_hal_aon_set_cfg(
    dw3000_device_t*          device,
    dw3000_aon_cfg_flags_t    cfg
);

dw3000_error_t dw3000_hal_aon_read_memory_byte(
    dw3000_device_t*       device,
    dw3000_aon_addr_t      addr,
    dw3000_aon_byte_t*     value
);

dw3000_error_t dw3000_hal_aon_write_memory_byte(
    dw3000_device_t*       device,
    dw3000_aon_addr_t      addr,
    dw3000_aon_byte_t      value
);

dw3000_error_t dw3000_hal_aon_read_memory(
    dw3000_device_t*       device,
    dw3000_aon_addr_t      start_addr,
    dw3000_aon_byte_t*     data,
    size_t                 data_len
);

dw3000_error_t dw3000_hal_aon_write_memory(
    dw3000_device_t*           device,
    dw3000_aon_addr_t          start_addr,
    const dw3000_aon_byte_t*   data,
    size_t                     data_len
);

dw3000_error_t dw3000_hal_aon_read_sleep_time(
    dw3000_device_t*           device,
    dw3000_aon_sleep_time_t*   sleep_time
);

dw3000_error_t dw3000_hal_aon_set_sleep_time(
    dw3000_device_t*          device,
    dw3000_aon_sleep_time_t   sleep_time
);

dw3000_error_t dw3000_hal_aon_upload_config(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aon_save(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aon_restore(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aon_configure(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config
);

dw3000_error_t dw3000_hal_aon_configure_current(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aon_enter_sleep(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config
);

dw3000_error_t dw3000_hal_aon_enter_deepsleep(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config
);

dw3000_error_t dw3000_hal_aon_finish_wake(
    dw3000_device_t*                          device,
    const dw3000_hal_aon_wake_options_t*      options,
    dw3000_calib_rx_cal_result_t*             rx_cal_result
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_AON_H */
