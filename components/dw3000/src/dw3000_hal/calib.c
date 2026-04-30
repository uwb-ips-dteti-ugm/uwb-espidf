#include "dw3000_hal/calib.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_hal/cia.h"
#include "dw3000_hal/otp.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_hal/rf.h"
#include "dw3000_register.h"

#define DW3000_HAL_CALIB_READY_POLL_DELAY_US 100U

#define DW3000_HAL_CALIB_RX_CAL_MODE_MASK        0x00000003UL
#define DW3000_HAL_CALIB_RX_CAL_EN_MASK          ((uint32_t)DW3000_CALIB_RX_CAL_EN)
#define DW3000_HAL_CALIB_RX_CAL_COMP_DLY_SHIFT   16U
#define DW3000_HAL_CALIB_RX_CAL_COMP_DLY_MASK    \
    (0xFUL << DW3000_HAL_CALIB_RX_CAL_COMP_DLY_SHIFT)
#define DW3000_HAL_CALIB_RX_CAL_VALUE_MASK       \
    (DW3000_HAL_CALIB_RX_CAL_MODE_MASK | \
     DW3000_HAL_CALIB_RX_CAL_EN_MASK | \
     DW3000_HAL_CALIB_RX_CAL_COMP_DLY_MASK)
#define DW3000_HAL_CALIB_RX_CAL_RESULT_MASK      0x1FFFFFFFUL

#define DW3000_HAL_CALIB_SAR_READING_LEN         3U
#define DW3000_HAL_CALIB_SAR_TEST_RDEN           (1U << 2)

#define DW3000_HAL_CALIB_PGC_TMEAS_SHIFT         2U
#define DW3000_HAL_CALIB_PGC_TMEAS_MASK          0x3CU
#define DW3000_HAL_CALIB_PGC_DELAY_MASK          0x0FFFU
#define DW3000_HAL_CALIB_PG_TARGET_MASK          0x0FFFU

#define DW3000_HAL_CALIB_PLL_CAL_OWNED_MASK      \
    ((uint16_t)DW3000_PLL_CAL_USE_OLD | (uint16_t)DW3000_PLL_CAL_EN)
#define DW3000_HAL_CALIB_XTAL_TRIM_FALLBACK 0x2EU
#define DW3000_HAL_CALIB_RX_CAL_LDO_MASK \
    ((dw3000_rf_ldo_ctrl_t)(DW3000_RF_LDO_VDDIF2 | \
                            DW3000_RF_LDO_VDDMS3 | \
                            DW3000_RF_LDO_VDDMS1))
#define DW3000_HAL_REG_RX_CAL_BYTE_0 \
    DW3000_REG_DESC(DW3000_REG_FILE_EXT_SYNC, 0x000CU, 1U)
#define DW3000_HAL_REG_RX_CAL_BYTE_2 \
    DW3000_REG_DESC(DW3000_REG_FILE_EXT_SYNC, 0x000EU, 1U)
#define DW3000_HAL_CALIB_RX_CAL_READ_EN 0x01U
#define DW3000_HAL_CALIB_RX_CAL_READ_VALUE \
    ((uint8_t)(DW3000_CALIB_RX_CAL_COMP_DLY_OPT | \
               DW3000_HAL_CALIB_RX_CAL_READ_EN))

static bool dw3000_hal_calib_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_calib_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static uint32_t dw3000_hal_calib_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static uint32_t dw3000_hal_calib_tx_power_raw(
    const dw3000_calib_tx_power_t* tx_power
) {
    return (uint32_t)tx_power->data |
           ((uint32_t)tx_power->phr << 8U) |
           ((uint32_t)tx_power->shr << 16U) |
           ((uint32_t)tx_power->sts << 24U);
}

