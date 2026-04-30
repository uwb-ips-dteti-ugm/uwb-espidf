#include "dw3000_hal/tx_cal.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_TX_CAL_SAR_READING_LEN 3U
#define DW3000_HAL_TX_CAL_PGC_TMEAS_SHIFT 2U
#define DW3000_HAL_TX_CAL_PGC_CTRL_MASK  \
    ((uint16_t)DW3000_TX_CAL_PGC_START | \
     (uint16_t)DW3000_TX_CAL_PGC_AUTOCAL_EN)
#define DW3000_HAL_TX_CAL_PGC_TMEAS_MASK \
    ((uint16_t)DW3000_TX_CAL_PGC_TMEAS_MAX << DW3000_HAL_TX_CAL_PGC_TMEAS_SHIFT)

static bool dw3000_hal_tx_cal_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_tx_cal_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static uint32_t dw3000_hal_tx_cal_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static bool dw3000_hal_tx_cal_pg_test_is_valid(
    dw3000_tx_cal_pg_test_t value
) {
    return (value == DW3000_TX_CAL_PG_TEST_NORMAL) ||
           (value == DW3000_TX_CAL_PG_TEST_CW);
}

dw3000_error_t dw3000_hal_tx_cal_validate_config(
    const dw3000_tx_cal_config_t* config
) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)config->pgc_ctrl & ~DW3000_HAL_TX_CAL_PGC_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->pg_target & ~DW3000_TX_CAL_PG_TARGET_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_cal_read_sar_status(
    dw3000_device_t*            device,
    dw3000_tx_cal_sar_status_t* status
) {
    uint8_t        raw;
    dw3000_error_t err;

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_SAR_STATUS, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *status = (dw3000_tx_cal_sar_status_t)(raw & (uint8_t)DW3000_TX_CAL_SAR_DONE);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_cal_write_sar_ctrl(
    dw3000_device_t*         device,
    dw3000_tx_cal_sar_ctrl_t ctrl
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint8_t)ctrl & ~(uint8_t)DW3000_TX_CAL_SAR_START) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_cal_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_SAR_CTRL, (uint8_t)ctrl);
}

dw3000_error_t dw3000_hal_tx_cal_start_sar(dw3000_device_t* device) {
    return dw3000_hal_tx_cal_write_sar_ctrl(device, DW3000_TX_CAL_SAR_START);
}

