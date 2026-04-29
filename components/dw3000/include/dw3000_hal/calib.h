#ifndef DW3000_HAL_CALIB_H
#define DW3000_HAL_CALIB_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/calib.h"
#include "dw3000_types/cia.h"
#include "dw3000_types/pll.h"
#include "dw3000_types/tx_cal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_CALIB_RX_CAL_TIMEOUT_US   10000U
#define DW3000_HAL_CALIB_SAR_TIMEOUT_US      10000U
#define DW3000_HAL_CALIB_PGC_TIMEOUT_US      10000U
#define DW3000_HAL_CALIB_PLL_LOCK_TIMEOUT_US 10000U

dw3000_error_t dw3000_hal_calib_validate_config(
    const dw3000_calib_config_t* config
);

dw3000_error_t dw3000_hal_calib_read_tx_power(
    dw3000_device_t*            device,
    dw3000_calib_tx_power_t*    tx_power
);

dw3000_error_t dw3000_hal_calib_set_tx_power(
    dw3000_device_t*                  device,
    const dw3000_calib_tx_power_t*    tx_power
);

dw3000_error_t dw3000_hal_calib_read_xtal_trim(
    dw3000_device_t*              device,
    dw3000_calib_xtal_trim_t*     trim
);

dw3000_error_t dw3000_hal_calib_set_xtal_trim(
    dw3000_device_t*             device,
    dw3000_calib_xtal_trim_t     trim
);

dw3000_error_t dw3000_hal_calib_read_antenna_delays(
    dw3000_device_t*             device,
    dw3000_cia_antenna_delay_t*  tx_delay,
    dw3000_cia_antenna_delay_t*  rx_delay
);

dw3000_error_t dw3000_hal_calib_set_antenna_delays(
    dw3000_device_t*            device,
    dw3000_cia_antenna_delay_t  tx_delay,
    dw3000_cia_antenna_delay_t  rx_delay
);

dw3000_error_t dw3000_hal_calib_read_otp_sar_reference(
    dw3000_device_t*                 device,
    dw3000_calib_sar_reference_t*    reference
);

int32_t dw3000_hal_calib_sar_vbat_millivolts(
    const dw3000_calib_sar_reference_t* reference,
    uint8_t                             raw_vbat
);

int32_t dw3000_hal_calib_sar_temp_centi_celsius(
    const dw3000_calib_sar_reference_t* reference,
    uint8_t                             raw_temp
);

dw3000_error_t dw3000_hal_calib_convert_sar_reading(
    const dw3000_calib_sar_reference_t* reference,
    const dw3000_tx_cal_sar_reading_t*  reading,
    dw3000_calib_sar_converted_t*       converted
);

dw3000_error_t dw3000_hal_calib_read_sar(
    dw3000_device_t*                  device,
    dw3000_tx_cal_sar_reading_t*      reading
);

dw3000_error_t dw3000_hal_calib_measure_sar(
    dw3000_device_t*                  device,
    uint32_t                          timeout_us,
    dw3000_tx_cal_sar_reading_t*      reading
);

dw3000_error_t dw3000_hal_calib_read_sar_wake(
    dw3000_device_t*              device,
    dw3000_tx_cal_sar_wake_t*     reading
);

dw3000_error_t dw3000_hal_calib_read_rx_cal_status(
    dw3000_device_t*                  device,
    dw3000_calib_rx_cal_status_t*     status
);

dw3000_error_t dw3000_hal_calib_read_rx_cal_result(
    dw3000_device_t*                  device,
    dw3000_calib_rx_cal_result_t*     result
);

bool dw3000_hal_calib_rx_cal_result_is_valid(
    const dw3000_calib_rx_cal_result_t* result
);

dw3000_error_t dw3000_hal_calib_run_rx_calibration(
    dw3000_device_t*                  device,
    uint32_t                          timeout_us,
    dw3000_calib_rx_cal_result_t*     result
);

dw3000_error_t dw3000_hal_calib_read_pgc_status(
    dw3000_device_t*                 device,
    dw3000_tx_cal_pgc_status_t*      status
);

dw3000_error_t dw3000_hal_calib_set_pgc_target(
    dw3000_device_t*                 device,
    dw3000_tx_cal_pg_target_t        target
);

dw3000_error_t dw3000_hal_calib_start_pgc_count(
    dw3000_device_t* device,
    uint8_t          tmeas
);

dw3000_error_t dw3000_hal_calib_start_pgc_autocal(
    dw3000_device_t* device,
    uint8_t          tmeas
);

dw3000_error_t dw3000_hal_calib_wait_pgc_autocal_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
);

dw3000_error_t dw3000_hal_calib_read_pll_coarse_code(
    dw3000_device_t*             device,
    dw3000_pll_coarse_code_t*    code
);

dw3000_error_t dw3000_hal_calib_set_pll_coarse_code(
    dw3000_device_t*            device,
    dw3000_pll_coarse_code_t    code
);

dw3000_error_t dw3000_hal_calib_read_otp_pll_lock_code(
    dw3000_device_t*             device,
    dw3000_pll_coarse_code_t*    code
);

dw3000_error_t dw3000_hal_calib_start_pll_calibration(
    dw3000_device_t*           device,
    dw3000_pll_cal_flags_t     flags
);

dw3000_error_t dw3000_hal_calib_recalibrate_pll(
    dw3000_device_t*       device,
    dw3000_phy_channel_t   channel,
    uint32_t               timeout_us
);

dw3000_error_t dw3000_hal_calib_apply_otp_factory(
    dw3000_device_t* device
);

dw3000_error_t dw3000_hal_calib_configure(
    dw3000_device_t*                device,
    const dw3000_calib_config_t*    config
);

dw3000_error_t dw3000_hal_calib_configure_current(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_CALIB_H */
