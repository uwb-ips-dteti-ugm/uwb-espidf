#include "dw3000_hal/otp.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_OTP_READY_POLL_DELAY_US 100U

#define DW3000_HAL_OTP_CFG_MANUAL_ACCESS_MASK \
    ((uint16_t)DW3000_OTP_CFG_MAN | \
     (uint16_t)DW3000_OTP_CFG_READ | \
     (uint16_t)DW3000_OTP_CFG_WRITE | \
     (uint16_t)DW3000_OTP_CFG_WRITE_MR)

#define DW3000_HAL_OTP_KICK_MASK \
    ((uint16_t)DW3000_OTP_CFG_DGC_KICK | \
     (uint16_t)DW3000_OTP_CFG_LDO_KICK | \
     (uint16_t)DW3000_OTP_CFG_BIAS_KICK | \
     (uint16_t)DW3000_OTP_CFG_OPS_KICK)

#define DW3000_HAL_OTP_CFG_SELECT_MASK \
    ((uint16_t)DW3000_OTP_OPS_SEL_MASK | (uint16_t)DW3000_OTP_CFG_DGC_SEL)

#define DW3000_HAL_OTP_CFG_KICK_OWNED_MASK \
    (DW3000_HAL_OTP_KICK_MASK | DW3000_HAL_OTP_CFG_SELECT_MASK)

static bool dw3000_hal_otp_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_otp_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static uint32_t dw3000_hal_otp_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static bool dw3000_hal_otp_channel_is_valid(dw3000_phy_channel_t channel) {
    return (channel == DW3000_PHY_CHANNEL_5) || (channel == DW3000_PHY_CHANNEL_9);
}

