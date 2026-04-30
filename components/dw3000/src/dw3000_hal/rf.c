#include "dw3000_hal/rf.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_RF_TX_TEST_MASK  0x0FU
#define DW3000_HAL_RF_SAR_TEST_MASK ((uint8_t)DW3000_RF_SAR_TEST_RDEN)
#define DW3000_HAL_RF_LDO_CTRL_MASK \
    ((uint32_t)(DW3000_RF_LDO_VDDMS1 | \
                DW3000_RF_LDO_VDDMS2 | \
                DW3000_RF_LDO_VDDMS3 | \
                DW3000_RF_LDO_VDDPLL | \
                DW3000_RF_LDO_VDDTX1 | \
                DW3000_RF_LDO_VDDTX2 | \
                DW3000_RF_LDO_VDDIF2 | \
                DW3000_RF_LDO_VDDHVTX | \
                DW3000_RF_LDO_VDDTX1_VREF | \
                DW3000_RF_LDO_VDDTX2_VREF | \
                DW3000_RF_LDO_VDDHVTX_VREF))

static bool dw3000_hal_rf_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static bool dw3000_hal_rf_channel_is_valid(dw3000_phy_channel_t channel) {
    return (channel == DW3000_PHY_CHANNEL_5) || (channel == DW3000_PHY_CHANNEL_9);
}

uint32_t dw3000_hal_rf_force_tx_value(
    dw3000_phy_channel_t channel
) {
    switch (channel) {
        case DW3000_PHY_CHANNEL_5:
            return DW3000_RF_FORCE_TX_CH5;

        case DW3000_PHY_CHANNEL_9:
            return DW3000_RF_FORCE_TX_CH9;

        default:
            return 0U;
    }
}

dw3000_rf_tx_ctrl_1_t dw3000_hal_rf_tx_ctrl_1_value(
    dw3000_phy_channel_t channel
) {
    if (!dw3000_hal_rf_channel_is_valid(channel)) {
        return 0U;
    }

    return DW3000_RF_TX_CTRL_1_OPT;
}

dw3000_rf_tx_ctrl_2_t dw3000_hal_rf_tx_ctrl_2_value(
    dw3000_phy_channel_t channel
) {
    switch (channel) {
        case DW3000_PHY_CHANNEL_5:
            return DW3000_RF_TX_CTRL_2_CH5;

        case DW3000_PHY_CHANNEL_9:
            return DW3000_RF_TX_CTRL_2_CH9;

        default:
            return 0U;
    }
}

dw3000_error_t dw3000_hal_rf_read_enable(
    dw3000_device_t* device,
    uint32_t*        value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_RF_ENABLE, value);
}

dw3000_error_t dw3000_hal_rf_write_enable(
    dw3000_device_t* device,
    uint32_t         value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_RF_ENABLE, value);
}

dw3000_error_t dw3000_hal_rf_read_ctrl_mask(
    dw3000_device_t* device,
    uint32_t*        value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_RF_CTRL_MASK, value);
}

dw3000_error_t dw3000_hal_rf_write_ctrl_mask(
    dw3000_device_t* device,
    uint32_t         value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_RF_CTRL_MASK, value);
}

dw3000_error_t dw3000_hal_rf_force_tx_path(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
) {
    dw3000_error_t err;
    uint32_t       value;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_channel_is_valid(channel)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    value = dw3000_hal_rf_force_tx_value(channel);
    err = dw3000_hal_rf_write_enable(device, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_rf_write_ctrl_mask(device, value);
}

dw3000_error_t dw3000_hal_rf_clear_forced_path(dw3000_device_t* device) {
    dw3000_error_t err;

    err = dw3000_hal_rf_write_enable(device, 0U);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_rf_write_ctrl_mask(device, 0U);
}

dw3000_error_t dw3000_hal_rf_read_switch(
    dw3000_device_t*     device,
    dw3000_rf_switch_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_RF_SWITCH, value);
}

dw3000_error_t dw3000_hal_rf_write_switch(
    dw3000_device_t*    device,
    dw3000_rf_switch_t  value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_RF_SWITCH, value);
}

