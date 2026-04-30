#include "dw3000_hal/sync.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_hal/calib.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_register.h"

#define DW3000_HAL_SYNC_OSTS_WAIT_SHIFT 3U
#define DW3000_HAL_SYNC_OSTS_WAIT_MASK \
    ((uint32_t)DW3000_SYNC_OSTS_WAIT_MAX << DW3000_HAL_SYNC_OSTS_WAIT_SHIFT)
#define DW3000_HAL_SYNC_EC_CTRL_MASK \
    (DW3000_HAL_SYNC_OSTS_WAIT_MASK | DW3000_SYNC_EC_CTRL_FLAGS_MASK)

static bool dw3000_hal_sync_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static uint32_t dw3000_hal_sync_encode_config(
    const dw3000_sync_config_t* config
) {
    return ((uint32_t)config->flags & DW3000_SYNC_EC_CTRL_FLAGS_MASK) |
           (((uint32_t)config->wait << DW3000_HAL_SYNC_OSTS_WAIT_SHIFT) &
            DW3000_HAL_SYNC_OSTS_WAIT_MASK);
}

static void dw3000_hal_sync_decode_config(
    uint32_t              raw,
    dw3000_sync_config_t* config
) {
    config->flags = (dw3000_sync_ec_ctrl_flags_t)(raw & DW3000_SYNC_EC_CTRL_FLAGS_MASK);
    config->wait  = (dw3000_sync_osts_wait_t)((raw & DW3000_HAL_SYNC_OSTS_WAIT_MASK) >>
                                             DW3000_HAL_SYNC_OSTS_WAIT_SHIFT);
}

bool dw3000_hal_sync_osts_wait_is_valid(
    dw3000_sync_osts_wait_t wait
) {
    return ((uint32_t)wait % DW3000_SYNC_OSTS_WAIT_MODULUS) ==
           DW3000_SYNC_OSTS_WAIT_PHASE;
}

dw3000_error_t dw3000_hal_sync_validate_config(
    const dw3000_sync_config_t* config
) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)config->flags & ~DW3000_SYNC_EC_CTRL_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)config->flags & (uint32_t)DW3000_SYNC_OSTR_MODE) != 0U) {
        if (!dw3000_hal_sync_osts_wait_is_valid(config->wait)) {
            return DW3000_ERROR_INVALID_ARG;
        }
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sync_read_ec_ctrl(
    dw3000_device_t* device,
    uint32_t*        value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_EC_CTRL, value);
}

dw3000_error_t dw3000_hal_sync_write_ec_ctrl(
    dw3000_device_t* device,
    uint32_t         value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((value & ~DW3000_HAL_SYNC_EC_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((value & (uint32_t)DW3000_SYNC_OSTR_MODE) != 0U) &&
        !dw3000_hal_sync_osts_wait_is_valid(
            (dw3000_sync_osts_wait_t)((value & DW3000_HAL_SYNC_OSTS_WAIT_MASK) >>
                                      DW3000_HAL_SYNC_OSTS_WAIT_SHIFT)
        )) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sync_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_EC_CTRL, value);
}

