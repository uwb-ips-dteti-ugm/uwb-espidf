#include "dw3000_hal/aon.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_hal/calib.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/otp.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_register.h"

#define DW3000_HAL_AON_DIG_CFG_LEN 3U

#define DW3000_HAL_AON_DIG_CFG_MASK \
    ((uint32_t)DW3000_AON_DIG_ONW_AON_DLD | \
     (uint32_t)DW3000_AON_DIG_ONW_RUN_SAR | \
     (uint32_t)DW3000_AON_DIG_ONW_GO2IDLE | \
     (uint32_t)DW3000_AON_DIG_ONW_GO2RX | \
     (uint32_t)DW3000_AON_DIG_ONW_PGFCAL)

#define DW3000_HAL_AON_DIG_CFG_AUTO_PLL_MASK \
    ((uint32_t)DW3000_AON_DIG_ONW_GO2IDLE | \
     (uint32_t)DW3000_AON_DIG_ONW_GO2RX)

#define DW3000_HAL_AON_CFG_MASK \
    ((uint8_t)DW3000_AON_CFG_SLEEP_EN | \
     (uint8_t)DW3000_AON_CFG_WAKE_CNT | \
     (uint8_t)DW3000_AON_CFG_BROUT_EN | \
     (uint8_t)DW3000_AON_CFG_WAKE_CSN | \
     (uint8_t)DW3000_AON_CFG_WAKE_WUP | \
     (uint8_t)DW3000_AON_CFG_PRES_SLEEP)

#define DW3000_HAL_AON_DEEPSLEEP_WAKE_MASK \
    ((uint8_t)DW3000_AON_CFG_WAKE_CSN | \
     (uint8_t)DW3000_AON_CFG_WAKE_WUP)

static bool dw3000_hal_aon_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_aon_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static void dw3000_hal_aon_mark_sleeping(dw3000_device_t* device) {
    device->state_flags = (dw3000_device_state_flags_t)(
        (device->state_flags | DW3000_DEVICE_STATE_SLEEPING) &
        ~(DW3000_DEVICE_STATE_IDLE_RC |
          DW3000_DEVICE_STATE_IDLE_PLL |
          DW3000_DEVICE_STATE_RX_ON |
          DW3000_DEVICE_STATE_TX_PENDING)
    );
}

static void dw3000_hal_aon_decode_dig_cfg(
    const uint8_t*           raw,
    dw3000_aon_dig_cfg_t*    dig_cfg
) {
    uint32_t value = (uint32_t)raw[0] |
                     ((uint32_t)raw[1] << 8U) |
                     ((uint32_t)raw[2] << 16U);

    *dig_cfg = (dw3000_aon_dig_cfg_t)(value & DW3000_HAL_AON_DIG_CFG_MASK);
}

static void dw3000_hal_aon_overlay_dig_cfg(
    uint8_t*                raw,
    dw3000_aon_dig_cfg_t    dig_cfg
) {
    uint32_t current = (uint32_t)raw[0] |
                       ((uint32_t)raw[1] << 8U) |
                       ((uint32_t)raw[2] << 16U);

    current = (current & ~DW3000_HAL_AON_DIG_CFG_MASK) |
              ((uint32_t)dig_cfg & DW3000_HAL_AON_DIG_CFG_MASK);

    raw[0] = (uint8_t)(current & 0xFFU);
    raw[1] = (uint8_t)((current >> 8U) & 0xFFU);
    raw[2] = (uint8_t)((current >> 16U) & 0xFFU);
}

