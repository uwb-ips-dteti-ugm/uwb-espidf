#ifndef DW3000_HAL_PLL_H
#define DW3000_HAL_PLL_H

#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/calib.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/pll.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_pll_cfg_t dw3000_hal_pll_cfg_for_channel(
    dw3000_phy_channel_t channel
);

dw3000_error_t dw3000_hal_pll_validate_config(
    const dw3000_pll_config_t* config
);

dw3000_error_t dw3000_hal_pll_read_cfg(
    dw3000_device_t*   device,
    dw3000_pll_cfg_t*  cfg
);

dw3000_error_t dw3000_hal_pll_write_cfg(
    dw3000_device_t*  device,
    dw3000_pll_cfg_t  cfg
);

dw3000_error_t dw3000_hal_pll_configure_channel(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
);

dw3000_error_t dw3000_hal_pll_read_coarse_code(
    dw3000_device_t*             device,
    dw3000_pll_coarse_code_t*    code
);

dw3000_error_t dw3000_hal_pll_write_coarse_code(
    dw3000_device_t*            device,
    dw3000_pll_coarse_code_t    code
);

dw3000_error_t dw3000_hal_pll_read_calibration(
    dw3000_device_t*         device,
    dw3000_pll_cal_flags_t*  flags
);

dw3000_error_t dw3000_hal_pll_write_calibration(
    dw3000_device_t*        device,
    dw3000_pll_cal_flags_t  flags
);

dw3000_error_t dw3000_hal_pll_start_calibration(
    dw3000_device_t*        device,
    dw3000_pll_cal_flags_t  flags
);

dw3000_error_t dw3000_hal_pll_read_xtal_trim(
    dw3000_device_t*             device,
    dw3000_calib_xtal_trim_t*    trim
);

dw3000_error_t dw3000_hal_pll_write_xtal_trim(
    dw3000_device_t*            device,
    dw3000_calib_xtal_trim_t    trim
);

dw3000_error_t dw3000_hal_pll_read_config(
    dw3000_device_t*       device,
    dw3000_pll_config_t*   config
);

dw3000_error_t dw3000_hal_pll_configure(
    dw3000_device_t*             device,
    const dw3000_pll_config_t*   config
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_PLL_H */