dw3000_error_t dw3000_hal_tx_cal_read_sar(
    dw3000_device_t*             device,
    dw3000_tx_cal_sar_reading_t* reading
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_HAL_TX_CAL_SAR_READING_LEN];

    if ((device == NULL) || (reading == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_SAR_READING, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reading->vbat  = raw[0];
    reading->vtemp = raw[1];
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_cal_read_sar_wake(
    dw3000_device_t*          device,
    dw3000_tx_cal_sar_wake_t* reading
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (reading == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_SAR_WAKE_RD, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reading->vbat  = (uint8_t)(raw & 0xFFU);
    reading->vtemp = (uint8_t)((raw >> 8U) & 0xFFU);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_cal_read_pgc_ctrl(
    dw3000_device_t*          device,
    dw3000_tx_cal_pgc_ctrl_t* ctrl,
    uint8_t*                  tmeas
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (ctrl == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PGC_CTRL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *ctrl = (dw3000_tx_cal_pgc_ctrl_t)(raw & DW3000_HAL_TX_CAL_PGC_CTRL_MASK);
    if (tmeas != NULL) {
        *tmeas = (uint8_t)((raw & DW3000_HAL_TX_CAL_PGC_TMEAS_MASK) >>
                           DW3000_HAL_TX_CAL_PGC_TMEAS_SHIFT);
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_cal_write_pgc_ctrl(
    dw3000_device_t*         device,
    dw3000_tx_cal_pgc_ctrl_t ctrl,
    uint8_t                  tmeas
) {
    uint16_t raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)ctrl & ~DW3000_HAL_TX_CAL_PGC_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (tmeas > DW3000_TX_CAL_PGC_TMEAS_MAX) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_cal_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    raw = ((uint16_t)ctrl & DW3000_HAL_TX_CAL_PGC_CTRL_MASK) |
          (((uint16_t)tmeas << DW3000_HAL_TX_CAL_PGC_TMEAS_SHIFT) &
           DW3000_HAL_TX_CAL_PGC_TMEAS_MASK);

    return dw3000_reg_write_u16(device, DW3000_REG_PGC_CTRL, raw);
}

dw3000_error_t dw3000_hal_tx_cal_start_pgc_count(
    dw3000_device_t* device,
    uint8_t          tmeas
) {
    return dw3000_hal_tx_cal_write_pgc_ctrl(
        device,
        DW3000_TX_CAL_PGC_START,
        tmeas
    );
}

dw3000_error_t dw3000_hal_tx_cal_start_pgc_autocal(
    dw3000_device_t* device,
    uint8_t          tmeas
) {
    return dw3000_hal_tx_cal_write_pgc_ctrl(
        device,
        DW3000_TX_CAL_PGC_AUTOCAL_EN,
        tmeas
    );
}

dw3000_error_t dw3000_hal_tx_cal_read_pgc_status(
    dw3000_device_t*            device,
    dw3000_tx_cal_pgc_status_t* status
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PGC_STATUS, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    status->delay = (uint16_t)(raw & DW3000_TX_CAL_PGC_DELAY_MASK);
    status->flags = (dw3000_tx_cal_pgc_status_flags_t)(raw & (uint16_t)DW3000_TX_CAL_PGC_AUTOCAL_DONE);
    return DW3000_ERROR_OK;
}

bool dw3000_hal_tx_cal_pgc_autocal_done(
    const dw3000_tx_cal_pgc_status_t* status
) {
    return (status != NULL) &&
           ((status->flags & DW3000_TX_CAL_PGC_AUTOCAL_DONE) != 0U);
}

dw3000_error_t dw3000_hal_tx_cal_wait_pgc_autocal_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t             err;
    dw3000_tx_cal_pgc_status_t status;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((timeout_us != 0U) &&
        (device->port.get_time_us == NULL) &&
        (device->port.delay_us == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (device->port.get_time_us != NULL) {
        uint64_t start_us = device->port.get_time_us(device->port.ctx);

        do {
            err = dw3000_hal_tx_cal_read_pgc_status(device, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if (dw3000_hal_tx_cal_pgc_autocal_done(&status)) {
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_tx_cal_delay_us(
                    device,
                    dw3000_hal_tx_cal_min_u32(
                        remaining_us,
                        DW3000_HAL_TX_CAL_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_hal_tx_cal_read_pgc_status(device, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if (dw3000_hal_tx_cal_pgc_autocal_done(&status)) {
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_tx_cal_min_u32(
            timeout_us,
            DW3000_HAL_TX_CAL_READY_POLL_DELAY_US
        );
        dw3000_hal_tx_cal_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_tx_cal_read_pg_test(
    dw3000_device_t*         device,
    dw3000_tx_cal_pg_test_t* value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u16(device, DW3000_REG_PG_TEST, value);
}

dw3000_error_t dw3000_hal_tx_cal_write_pg_test(
    dw3000_device_t*        device,
    dw3000_tx_cal_pg_test_t value
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_cal_pg_test_is_valid(value)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_cal_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u16(device, DW3000_REG_PG_TEST, value);
}

dw3000_error_t dw3000_hal_tx_cal_read_pg_target(
    dw3000_device_t*           device,
    dw3000_tx_cal_pg_target_t* target
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (target == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PG_CAL_TARGET, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *target = (dw3000_tx_cal_pg_target_t)(raw & DW3000_TX_CAL_PG_TARGET_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_cal_write_pg_target(
    dw3000_device_t*          device,
    dw3000_tx_cal_pg_target_t target
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((target & ~DW3000_TX_CAL_PG_TARGET_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_cal_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u16(device, DW3000_REG_PG_CAL_TARGET, target);
}

dw3000_error_t dw3000_hal_tx_cal_configure(
    dw3000_device_t*              device,
    const dw3000_tx_cal_config_t* config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_tx_cal_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_tx_cal_write_pg_target(device, config->pg_target);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_tx_cal_write_pgc_ctrl(device, config->pgc_ctrl, 0U);
}