static dw3000_error_t dw3000_hal_aon_prepare_sleep(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config,
    bool                          deep_sleep
) {
    dw3000_error_t       err;
    dw3000_aon_config_t  actual;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    actual = (config != NULL) ? *config : device->config.aon;

    err = dw3000_hal_aon_validate_config(&actual);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (device->config.load_otp_calibration &&
        (((uint32_t)actual.dig_cfg & DW3000_HAL_AON_DIG_CFG_AUTO_PLL_MASK) != 0U)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    actual.cfg = (dw3000_aon_cfg_flags_t)(actual.cfg | DW3000_AON_CFG_SLEEP_EN);
    if (deep_sleep) {
        actual.cfg = (dw3000_aon_cfg_flags_t)(actual.cfg & ~DW3000_AON_CFG_WAKE_CNT);
        if (((uint8_t)actual.cfg & DW3000_HAL_AON_DEEPSLEEP_WAKE_MASK) == 0U) {
            return DW3000_ERROR_INVALID_ARG;
        }
    } else {
        if (actual.sleep_time == 0U) {
            return DW3000_ERROR_INVALID_ARG;
        }
        actual.cfg = (dw3000_aon_cfg_flags_t)(actual.cfg | DW3000_AON_CFG_WAKE_CNT);
    }

    if (!deep_sleep) {
        if (device->port.delay_us == NULL) {
            return DW3000_ERROR_INVALID_STATE;
        }

        err = dw3000_hal_aon_set_sleep_time(device, actual.sleep_time);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        dw3000_hal_aon_delay_us(device, DW3000_HAL_AON_SLEEP_TIMER_LOAD_DELAY_US);
    }

    err = dw3000_hal_aon_set_dig_cfg(device, actual.dig_cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aon_set_cfg(device, actual.cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_clear_seq_flags(device, DW3000_PMSC_SEQ_AINIT2IDLE);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aon_save(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aon = actual;
    dw3000_hal_aon_mark_sleeping(device);
    return DW3000_ERROR_OK;
}

void dw3000_hal_aon_default_wake_options(
    dw3000_hal_aon_wake_options_t* options
) {
    if (options == NULL) {
        return;
    }

    options->spi_ready_timeout_us = DW3000_HAL_AON_WAKE_TIMEOUT_US;
    options->pll_lock_timeout_us  = DW3000_HAL_AON_PLL_LOCK_TIMEOUT_US;
    options->rx_cal_timeout_us    = DW3000_HAL_AON_RX_CAL_TIMEOUT_US;
    options->reload_otp_ldo_bias  = true;
    options->run_rx_calibration   = false;
    options->enter_idle_pll       = true;
}

bool dw3000_hal_aon_addr_is_valid(dw3000_aon_addr_t addr) {
    return ((uint16_t)addr & ~DW3000_AON_ADDR_MASK) == 0U;
}

dw3000_error_t dw3000_hal_aon_validate_dig_cfg(dw3000_aon_dig_cfg_t dig_cfg) {
    if (((uint32_t)dig_cfg & ~DW3000_HAL_AON_DIG_CFG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_validate_cfg(dw3000_aon_cfg_flags_t cfg) {
    if (((uint8_t)cfg & ~DW3000_HAL_AON_CFG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_validate_config(
    const dw3000_aon_config_t* config
) {
    dw3000_error_t err;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_aon_validate_dig_cfg(config->dig_cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_aon_validate_cfg(config->cfg);
}

dw3000_error_t dw3000_hal_aon_sleep_time_from_us(
    uint32_t                    sleep_us,
    uint32_t                    lp_osc_hz,
    dw3000_aon_sleep_time_t*    sleep_time
) {
    uint64_t numerator;
    uint64_t denominator;
    uint64_t value;

    if ((sleep_us == 0U) || (lp_osc_hz == 0U) || (sleep_time == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    numerator = (uint64_t)sleep_us * (uint64_t)lp_osc_hz;
    denominator = UINT64_C(1000000) * (uint64_t)DW3000_AON_SLEEP_TIM_LP_TICKS;
    value = (numerator + denominator - 1U) / denominator;
    if (value == 0U) {
        value = 1U;
    }

    if (value > UINT16_MAX) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    *sleep_time = (dw3000_aon_sleep_time_t)value;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_read_dig_cfg(
    dw3000_device_t*        device,
    dw3000_aon_dig_cfg_t*   dig_cfg
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_HAL_AON_DIG_CFG_LEN];

    if ((device == NULL) || (dig_cfg == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_AON_DIG_CFG, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aon_decode_dig_cfg(raw, dig_cfg);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_set_dig_cfg(
    dw3000_device_t*       device,
    dw3000_aon_dig_cfg_t   dig_cfg
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_HAL_AON_DIG_CFG_LEN];

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_aon_validate_dig_cfg(dig_cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read(device, DW3000_REG_AON_DIG_CFG, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aon_overlay_dig_cfg(raw, dig_cfg);

    err = dw3000_reg_write(device, DW3000_REG_AON_DIG_CFG, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aon.dig_cfg = dig_cfg;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_read_cfg(
    dw3000_device_t*           device,
    dw3000_aon_cfg_flags_t*    cfg
) {
    dw3000_error_t err;
    uint8_t        raw;

    if ((device == NULL) || (cfg == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_AON_CFG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *cfg = (dw3000_aon_cfg_flags_t)(raw & DW3000_HAL_AON_CFG_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_set_cfg(
    dw3000_device_t*          device,
    dw3000_aon_cfg_flags_t    cfg
) {
    dw3000_error_t err;
    uint8_t        raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_aon_validate_cfg(cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_AON_CFG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    raw = (uint8_t)((raw & ~DW3000_HAL_AON_CFG_MASK) |
                    ((uint8_t)cfg & DW3000_HAL_AON_CFG_MASK));

    err = dw3000_reg_write_u8(device, DW3000_REG_AON_CFG, raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aon.cfg = cfg;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_read_memory_byte(
    dw3000_device_t*       device,
    dw3000_aon_addr_t      addr,
    dw3000_aon_byte_t*     value
) {
    dw3000_error_t err;
    dw3000_error_t clear_err;

    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (!dw3000_hal_aon_addr_is_valid(addr)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_AON_ADDR,
        (uint16_t)(addr & DW3000_AON_ADDR_MASK)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u8(device, DW3000_REG_AON_CTRL, DW3000_AON_CTRL_DCA_ENAB);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u8(
        device,
        DW3000_REG_AON_CTRL,
        (uint8_t)(DW3000_AON_CTRL_DCA_ENAB | DW3000_AON_CTRL_DCA_READ)
    );
    if (err == DW3000_ERROR_OK) {
        err = dw3000_reg_read_u8(device, DW3000_REG_AON_RDATA, value);
    }

    clear_err = dw3000_reg_write_u8(device, DW3000_REG_AON_CTRL, 0U);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return clear_err;
}

dw3000_error_t dw3000_hal_aon_write_memory_byte(
    dw3000_device_t*       device,
    dw3000_aon_addr_t      addr,
    dw3000_aon_byte_t      value
) {
    dw3000_error_t err;
    dw3000_error_t clear_err;
    uint8_t        command;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (!dw3000_hal_aon_addr_is_valid(addr)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_write_u8(device, DW3000_REG_AON_WDATA, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_AON_ADDR,
        (uint16_t)(addr & DW3000_AON_ADDR_MASK)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    command = (uint8_t)DW3000_AON_CTRL_DCA_WRITE;
    if (addr >= 0x0100U) {
        command = (uint8_t)(command | DW3000_AON_CTRL_DCA_WRITE_HI);
    }

    err = dw3000_reg_write_u8(device, DW3000_REG_AON_CTRL, command);
    if (err == DW3000_ERROR_OK) {
        err = dw3000_reg_write_u8(
            device,
            DW3000_REG_AON_CTRL,
            (uint8_t)DW3000_AON_CTRL_DCA_ENAB
        );
    }

    clear_err = dw3000_reg_write_u8(device, DW3000_REG_AON_CTRL, 0U);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return clear_err;
}

dw3000_error_t dw3000_hal_aon_read_memory(
    dw3000_device_t*       device,
    dw3000_aon_addr_t      start_addr,
    dw3000_aon_byte_t*     data,
    size_t                 data_len
) {
    if ((device == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (!dw3000_hal_aon_addr_is_valid(start_addr) ||
        (data_len > ((size_t)DW3000_AON_ADDR_MASK + 1U)) ||
        (((size_t)start_addr + data_len - 1U) > (size_t)DW3000_AON_ADDR_MASK)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    for (size_t i = 0U; i < data_len; ++i) {
        dw3000_error_t err = dw3000_hal_aon_read_memory_byte(
            device,
            (dw3000_aon_addr_t)(start_addr + (dw3000_aon_addr_t)i),
            &data[i]
        );
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_write_memory(
    dw3000_device_t*           device,
    dw3000_aon_addr_t          start_addr,
    const dw3000_aon_byte_t*   data,
    size_t                     data_len
) {
    if ((device == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (!dw3000_hal_aon_addr_is_valid(start_addr) ||
        (data_len > ((size_t)DW3000_AON_ADDR_MASK + 1U)) ||
        (((size_t)start_addr + data_len - 1U) > (size_t)DW3000_AON_ADDR_MASK)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    for (size_t i = 0U; i < data_len; ++i) {
        dw3000_error_t err = dw3000_hal_aon_write_memory_byte(
            device,
            (dw3000_aon_addr_t)(start_addr + (dw3000_aon_addr_t)i),
            data[i]
        );
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_read_sleep_time(
    dw3000_device_t*           device,
    dw3000_aon_sleep_time_t*   sleep_time
) {
    dw3000_error_t err;
    uint8_t        raw[sizeof(*sleep_time)];

    if ((device == NULL) || (sleep_time == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_aon_read_memory(
        device,
        DW3000_AON_SLEEP_TIM_ADDR_LO,
        raw,
        sizeof(raw)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *sleep_time = (dw3000_aon_sleep_time_t)(
        (uint16_t)raw[0] | ((uint16_t)raw[1] << 8U)
    );
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_set_sleep_time(
    dw3000_device_t*          device,
    dw3000_aon_sleep_time_t   sleep_time
) {
    dw3000_error_t err;
    uint8_t        raw[sizeof(sleep_time)];

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    raw[0] = (uint8_t)(sleep_time & 0xFFU);
    raw[1] = (uint8_t)((sleep_time >> 8U) & 0xFFU);

    err = dw3000_hal_aon_write_memory(
        device,
        DW3000_AON_SLEEP_TIM_ADDR_LO,
        raw,
        sizeof(raw)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aon.sleep_time = sleep_time;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_upload_config(dw3000_device_t* device) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write_u8(
        device,
        DW3000_REG_AON_CTRL,
        (uint8_t)DW3000_AON_CTRL_CFG_UPLOAD
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aon_delay_us(device, DW3000_HAL_AON_CFG_UPLOAD_SETTLE_US);
    return dw3000_reg_write_u8(device, DW3000_REG_AON_CTRL, 0U);
}

dw3000_error_t dw3000_hal_aon_save(dw3000_device_t* device) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(
        device,
        DW3000_REG_AON_CTRL,
        (uint8_t)DW3000_AON_CTRL_SAVE
    );
}

dw3000_error_t dw3000_hal_aon_restore(dw3000_device_t* device) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aon_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(
        device,
        DW3000_REG_AON_CTRL,
        (uint8_t)DW3000_AON_CTRL_RESTORE
    );
}

dw3000_error_t dw3000_hal_aon_configure(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config
) {
    dw3000_error_t       err;
    dw3000_aon_config_t  actual;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    actual = (config != NULL) ? *config : device->config.aon;

    err = dw3000_hal_aon_validate_config(&actual);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (((uint8_t)actual.cfg & (uint8_t)DW3000_AON_CFG_WAKE_CNT) != 0U &&
        actual.sleep_time != 0U) {
        if (device->port.delay_us == NULL) {
            return DW3000_ERROR_INVALID_STATE;
        }

        err = dw3000_hal_aon_set_sleep_time(device, actual.sleep_time);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        dw3000_hal_aon_delay_us(device, DW3000_HAL_AON_SLEEP_TIMER_LOAD_DELAY_US);
    }

    err = dw3000_hal_aon_set_dig_cfg(device, actual.dig_cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aon_set_cfg(device, actual.cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aon_upload_config(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aon = actual;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aon_configure_current(dw3000_device_t* device) {
    return dw3000_hal_aon_configure(device, NULL);
}

dw3000_error_t dw3000_hal_aon_enter_sleep(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config
) {
    return dw3000_hal_aon_prepare_sleep(device, config, false);
}

dw3000_error_t dw3000_hal_aon_enter_deepsleep(
    dw3000_device_t*              device,
    const dw3000_aon_config_t*    config
) {
    return dw3000_hal_aon_prepare_sleep(device, config, true);
}

dw3000_error_t dw3000_hal_aon_finish_wake(
    dw3000_device_t*                          device,
    const dw3000_hal_aon_wake_options_t*      options,
    dw3000_calib_rx_cal_result_t*             rx_cal_result
) {
    dw3000_error_t                    err;
    dw3000_hal_aon_wake_options_t     default_options;
    dw3000_calib_rx_cal_result_t      local_rx_cal_result;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (options == NULL) {
        dw3000_hal_aon_default_wake_options(&default_options);
        default_options.enter_idle_pll = device->config.auto_init_pll;
        options = &default_options;
    }

    err = dw3000_hal_wait_for_spi_ready(device, options->spi_ready_timeout_us);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aon.cfg = (dw3000_aon_cfg_flags_t)(
        device->config.aon.cfg & ~DW3000_AON_CFG_SLEEP_EN
    );
    if (((uint8_t)device->config.aon.cfg & (uint8_t)DW3000_AON_CFG_PRES_SLEEP) == 0U) {
        device->config.pmsc.seq_flags = (dw3000_pmsc_seq_ctrl_flags_t)(
            device->config.pmsc.seq_flags &
            ~(DW3000_PMSC_SEQ_ATX2SLP | DW3000_PMSC_SEQ_ARX2SLP)
        );
    }

    if (options->reload_otp_ldo_bias && device->config.load_otp_calibration) {
        err = dw3000_hal_otp_kick_ldo(device);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        err = dw3000_hal_otp_kick_bias(device);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (options->run_rx_calibration) {
        err = dw3000_hal_calib_run_rx_calibration(
            device,
            options->rx_cal_timeout_us,
            (rx_cal_result != NULL) ? rx_cal_result : &local_rx_cal_result
        );
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (options->enter_idle_pll) {
        err = dw3000_hal_pmsc_enter_idle_pll(device, options->pll_lock_timeout_us);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return DW3000_ERROR_OK;
}