dw3000_error_t dw3000_hal_rf_read_tx_ctrl_1(
    dw3000_device_t*        device,
    dw3000_rf_tx_ctrl_1_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_RF_TX_CTRL_1, value);
}

dw3000_error_t dw3000_hal_rf_write_tx_ctrl_1(
    dw3000_device_t*        device,
    dw3000_rf_tx_ctrl_1_t   value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_RF_TX_CTRL_1, value);
}

dw3000_error_t dw3000_hal_rf_read_tx_ctrl_2(
    dw3000_device_t*        device,
    dw3000_rf_tx_ctrl_2_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_RF_TX_CTRL_2, value);
}

dw3000_error_t dw3000_hal_rf_write_tx_ctrl_2(
    dw3000_device_t*        device,
    dw3000_rf_tx_ctrl_2_t   value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_RF_TX_CTRL_2, value);
}

dw3000_error_t dw3000_hal_rf_configure_tx_channel(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_channel_is_valid(channel)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_rf_write_tx_ctrl_1(
        device,
        dw3000_hal_rf_tx_ctrl_1_value(channel)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_rf_write_tx_ctrl_2(
        device,
        dw3000_hal_rf_tx_ctrl_2_value(channel)
    );
}

dw3000_error_t dw3000_hal_rf_read_tx_test(
    dw3000_device_t*      device,
    dw3000_rf_tx_test_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_TX_TEST, value);
}

dw3000_error_t dw3000_hal_rf_write_tx_test(
    dw3000_device_t*     device,
    dw3000_rf_tx_test_t  value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((value & ~DW3000_HAL_RF_TX_TEST_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_TX_TEST, value);
}

dw3000_error_t dw3000_hal_rf_read_sar_test(
    dw3000_device_t*       device,
    dw3000_rf_sar_test_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_SAR_TEST, value);
}

dw3000_error_t dw3000_hal_rf_write_sar_test(
    dw3000_device_t*      device,
    dw3000_rf_sar_test_t  value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((value & ~DW3000_HAL_RF_SAR_TEST_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_SAR_TEST, value);
}

dw3000_error_t dw3000_hal_rf_set_sar_read_enable(
    dw3000_device_t* device,
    bool             enable
) {
    uint8_t value = enable ? (uint8_t)DW3000_RF_SAR_TEST_RDEN : 0U;

    return dw3000_hal_rf_write_sar_test(device, value);
}

dw3000_error_t dw3000_hal_rf_read_ldo_tune(
    dw3000_device_t*       device,
    dw3000_rf_ldo_tune_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read(device, DW3000_REG_LDO_TUNE, value->bytes, sizeof(value->bytes));
}

dw3000_error_t dw3000_hal_rf_write_ldo_tune(
    dw3000_device_t*             device,
    const dw3000_rf_ldo_tune_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write(device, DW3000_REG_LDO_TUNE, value->bytes, sizeof(value->bytes));
}

dw3000_error_t dw3000_hal_rf_read_ldo_ctrl(
    dw3000_device_t*      device,
    dw3000_rf_ldo_ctrl_t* value
) {
    uint32_t raw;
    dw3000_error_t err;

    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_LDO_CTRL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *value = (dw3000_rf_ldo_ctrl_t)(raw & DW3000_HAL_RF_LDO_CTRL_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_rf_write_ldo_ctrl(
    dw3000_device_t*     device,
    dw3000_rf_ldo_ctrl_t value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)value & ~DW3000_HAL_RF_LDO_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_LDO_CTRL, (uint32_t)value);
}

dw3000_error_t dw3000_hal_rf_read_ldo_rload(
    dw3000_device_t*        device,
    dw3000_rf_ldo_rload_t*  value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_LDO_RLOAD, value);
}

dw3000_error_t dw3000_hal_rf_write_ldo_rload(
    dw3000_device_t*       device,
    dw3000_rf_ldo_rload_t  value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rf_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_LDO_RLOAD, value);
}
