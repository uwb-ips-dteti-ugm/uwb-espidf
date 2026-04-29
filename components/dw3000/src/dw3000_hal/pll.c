#include "dw3000_hal/pll.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

static bool dw3000_hal_pll_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static bool dw3000_hal_pll_channel_is_valid(dw3000_phy_channel_t channel) {
    return (channel == DW3000_PHY_CHANNEL_5) || (channel == DW3000_PHY_CHANNEL_9);
}

static bool dw3000_hal_pll_cfg_is_valid(dw3000_pll_cfg_t cfg) {
    return (cfg == DW3000_PLL_CFG_CH5) || (cfg == DW3000_PLL_CFG_CH9);
}

dw3000_pll_cfg_t dw3000_hal_pll_cfg_for_channel(
    dw3000_phy_channel_t channel
) {
    switch (channel) {
        case DW3000_PHY_CHANNEL_5:
            return DW3000_PLL_CFG_CH5;

        case DW3000_PHY_CHANNEL_9:
            return DW3000_PLL_CFG_CH9;

        default:
            return 0U;
    }
}

dw3000_error_t dw3000_hal_pll_validate_config(
    const dw3000_pll_config_t* config
) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_cfg_is_valid(config->cfg)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->coarse_code & ~DW3000_PLL_COARSE_CODE_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)config->cal_flags & ~DW3000_PLL_CAL_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pll_read_cfg(
    dw3000_device_t*   device,
    dw3000_pll_cfg_t*  cfg
) {
    if ((device == NULL) || (cfg == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u16(device, DW3000_REG_PLL_CFG, cfg);
}

dw3000_error_t dw3000_hal_pll_write_cfg(
    dw3000_device_t*  device,
    dw3000_pll_cfg_t  cfg
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_cfg_is_valid(cfg)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u16(device, DW3000_REG_PLL_CFG, cfg);
}

dw3000_error_t dw3000_hal_pll_configure_channel(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_channel_is_valid(channel)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_pll_write_cfg(
        device,
        dw3000_hal_pll_cfg_for_channel(channel)
    );
}

dw3000_error_t dw3000_hal_pll_read_coarse_code(
    dw3000_device_t*             device,
    dw3000_pll_coarse_code_t*    code
) {
    if ((device == NULL) || (code == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_PLL_CC, code);
}

dw3000_error_t dw3000_hal_pll_write_coarse_code(
    dw3000_device_t*            device,
    dw3000_pll_coarse_code_t    code
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((code & ~DW3000_PLL_COARSE_CODE_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_PLL_CC, code);
}

dw3000_error_t dw3000_hal_pll_read_calibration(
    dw3000_device_t*         device,
    dw3000_pll_cal_flags_t*  flags
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (flags == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PLL_CAL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *flags = (dw3000_pll_cal_flags_t)(raw & DW3000_PLL_CAL_FLAGS_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pll_write_calibration(
    dw3000_device_t*        device,
    dw3000_pll_cal_flags_t  flags
) {
    dw3000_error_t err;
    uint16_t       raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)flags & ~DW3000_PLL_CAL_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PLL_CAL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    raw = (uint16_t)((raw & ~DW3000_PLL_CAL_FLAGS_MASK) |
                     ((uint16_t)flags & DW3000_PLL_CAL_FLAGS_MASK));

    return dw3000_reg_write_u16(device, DW3000_REG_PLL_CAL, raw);
}

dw3000_error_t dw3000_hal_pll_start_calibration(
    dw3000_device_t*        device,
    dw3000_pll_cal_flags_t  flags
) {
    if (((uint16_t)flags & ~DW3000_PLL_CAL_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_pll_write_calibration(
        device,
        (dw3000_pll_cal_flags_t)((uint16_t)flags | (uint16_t)DW3000_PLL_CAL_EN)
    );
}

dw3000_error_t dw3000_hal_pll_read_xtal_trim(
    dw3000_device_t*             device,
    dw3000_calib_xtal_trim_t*    trim
) {
    dw3000_error_t err;
    uint8_t        raw;

    if ((device == NULL) || (trim == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_XTAL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *trim = (dw3000_calib_xtal_trim_t)(raw & DW3000_CALIB_XTAL_TRIM_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pll_write_xtal_trim(
    dw3000_device_t*            device,
    dw3000_calib_xtal_trim_t    trim
) {
    dw3000_error_t err;
    uint8_t        raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((trim & ~DW3000_CALIB_XTAL_TRIM_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pll_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_XTAL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    raw = (uint8_t)((raw & ~DW3000_CALIB_XTAL_TRIM_MASK) |
                    (trim & DW3000_CALIB_XTAL_TRIM_MASK));

    return dw3000_reg_write_u8(device, DW3000_REG_XTAL, raw);
}

dw3000_error_t dw3000_hal_pll_read_config(
    dw3000_device_t*       device,
    dw3000_pll_config_t*   config
) {
    dw3000_error_t err;

    if ((device == NULL) || (config == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_pll_read_cfg(device, &config->cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pll_read_coarse_code(device, &config->coarse_code);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_pll_read_calibration(device, &config->cal_flags);
}

dw3000_error_t dw3000_hal_pll_configure(
    dw3000_device_t*             device,
    const dw3000_pll_config_t*   config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_pll_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pll_write_cfg(device, config->cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pll_write_coarse_code(device, config->coarse_code);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_pll_write_calibration(device, config->cal_flags);
}