dw3000_error_t dw3000_hal_sync_read_config(
    dw3000_device_t*      device,
    dw3000_sync_config_t* config
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (config == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_sync_read_ec_ctrl(device, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_sync_decode_config(raw, config);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sync_configure(
    dw3000_device_t*            device,
    const dw3000_sync_config_t* config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_sync_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sync_write_ec_ctrl(
        device,
        dw3000_hal_sync_encode_config(config)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.sync = *config;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sync_configure_current(
    dw3000_device_t* device
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_sync_configure(device, &device->config.sync);
}

dw3000_error_t dw3000_hal_sync_set_flags(
    dw3000_device_t*            device,
    dw3000_sync_ec_ctrl_flags_t flags
) {
    dw3000_error_t       err;
    dw3000_sync_config_t config;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)flags & ~DW3000_SYNC_EC_CTRL_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_sync_read_config(device, &config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    config.flags = (dw3000_sync_ec_ctrl_flags_t)(config.flags | flags);
    return dw3000_hal_sync_configure(device, &config);
}

dw3000_error_t dw3000_hal_sync_clear_flags(
    dw3000_device_t*            device,
    dw3000_sync_ec_ctrl_flags_t flags
) {
    dw3000_error_t       err;
    dw3000_sync_config_t config;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)flags & ~DW3000_SYNC_EC_CTRL_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_sync_read_config(device, &config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    config.flags = (dw3000_sync_ec_ctrl_flags_t)(config.flags & ~flags);
    return dw3000_hal_sync_configure(device, &config);
}

dw3000_error_t dw3000_hal_sync_set_osts_wait(
    dw3000_device_t*        device,
    dw3000_sync_osts_wait_t wait
) {
    dw3000_error_t       err;
    dw3000_sync_config_t config;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_sync_read_config(device, &config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    config.wait = wait;
    return dw3000_hal_sync_configure(device, &config);
}

dw3000_error_t dw3000_hal_sync_arm_ostr(
    dw3000_device_t*        device,
    dw3000_sync_osts_wait_t wait
) {
    const dw3000_sync_config_t config = {
        .flags = DW3000_SYNC_OSTR_MODE,
        .wait  = wait,
    };

    return dw3000_hal_sync_configure(device, &config);
}

dw3000_error_t dw3000_hal_sync_disarm_ostr(
    dw3000_device_t* device
) {
    return dw3000_hal_sync_clear_flags(device, DW3000_SYNC_OSTR_MODE);
}

dw3000_error_t dw3000_hal_sync_set_pll_sync_clock(
    dw3000_device_t* device,
    bool             enable
) {
    if (enable) {
        return dw3000_hal_pmsc_set_seq_flags(device, DW3000_PMSC_SEQ_PLL_SYNC);
    }

    return dw3000_hal_pmsc_clear_seq_flags(device, DW3000_PMSC_SEQ_PLL_SYNC);
}

dw3000_error_t dw3000_hal_sync_prepare_ostr(
    dw3000_device_t*        device,
    dw3000_sync_osts_wait_t wait
) {
    dw3000_error_t               err;
    dw3000_pmsc_seq_ctrl_flags_t seq_flags;
    bool                         had_pll_sync;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_pmsc_read_seq_ctrl(device, &seq_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    had_pll_sync = (seq_flags & DW3000_PMSC_SEQ_PLL_SYNC) != 0U;

    err = dw3000_hal_sync_set_pll_sync_clock(device, true);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sync_arm_ostr(device, wait);
    if ((err != DW3000_ERROR_OK) && !had_pll_sync) {
        (void)dw3000_hal_sync_set_pll_sync_clock(device, false);
    }

    return err;
}

dw3000_error_t dw3000_hal_sync_clear_rx_cal_status(
    dw3000_device_t* device
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_write_u8(
        device,
        DW3000_REG_RX_CAL_STS,
        (uint8_t)DW3000_CALIB_RX_CAL_DONE
    );
}

dw3000_error_t dw3000_hal_sync_read_rx_cal_status(
    dw3000_device_t*              device,
    dw3000_calib_rx_cal_status_t* status
) {
    return dw3000_hal_calib_read_rx_cal_status(device, status);
}

dw3000_error_t dw3000_hal_sync_read_rx_cal_result(
    dw3000_device_t*              device,
    dw3000_calib_rx_cal_result_t* result
) {
    return dw3000_hal_calib_read_rx_cal_result(device, result);
}

bool dw3000_hal_sync_rx_cal_result_is_valid(
    const dw3000_calib_rx_cal_result_t* result
) {
    return dw3000_hal_calib_rx_cal_result_is_valid(result);
}

dw3000_error_t dw3000_hal_sync_run_rx_calibration(
    dw3000_device_t*              device,
    uint32_t                      timeout_us,
    dw3000_calib_rx_cal_result_t* result
) {
    return dw3000_hal_calib_run_rx_calibration(device, timeout_us, result);
}