static dw3000_error_t dw3000_hal_calib_read_otp_xtal_trim(
    dw3000_device_t*             device,
    dw3000_calib_xtal_trim_t*    trim
) {
    dw3000_error_t      err;
    dw3000_otp_word_t   word;

    if ((device == NULL) || (trim == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_otp_read_word(device, DW3000_CALIB_OTP_XTAL, &word);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *trim = (dw3000_calib_xtal_trim_t)(word & DW3000_CALIB_XTAL_TRIM_MASK);
    if (*trim == 0U) {
        *trim = DW3000_HAL_CALIB_XTAL_TRIM_FALLBACK;
    }

    return DW3000_ERROR_OK;
}

static void dw3000_hal_calib_decode_tx_power(
    uint32_t                  raw,
    dw3000_calib_tx_power_t*  tx_power
) {
    tx_power->data = (uint8_t)(raw & 0xFFU);
    tx_power->phr  = (uint8_t)((raw >> 8U) & 0xFFU);
    tx_power->shr  = (uint8_t)((raw >> 16U) & 0xFFU);
    tx_power->sts  = (uint8_t)((raw >> 24U) & 0xFFU);
}

static uint32_t dw3000_hal_calib_rx_cal_value(
    dw3000_calib_rx_cal_mode_t mode,
    dw3000_calib_rx_cal_flags_t flags,
    uint8_t comp_dly
) {
    return ((uint32_t)mode & DW3000_HAL_CALIB_RX_CAL_MODE_MASK) |
           ((uint32_t)flags & DW3000_HAL_CALIB_RX_CAL_EN_MASK) |
           (((uint32_t)comp_dly << DW3000_HAL_CALIB_RX_CAL_COMP_DLY_SHIFT) &
            DW3000_HAL_CALIB_RX_CAL_COMP_DLY_MASK);
}

static dw3000_error_t dw3000_hal_calib_wait_sar_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t err;
    uint8_t        status;

    if ((timeout_us != 0U) &&
        (device->port.get_time_us == NULL) &&
        (device->port.delay_us == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (device->port.get_time_us != NULL) {
        uint64_t start_us = device->port.get_time_us(device->port.ctx);

        do {
            err = dw3000_reg_read_u8(device, DW3000_REG_SAR_STATUS, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if ((status & (uint8_t)DW3000_TX_CAL_SAR_DONE) != 0U) {
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_calib_delay_us(
                    device,
                    dw3000_hal_calib_min_u32(
                        remaining_us,
                        DW3000_HAL_CALIB_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_reg_read_u8(device, DW3000_REG_SAR_STATUS, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status & (uint8_t)DW3000_TX_CAL_SAR_DONE) != 0U) {
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_calib_min_u32(
            timeout_us,
            DW3000_HAL_CALIB_READY_POLL_DELAY_US
        );
        dw3000_hal_calib_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

static dw3000_error_t dw3000_hal_calib_wait_rx_cal_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t                  err;
    dw3000_calib_rx_cal_status_t    status;

    if ((timeout_us != 0U) &&
        (device->port.get_time_us == NULL) &&
        (device->port.delay_us == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (device->port.get_time_us != NULL) {
        uint64_t start_us = device->port.get_time_us(device->port.ctx);

        do {
            err = dw3000_hal_calib_read_rx_cal_status(device, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if ((status & DW3000_CALIB_RX_CAL_DONE) != 0U) {
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_calib_delay_us(
                    device,
                    dw3000_hal_calib_min_u32(
                        remaining_us,
                        DW3000_HAL_CALIB_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_hal_calib_read_rx_cal_status(device, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status & DW3000_CALIB_RX_CAL_DONE) != 0U) {
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_calib_min_u32(
            timeout_us,
            DW3000_HAL_CALIB_READY_POLL_DELAY_US
        );
        dw3000_hal_calib_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_calib_validate_config(
    const dw3000_calib_config_t* config
) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->xtal_trim & ~DW3000_CALIB_XTAL_TRIM_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_read_tx_power(
    dw3000_device_t*            device,
    dw3000_calib_tx_power_t*    tx_power
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (tx_power == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_TX_POWER, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_calib_decode_tx_power(raw, tx_power);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_set_tx_power(
    dw3000_device_t*                  device,
    const dw3000_calib_tx_power_t*    tx_power
) {
    dw3000_error_t err;

    if ((device == NULL) || (tx_power == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_calib_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_TX_POWER,
        dw3000_hal_calib_tx_power_raw(tx_power)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.calib.tx_power = *tx_power;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_read_xtal_trim(
    dw3000_device_t*              device,
    dw3000_calib_xtal_trim_t*     trim
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

dw3000_error_t dw3000_hal_calib_set_xtal_trim(
    dw3000_device_t*             device,
    dw3000_calib_xtal_trim_t     trim
) {
    dw3000_error_t err;
    uint8_t        raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_calib_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if ((trim & ~DW3000_CALIB_XTAL_TRIM_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_XTAL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    raw = (uint8_t)((raw & ~DW3000_CALIB_XTAL_TRIM_MASK) |
                    (trim & DW3000_CALIB_XTAL_TRIM_MASK));

    err = dw3000_reg_write_u8(device, DW3000_REG_XTAL, raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.calib.xtal_trim = trim;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_read_antenna_delays(
    dw3000_device_t*             device,
    dw3000_cia_antenna_delay_t*  tx_delay,
    dw3000_cia_antenna_delay_t*  rx_delay
) {
    dw3000_error_t         err;
    dw3000_cia_conf_flags_t flags;

    if ((device == NULL) || (tx_delay == NULL) || (rx_delay == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_cia_read_tx_antenna_delay(device, tx_delay);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_cia_read_conf(device, rx_delay, &flags);
}

dw3000_error_t dw3000_hal_calib_set_antenna_delays(
    dw3000_device_t*            device,
    dw3000_cia_antenna_delay_t  tx_delay,
    dw3000_cia_antenna_delay_t  rx_delay
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_cia_set_tx_antenna_delay(device, tx_delay);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_cia_set_rx_antenna_delay(device, rx_delay);
}

dw3000_error_t dw3000_hal_calib_read_otp_sar_reference(
    dw3000_device_t*                 device,
    dw3000_calib_sar_reference_t*    reference
) {
    dw3000_error_t      err;
    dw3000_otp_word_t   vbat;
    dw3000_otp_word_t   vtemp;

    if ((device == NULL) || (reference == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_otp_read_word(device, DW3000_CALIB_OTP_VBAT, &vbat);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_otp_read_word(device, DW3000_CALIB_OTP_VTEMP, &vtemp);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reference->vbat_1v62 = (uint8_t)(vbat & 0xFFU);
    reference->vbat_3v62 = (uint8_t)((vbat >> 8U) & 0xFFU);
    reference->vbat_3v0  = (uint8_t)((vbat >> 16U) & 0xFFU);
    reference->vtemp_22c = (uint8_t)(vtemp & 0xFFU);

    return DW3000_ERROR_OK;
}

int32_t dw3000_hal_calib_sar_vbat_millivolts(
    const dw3000_calib_sar_reference_t* reference,
    uint8_t                             raw_vbat
) {
    if (reference == NULL) {
        return 0;
    }

    return 3000 + (((int32_t)raw_vbat - (int32_t)reference->vbat_3v0) * 251) / 10;
}

int32_t dw3000_hal_calib_sar_temp_centi_celsius(
    const dw3000_calib_sar_reference_t* reference,
    uint8_t                             raw_temp
) {
    if (reference == NULL) {
        return 0;
    }

    return 2200 + (((int32_t)raw_temp - (int32_t)reference->vtemp_22c) * 105);
}

dw3000_error_t dw3000_hal_calib_convert_sar_reading(
    const dw3000_calib_sar_reference_t* reference,
    const dw3000_tx_cal_sar_reading_t*  reading,
    dw3000_calib_sar_converted_t*       converted
) {
    if ((reference == NULL) || (reading == NULL) || (converted == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    converted->millivolts = dw3000_hal_calib_sar_vbat_millivolts(
        reference,
        reading->vbat
    );
    converted->centi_celsius = dw3000_hal_calib_sar_temp_centi_celsius(
        reference,
        reading->vtemp
    );
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_read_sar(
    dw3000_device_t*                  device,
    dw3000_tx_cal_sar_reading_t*      reading
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_HAL_CALIB_SAR_READING_LEN];

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

dw3000_error_t dw3000_hal_calib_measure_sar(
    dw3000_device_t*                  device,
    uint32_t                          timeout_us,
    dw3000_tx_cal_sar_reading_t*      reading
) {
    dw3000_error_t          err;
    dw3000_pmsc_clk_ctrl_t  clock_ctrl;
    uint8_t                 sar_test;
    bool                    had_sar_clk;

    if ((device == NULL) || (reading == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_calib_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_pmsc_read_clock_ctrl(device, &clock_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    had_sar_clk = (clock_ctrl.flags & DW3000_PMSC_CLK_SAR_CLK_EN) != 0U;
    if (!had_sar_clk) {
        err = dw3000_hal_pmsc_set_clock_flags(device, DW3000_PMSC_CLK_SAR_CLK_EN);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_SAR_TEST, &sar_test);
    if (err != DW3000_ERROR_OK) {
        if (!had_sar_clk) {
            (void)dw3000_hal_pmsc_clear_clock_flags(device, DW3000_PMSC_CLK_SAR_CLK_EN);
        }
        return err;
    }

    err = dw3000_reg_write_u8(
        device,
        DW3000_REG_SAR_TEST,
        (uint8_t)(sar_test | DW3000_HAL_CALIB_SAR_TEST_RDEN)
    );
    if (err != DW3000_ERROR_OK) {
        (void)dw3000_reg_write_u8(device, DW3000_REG_SAR_TEST, sar_test);
        if (!had_sar_clk) {
            (void)dw3000_hal_pmsc_clear_clock_flags(device, DW3000_PMSC_CLK_SAR_CLK_EN);
        }
        return err;
    }

    err = dw3000_reg_write_u8(device, DW3000_REG_SAR_CTRL, DW3000_TX_CAL_SAR_START);
    if (err == DW3000_ERROR_OK) {
        err = dw3000_hal_calib_wait_sar_done(device, timeout_us);
    }
    if (err == DW3000_ERROR_OK) {
        err = dw3000_hal_calib_read_sar(device, reading);
    }

    (void)dw3000_reg_write_u8(device, DW3000_REG_SAR_CTRL, 0U);
    (void)dw3000_reg_write_u8(device, DW3000_REG_SAR_TEST, sar_test);

    if (!had_sar_clk) {
        (void)dw3000_hal_pmsc_clear_clock_flags(device, DW3000_PMSC_CLK_SAR_CLK_EN);
    }

    return err;
}

dw3000_error_t dw3000_hal_calib_read_sar_wake(
    dw3000_device_t*              device,
    dw3000_tx_cal_sar_wake_t*     reading
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

dw3000_error_t dw3000_hal_calib_read_rx_cal_status(
    dw3000_device_t*                  device,
    dw3000_calib_rx_cal_status_t*     status
) {
    dw3000_error_t err;
    uint8_t        raw;

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_RX_CAL_STS, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *status = (dw3000_calib_rx_cal_status_t)(raw & (uint8_t)DW3000_CALIB_RX_CAL_DONE);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_read_rx_cal_result(
    dw3000_device_t*                  device,
    dw3000_calib_rx_cal_result_t*     result
) {
    dw3000_error_t err;

    if ((device == NULL) || (result == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_RX_CAL_RESI, &result->i);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_RX_CAL_RESQ, &result->q);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    result->i &= DW3000_HAL_CALIB_RX_CAL_RESULT_MASK;
    result->q &= DW3000_HAL_CALIB_RX_CAL_RESULT_MASK;
    return DW3000_ERROR_OK;
}

bool dw3000_hal_calib_rx_cal_result_is_valid(
    const dw3000_calib_rx_cal_result_t* result
) {
    return (result != NULL) &&
           (result->i != DW3000_CALIB_RX_CAL_RESULT_FAIL) &&
           (result->q != DW3000_CALIB_RX_CAL_RESULT_FAIL);
}

dw3000_error_t dw3000_hal_calib_run_rx_calibration(
    dw3000_device_t*                  device,
    uint32_t                          timeout_us,
    dw3000_calib_rx_cal_result_t*     result
) {
    dw3000_error_t     err;
    dw3000_error_t     cleanup_err = DW3000_ERROR_OK;
    dw3000_rf_ldo_ctrl_t saved_ldo_ctrl;
    dw3000_rf_ldo_ctrl_t cal_ldo_ctrl;
    uint8_t             rx_cal_start;

    if ((device == NULL) || (result == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_calib_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_rf_read_ldo_ctrl(device, &saved_ldo_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    cal_ldo_ctrl = (dw3000_rf_ldo_ctrl_t)(
        (uint32_t)saved_ldo_ctrl | (uint32_t)DW3000_HAL_CALIB_RX_CAL_LDO_MASK
    );
    err = dw3000_hal_rf_write_ldo_ctrl(device, cal_ldo_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u8(device, DW3000_REG_RX_CAL_STS, DW3000_CALIB_RX_CAL_DONE);
    if (err != DW3000_ERROR_OK) {
        goto cleanup;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_RX_CAL,
        dw3000_hal_calib_rx_cal_value(
            DW3000_CALIB_RX_CAL_MODE_CALIBRATION,
            (dw3000_calib_rx_cal_flags_t)0U,
            DW3000_CALIB_RX_CAL_COMP_DLY_OPT
        )
    );
    rx_cal_start = (uint8_t)(
        (uint8_t)DW3000_CALIB_RX_CAL_MODE_CALIBRATION |
        (uint8_t)DW3000_CALIB_RX_CAL_EN
    );
    if (err == DW3000_ERROR_OK) {
        err = dw3000_reg_write_u8(
            device,
            DW3000_HAL_REG_RX_CAL_BYTE_0,
            rx_cal_start
        );
    }
    if (err == DW3000_ERROR_OK) {
        err = dw3000_hal_calib_wait_rx_cal_done(device, timeout_us);
    }
    if (err == DW3000_ERROR_OK) {
        err = dw3000_reg_write_u8(device, DW3000_HAL_REG_RX_CAL_BYTE_0, 0U);
    }
    if (err == DW3000_ERROR_OK) {
        err = dw3000_reg_write_u8(
            device,
            DW3000_REG_RX_CAL_STS,
            DW3000_CALIB_RX_CAL_DONE
        );
    }
    if (err == DW3000_ERROR_OK) {
        err = dw3000_reg_write_u8(
            device,
            DW3000_HAL_REG_RX_CAL_BYTE_2,
            DW3000_HAL_CALIB_RX_CAL_READ_VALUE
        );
    }
    if (err == DW3000_ERROR_OK) {
        err = dw3000_hal_calib_read_rx_cal_result(device, result);
    }
    if ((err == DW3000_ERROR_OK) &&
        !dw3000_hal_calib_rx_cal_result_is_valid(result)) {
        err = DW3000_ERROR_INVALID_STATE;
    }

cleanup:
    cleanup_err = dw3000_reg_write_u8(device, DW3000_HAL_REG_RX_CAL_BYTE_0, 0U);
    {
        dw3000_error_t clear_err = dw3000_reg_write_u8(
            device,
            DW3000_REG_RX_CAL_STS,
            DW3000_CALIB_RX_CAL_DONE
        );

        if (cleanup_err == DW3000_ERROR_OK) {
            cleanup_err = clear_err;
        }
    }
    {
        dw3000_error_t restore_err = dw3000_hal_rf_write_ldo_ctrl(
            device,
            saved_ldo_ctrl
        );

        if (cleanup_err == DW3000_ERROR_OK) {
            cleanup_err = restore_err;
        }
    }

    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return cleanup_err;
}

dw3000_error_t dw3000_hal_calib_read_pgc_status(
    dw3000_device_t*                 device,
    dw3000_tx_cal_pgc_status_t*      status
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

    status->delay = (uint16_t)(raw & DW3000_HAL_CALIB_PGC_DELAY_MASK);
    status->flags = (dw3000_tx_cal_pgc_status_flags_t)(
        raw & (uint16_t)DW3000_TX_CAL_PGC_AUTOCAL_DONE
    );
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_set_pgc_target(
    dw3000_device_t*                 device,
    dw3000_tx_cal_pg_target_t        target
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((target & ~DW3000_HAL_CALIB_PG_TARGET_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_write_u16(device, DW3000_REG_PG_CAL_TARGET, target);
}

dw3000_error_t dw3000_hal_calib_start_pgc_count(
    dw3000_device_t* device,
    uint8_t          tmeas
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((tmeas & ~0x0FU) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_write_u16(
        device,
        DW3000_REG_PGC_CTRL,
        (uint16_t)(
            DW3000_TX_CAL_PGC_START |
            ((uint16_t)tmeas << DW3000_HAL_CALIB_PGC_TMEAS_SHIFT)
        )
    );
}

dw3000_error_t dw3000_hal_calib_start_pgc_autocal(
    dw3000_device_t* device,
    uint8_t          tmeas
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((tmeas & ~0x0FU) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_write_u16(
        device,
        DW3000_REG_PGC_CTRL,
        (uint16_t)(
            DW3000_TX_CAL_PGC_AUTOCAL_EN |
            ((uint16_t)tmeas << DW3000_HAL_CALIB_PGC_TMEAS_SHIFT)
        )
    );
}

dw3000_error_t dw3000_hal_calib_wait_pgc_autocal_done(
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
            err = dw3000_hal_calib_read_pgc_status(device, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if ((status.flags & DW3000_TX_CAL_PGC_AUTOCAL_DONE) != 0U) {
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_calib_delay_us(
                    device,
                    dw3000_hal_calib_min_u32(
                        remaining_us,
                        DW3000_HAL_CALIB_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_hal_calib_read_pgc_status(device, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status.flags & DW3000_TX_CAL_PGC_AUTOCAL_DONE) != 0U) {
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_calib_min_u32(
            timeout_us,
            DW3000_HAL_CALIB_READY_POLL_DELAY_US
        );
        dw3000_hal_calib_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_calib_read_pll_coarse_code(
    dw3000_device_t*             device,
    dw3000_pll_coarse_code_t*    code
) {
    if ((device == NULL) || (code == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_PLL_CC, code);
}

dw3000_error_t dw3000_hal_calib_set_pll_coarse_code(
    dw3000_device_t*            device,
    dw3000_pll_coarse_code_t    code
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_calib_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_PLL_CC, code);
}

dw3000_error_t dw3000_hal_calib_read_otp_pll_lock_code(
    dw3000_device_t*             device,
    dw3000_pll_coarse_code_t*    code
) {
    dw3000_error_t      err;
    dw3000_otp_word_t   word;

    if ((device == NULL) || (code == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_otp_read_word(device, 0x35U, &word);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *code = (dw3000_pll_coarse_code_t)(word & 0xFFU);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_start_pll_calibration(
    dw3000_device_t*           device,
    dw3000_pll_cal_flags_t     flags
) {
    dw3000_error_t err;
    uint16_t       current;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)flags & ~DW3000_HAL_CALIB_PLL_CAL_OWNED_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PLL_CAL, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    current = (uint16_t)((current & ~DW3000_HAL_CALIB_PLL_CAL_OWNED_MASK) |
                         ((uint16_t)flags & DW3000_HAL_CALIB_PLL_CAL_OWNED_MASK));

    return dw3000_reg_write_u16(device, DW3000_REG_PLL_CAL, current);
}

dw3000_error_t dw3000_hal_calib_recalibrate_pll(
    dw3000_device_t*       device,
    dw3000_phy_channel_t   channel,
    uint32_t               timeout_us
) {
    dw3000_error_t             err;
    dw3000_pll_cal_flags_t     flags = DW3000_PLL_CAL_EN;
    dw3000_pll_coarse_code_t   code;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((channel != DW3000_PHY_CHANNEL_5) && (channel != DW3000_PHY_CHANNEL_9)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_calib_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_pmsc_force_idle_rc(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_calib_read_otp_pll_lock_code(device, &code);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (code != 0U) {
        err = dw3000_hal_calib_set_pll_coarse_code(device, code);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        flags = (dw3000_pll_cal_flags_t)(flags | DW3000_PLL_CAL_USE_OLD);
    }

    err = dw3000_hal_calib_start_pll_calibration(device, flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_pmsc_enter_idle_pll(device, timeout_us);
}

dw3000_error_t dw3000_hal_calib_apply_otp_factory(
    dw3000_device_t* device
) {
    dw3000_otp_ops_sel_t ops_sel;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    ops_sel = dw3000_hal_otp_ops_sel_for_preamble(device->config.phy.preamble_length);
    return dw3000_hal_otp_kick_factory_calibration(
        device,
        device->config.phy.channel,
        ops_sel
    );
}

dw3000_error_t dw3000_hal_calib_configure(
    dw3000_device_t*                device,
    const dw3000_calib_config_t*    config
) {
    dw3000_error_t err;
    dw3000_calib_config_t actual;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (config == NULL) {
        config = &device->config.calib;
    }
    actual = *config;

    err = dw3000_hal_calib_validate_config(&actual);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (device->config.load_otp_calibration) {
        err = dw3000_hal_calib_apply_otp_factory(device);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if (actual.xtal_trim == 0U) {
            err = dw3000_hal_calib_read_otp_xtal_trim(device, &actual.xtal_trim);
            if (err != DW3000_ERROR_OK) {
                return err;
            }
        }
    }

    err = dw3000_hal_calib_set_tx_power(device, &actual.tx_power);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (actual.xtal_trim != 0U) {
        err = dw3000_hal_calib_set_xtal_trim(device, actual.xtal_trim);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    device->config.calib = actual;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_calib_configure_current(dw3000_device_t* device) {
    return dw3000_hal_calib_configure(device, NULL);
}