static dw3000_error_t dw3000_hal_otp_read_cfg_raw(
    dw3000_device_t* device,
    uint16_t*        raw
) {
    if ((device == NULL) || (raw == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u16(device, DW3000_REG_OTP_CFG, raw);
}

static dw3000_error_t dw3000_hal_otp_write_cfg_masked(
    dw3000_device_t* device,
    uint16_t         mask,
    uint16_t         value
) {
    dw3000_error_t err;
    uint16_t       current;

    err = dw3000_hal_otp_read_cfg_raw(device, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    current = (uint16_t)((current & ~mask) | (value & mask));
    return dw3000_reg_write_u16(device, DW3000_REG_OTP_CFG, current);
}

static uint16_t dw3000_hal_otp_build_kick_cfg(
    dw3000_otp_cfg_flags_t kicks,
    dw3000_phy_channel_t   dgc_channel,
    dw3000_otp_ops_sel_t   ops_sel
) {
    uint16_t value = (uint16_t)kicks;

    if (((uint16_t)kicks & (uint16_t)DW3000_OTP_CFG_DGC_KICK) != 0U) {
        if (dgc_channel == DW3000_PHY_CHANNEL_9) {
            value = (uint16_t)(value | (uint16_t)DW3000_OTP_CFG_DGC_SEL);
        }
    }

    if (((uint16_t)kicks & (uint16_t)DW3000_OTP_CFG_OPS_KICK) != 0U) {
        value = (uint16_t)(
            value |
            (((uint16_t)ops_sel << DW3000_OTP_OPS_SEL_SHIFT) &
             (uint16_t)DW3000_OTP_OPS_SEL_MASK)
        );
    }

    return value;
}

void dw3000_hal_otp_default_program_options(
    dw3000_hal_otp_program_options_t* options
) {
    if (options == NULL) {
        return;
    }

    options->allow_non_customer_addr = false;
    options->allow_non_empty_word    = false;
    options->require_vpp_ok          = true;
    options->timeout_us              = DW3000_HAL_OTP_PROGRAM_DEFAULT_TIMEOUT_US;
}

bool dw3000_hal_otp_addr_is_valid(dw3000_otp_addr_t addr) {
    return ((uint16_t)addr & ~DW3000_OTP_ADDR_MASK) == 0U;
}

bool dw3000_hal_otp_addr_is_customer(dw3000_otp_addr_t addr) {
    if (!dw3000_hal_otp_addr_is_valid(addr)) {
        return false;
    }

    return (addr <= 0x03U) ||
           ((addr >= 0x10U) && (addr <= 0x1FU)) ||
           ((addr >= 0x36U) && (addr <= 0x5FU)) ||
           ((addr >= 0x62U) && (addr <= 0x7FU));
}

bool dw3000_hal_otp_ops_sel_is_valid(dw3000_otp_ops_sel_t ops_sel) {
    return (ops_sel == DW3000_OTP_OPS_SEL_LONG) ||
           (ops_sel == DW3000_OTP_OPS_SEL_SHORT);
}

dw3000_otp_ops_sel_t dw3000_hal_otp_ops_sel_for_preamble(
    dw3000_phy_preamble_length_t preamble_length
) {
    if ((preamble_length == DW3000_PHY_PREAMBLE_LEN_32) ||
        (preamble_length == DW3000_PHY_PREAMBLE_LEN_64) ||
        (preamble_length == DW3000_PHY_PREAMBLE_LEN_128)) {
        return DW3000_OTP_OPS_SEL_SHORT;
    }

    return DW3000_OTP_OPS_SEL_LONG;
}

dw3000_error_t dw3000_hal_otp_read_config(
    dw3000_device_t*          device,
    dw3000_otp_cfg_flags_t*   flags,
    dw3000_otp_ops_sel_t*     ops_sel,
    dw3000_phy_channel_t*     dgc_channel
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (flags == NULL) || (ops_sel == NULL) ||
        (dgc_channel == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_otp_read_cfg_raw(device, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *flags = (dw3000_otp_cfg_flags_t)(
        raw & (DW3000_HAL_OTP_CFG_MANUAL_ACCESS_MASK | DW3000_HAL_OTP_KICK_MASK)
    );
    *ops_sel = (dw3000_otp_ops_sel_t)(
        (raw & (uint16_t)DW3000_OTP_OPS_SEL_MASK) >> DW3000_OTP_OPS_SEL_SHIFT
    );
    *dgc_channel = ((raw & (uint16_t)DW3000_OTP_CFG_DGC_SEL) != 0U) ?
        DW3000_PHY_CHANNEL_9 :
        DW3000_PHY_CHANNEL_5;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_otp_read_status(
    dw3000_device_t*      device,
    dw3000_otp_status_t*  status
) {
    dw3000_error_t err;
    uint8_t        raw;

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_OTP_STAT, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *status = (dw3000_otp_status_t)(raw & ((uint8_t)DW3000_OTP_STAT_PROG_DONE |
                                           (uint8_t)DW3000_OTP_STAT_VPP_OK));
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_otp_read_word(
    dw3000_device_t*    device,
    dw3000_otp_addr_t   addr,
    dw3000_otp_word_t*  word
) {
    dw3000_error_t err;
    dw3000_error_t clear_err;

    if ((device == NULL) || (word == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_otp_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (!dw3000_hal_otp_addr_is_valid(addr)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_OTP_ADDR, addr);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_otp_write_cfg_masked(
        device,
        DW3000_HAL_OTP_CFG_MANUAL_ACCESS_MASK,
        (uint16_t)DW3000_OTP_CFG_MAN | (uint16_t)DW3000_OTP_CFG_READ
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_OTP_RDATA, word);

    clear_err = dw3000_hal_otp_write_cfg_masked(
        device,
        (uint16_t)DW3000_OTP_CFG_MAN | (uint16_t)DW3000_OTP_CFG_READ,
        0U
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return clear_err;
}

dw3000_error_t dw3000_hal_otp_read_words(
    dw3000_device_t*    device,
    dw3000_otp_addr_t   start_addr,
    dw3000_otp_word_t*  words,
    size_t              word_count
) {
    if ((device == NULL) || ((word_count != 0U) && (words == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (word_count == 0U) {
        return DW3000_ERROR_OK;
    }

    if (!dw3000_hal_otp_addr_is_valid(start_addr) ||
        (word_count > ((size_t)DW3000_OTP_ADDR_MASK + 1U)) ||
        (((size_t)start_addr + word_count - 1U) > (size_t)DW3000_OTP_ADDR_MASK)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    for (size_t i = 0U; i < word_count; ++i) {
        dw3000_error_t err = dw3000_hal_otp_read_word(
            device,
            (dw3000_otp_addr_t)(start_addr + (dw3000_otp_addr_t)i),
            &words[i]
        );
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_otp_read_special_register(
    dw3000_device_t*    device,
    dw3000_otp_word_t*  word
) {
    if ((device == NULL) || (word == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_OTP_SRDATA, word);
}

dw3000_error_t dw3000_hal_otp_kick(
    dw3000_device_t*         device,
    dw3000_otp_cfg_flags_t   kicks,
    dw3000_phy_channel_t     dgc_channel,
    dw3000_otp_ops_sel_t     ops_sel
) {
    uint16_t kick_bits = (uint16_t)kicks;
    uint16_t value;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_otp_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if ((kick_bits & ~DW3000_HAL_OTP_KICK_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((kick_bits == 0U)) {
        return DW3000_ERROR_OK;
    }

    if (((kick_bits & (uint16_t)DW3000_OTP_CFG_DGC_KICK) != 0U) &&
        !dw3000_hal_otp_channel_is_valid(dgc_channel)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((kick_bits & (uint16_t)DW3000_OTP_CFG_OPS_KICK) != 0U) &&
        !dw3000_hal_otp_ops_sel_is_valid(ops_sel)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    value = dw3000_hal_otp_build_kick_cfg(kicks, dgc_channel, ops_sel);
    return dw3000_hal_otp_write_cfg_masked(
        device,
        DW3000_HAL_OTP_CFG_KICK_OWNED_MASK,
        value
    );
}

dw3000_error_t dw3000_hal_otp_kick_dgc(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
) {
    return dw3000_hal_otp_kick(
        device,
        DW3000_OTP_CFG_DGC_KICK,
        channel,
        DW3000_OTP_OPS_SEL_SHORT
    );
}

dw3000_error_t dw3000_hal_otp_kick_ldo(dw3000_device_t* device) {
    return dw3000_hal_otp_kick(
        device,
        DW3000_OTP_CFG_LDO_KICK,
        DW3000_PHY_CHANNEL_5,
        DW3000_OTP_OPS_SEL_SHORT
    );
}

dw3000_error_t dw3000_hal_otp_kick_bias(dw3000_device_t* device) {
    return dw3000_hal_otp_kick(
        device,
        DW3000_OTP_CFG_BIAS_KICK,
        DW3000_PHY_CHANNEL_5,
        DW3000_OTP_OPS_SEL_SHORT
    );
}

dw3000_error_t dw3000_hal_otp_kick_ops(
    dw3000_device_t*      device,
    dw3000_otp_ops_sel_t  ops_sel
) {
    return dw3000_hal_otp_kick(
        device,
        DW3000_OTP_CFG_OPS_KICK,
        DW3000_PHY_CHANNEL_5,
        ops_sel
    );
}

dw3000_error_t dw3000_hal_otp_kick_factory_calibration(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel,
    dw3000_otp_ops_sel_t  ops_sel
) {
    return dw3000_hal_otp_kick(
        device,
        (dw3000_otp_cfg_flags_t)(
            DW3000_OTP_CFG_DGC_KICK |
            DW3000_OTP_CFG_LDO_KICK |
            DW3000_OTP_CFG_BIAS_KICK |
            DW3000_OTP_CFG_OPS_KICK
        ),
        channel,
        ops_sel
    );
}

dw3000_error_t dw3000_hal_otp_wait_program_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t      err;
    dw3000_otp_status_t status;

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
            err = dw3000_hal_otp_read_status(device, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if ((status & DW3000_OTP_STAT_PROG_DONE) != 0U) {
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_otp_delay_us(
                    device,
                    dw3000_hal_otp_min_u32(
                        remaining_us,
                        DW3000_HAL_OTP_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_hal_otp_read_status(device, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status & DW3000_OTP_STAT_PROG_DONE) != 0U) {
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_otp_min_u32(
            timeout_us,
            DW3000_HAL_OTP_READY_POLL_DELAY_US
        );
        dw3000_hal_otp_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_otp_program_word(
    dw3000_device_t*                            device,
    dw3000_otp_addr_t                           addr,
    dw3000_otp_word_t                           word,
    const dw3000_hal_otp_program_options_t*     options
) {
    dw3000_error_t                  err;
    dw3000_error_t                  clear_err;
    dw3000_hal_otp_program_options_t default_options;
    dw3000_otp_status_t             status;
    dw3000_otp_word_t               current;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_otp_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (!dw3000_hal_otp_addr_is_valid(addr)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (options == NULL) {
        dw3000_hal_otp_default_program_options(&default_options);
        options = &default_options;
    }

    if (!options->allow_non_customer_addr &&
        !dw3000_hal_otp_addr_is_customer(addr)) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    err = dw3000_hal_otp_read_word(device, addr, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (current == word) {
        return DW3000_ERROR_OK;
    }

    if ((current != 0U) && !options->allow_non_empty_word) {
        return DW3000_ERROR_INVALID_STATE;
    }

    err = dw3000_hal_otp_read_status(device, &status);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (options->require_vpp_ok && ((status & DW3000_OTP_STAT_VPP_OK) == 0U)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_OTP_ADDR, addr);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u32(device, DW3000_REG_OTP_WDATA, word);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_otp_write_cfg_masked(
        device,
        DW3000_HAL_OTP_CFG_MANUAL_ACCESS_MASK,
        (uint16_t)DW3000_OTP_CFG_MAN | (uint16_t)DW3000_OTP_CFG_WRITE
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_otp_wait_program_done(device, options->timeout_us);

    clear_err = dw3000_hal_otp_write_cfg_masked(
        device,
        (uint16_t)DW3000_OTP_CFG_MAN | (uint16_t)DW3000_OTP_CFG_WRITE,
        0U
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return clear_err;
}
