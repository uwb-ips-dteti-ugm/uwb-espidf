#include "dw3000_hal/pmsc.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_PMSC_READY_POLL_DELAY_US 100U

#define DW3000_HAL_PMSC_CLK_SRC_MASK    0x3UL
#define DW3000_HAL_PMSC_CLK_SYS_SHIFT   0U
#define DW3000_HAL_PMSC_CLK_RX_SHIFT    2U
#define DW3000_HAL_PMSC_CLK_TX_SHIFT    4U
#define DW3000_HAL_PMSC_CLK_SELECT_MASK 0x0000003FUL
#define DW3000_HAL_PMSC_CLK_FLAGS_MASK        \
    ((uint32_t)DW3000_PMSC_CLK_ACC_CLK_EN |   \
     (uint32_t)DW3000_PMSC_CLK_CIA_CLK_EN |   \
     (uint32_t)DW3000_PMSC_CLK_SAR_CLK_EN |   \
     (uint32_t)DW3000_PMSC_CLK_ACC_MCLK_EN |  \
     (uint32_t)DW3000_PMSC_CLK_GPIO_CLK_EN |  \
     (uint32_t)DW3000_PMSC_CLK_GPIO_DCLK_EN | \
     (uint32_t)DW3000_PMSC_CLK_GPIO_DRST_N |  \
     (uint32_t)DW3000_PMSC_CLK_LP_CLK_EN)
#define DW3000_HAL_PMSC_CLK_MASK \
    (DW3000_HAL_PMSC_CLK_SELECT_MASK | DW3000_HAL_PMSC_CLK_FLAGS_MASK)
#define DW3000_HAL_PMSC_CLK_SYS_MASK \
    (DW3000_HAL_PMSC_CLK_SRC_MASK << DW3000_HAL_PMSC_CLK_SYS_SHIFT)

#define DW3000_HAL_PMSC_SEQ_CONFIG_MASK     \
    ((uint32_t)DW3000_PMSC_SEQ_AINIT2IDLE | \
     (uint32_t)DW3000_PMSC_SEQ_ATX2SLP |    \
     (uint32_t)DW3000_PMSC_SEQ_ARX2SLP |    \
     (uint32_t)DW3000_PMSC_SEQ_PLL_SYNC |   \
     (uint32_t)DW3000_PMSC_SEQ_CIARUNE)
#define DW3000_HAL_PMSC_SEQ_PULSE_MASK ((uint32_t)DW3000_PMSC_SEQ_FORCE2INIT)
#define DW3000_HAL_PMSC_SEQ_MASK \
    (DW3000_HAL_PMSC_SEQ_CONFIG_MASK | DW3000_HAL_PMSC_SEQ_PULSE_MASK)

#define DW3000_HAL_PMSC_LED_BLINK_TIM_MASK 0x000000FFUL
#define DW3000_HAL_PMSC_LED_FLAGS_MASK        \
    ((uint32_t)DW3000_PMSC_LED_BLNKEN |       \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_RXOK | \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_SFD |  \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_RX |   \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_TX)
#define DW3000_HAL_PMSC_LED_CONFIG_MASK \
    (DW3000_HAL_PMSC_LED_BLINK_TIM_MASK | (uint32_t)DW3000_PMSC_LED_BLNKEN)
#define DW3000_HAL_PMSC_LED_FORCE_MASK        \
    ((uint32_t)DW3000_PMSC_LED_BLNKNOW_RXOK | \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_SFD |  \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_RX |   \
     (uint32_t)DW3000_PMSC_LED_BLNKNOW_TX)
#define DW3000_HAL_PMSC_LED_CONFIG_FLAGS_MASK ((uint32_t)DW3000_PMSC_LED_BLNKEN)

static bool dw3000_hal_pmsc_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_pmsc_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static uint32_t dw3000_hal_pmsc_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static void dw3000_hal_pmsc_mark_idle_rc(dw3000_device_t* device) {
    device->state_flags = (dw3000_device_state_flags_t)((device->state_flags | DW3000_DEVICE_STATE_IDLE_RC) &
                                                        ~(DW3000_DEVICE_STATE_IDLE_PLL |
                                                          DW3000_DEVICE_STATE_RX_ON |
                                                          DW3000_DEVICE_STATE_TX_PENDING |
                                                          DW3000_DEVICE_STATE_SLEEPING));
}

static void dw3000_hal_pmsc_mark_idle_pll(dw3000_device_t* device) {
    device->state_flags = (dw3000_device_state_flags_t)((device->state_flags | DW3000_DEVICE_STATE_IDLE_PLL) &
                                                        ~(DW3000_DEVICE_STATE_IDLE_RC |
                                                          DW3000_DEVICE_STATE_RX_ON |
                                                          DW3000_DEVICE_STATE_TX_PENDING |
                                                          DW3000_DEVICE_STATE_SLEEPING));
}

static bool dw3000_hal_pmsc_is_valid_clk_src(dw3000_pmsc_clk_src_t src) {
    return (uint32_t)src <= DW3000_HAL_PMSC_CLK_SRC_MASK;
}

static uint32_t dw3000_hal_pmsc_build_clock_ctrl(
    const dw3000_pmsc_clk_ctrl_t* clock
) {
    return (((uint32_t)clock->sys_clk & DW3000_HAL_PMSC_CLK_SRC_MASK)
            << DW3000_HAL_PMSC_CLK_SYS_SHIFT) |
           (((uint32_t)clock->rx_clk & DW3000_HAL_PMSC_CLK_SRC_MASK)
            << DW3000_HAL_PMSC_CLK_RX_SHIFT) |
           (((uint32_t)clock->tx_clk & DW3000_HAL_PMSC_CLK_SRC_MASK)
            << DW3000_HAL_PMSC_CLK_TX_SHIFT) |
           ((uint32_t)clock->flags & DW3000_HAL_PMSC_CLK_FLAGS_MASK);
}

static void dw3000_hal_pmsc_decode_clock_ctrl(
    uint32_t                raw,
    dw3000_pmsc_clk_ctrl_t* clock
) {
    clock->sys_clk = (dw3000_pmsc_clk_src_t)((raw >> DW3000_HAL_PMSC_CLK_SYS_SHIFT) & DW3000_HAL_PMSC_CLK_SRC_MASK);
    clock->rx_clk  = (dw3000_pmsc_clk_src_t)((raw >> DW3000_HAL_PMSC_CLK_RX_SHIFT) & DW3000_HAL_PMSC_CLK_SRC_MASK);
    clock->tx_clk  = (dw3000_pmsc_clk_src_t)((raw >> DW3000_HAL_PMSC_CLK_TX_SHIFT) & DW3000_HAL_PMSC_CLK_SRC_MASK);
    clock->flags   = (dw3000_pmsc_clk_flags_t)(raw & DW3000_HAL_PMSC_CLK_FLAGS_MASK);
}

static dw3000_error_t dw3000_hal_pmsc_write_sys_clock(
    dw3000_device_t*      device,
    dw3000_pmsc_clk_src_t sys_clk
) {
    return dw3000_reg_modify_u32(
        device,
        DW3000_REG_CLK_CTRL,
        DW3000_HAL_PMSC_CLK_SYS_MASK,
        ((uint32_t)sys_clk & DW3000_HAL_PMSC_CLK_SRC_MASK) << DW3000_HAL_PMSC_CLK_SYS_SHIFT
    );
}

static dw3000_error_t dw3000_hal_pmsc_set_seq_flags_raw(
    dw3000_device_t*             device,
    dw3000_pmsc_seq_ctrl_flags_t flags
) {
    return dw3000_reg_modify_u32(
        device,
        DW3000_REG_SEQ_CTRL,
        (uint32_t)flags & DW3000_HAL_PMSC_SEQ_MASK,
        (uint32_t)flags & DW3000_HAL_PMSC_SEQ_MASK
    );
}

static dw3000_error_t dw3000_hal_pmsc_clear_seq_flags_raw(
    dw3000_device_t*             device,
    dw3000_pmsc_seq_ctrl_flags_t flags
) {
    return dw3000_reg_modify_u32(
        device,
        DW3000_REG_SEQ_CTRL,
        (uint32_t)flags & DW3000_HAL_PMSC_SEQ_MASK,
        0U
    );
}

dw3000_error_t dw3000_hal_pmsc_validate_clock_ctrl(
    const dw3000_pmsc_clk_ctrl_t* clock
) {
    if (clock == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_valid_clk_src(clock->sys_clk) ||
        !dw3000_hal_pmsc_is_valid_clk_src(clock->rx_clk) ||
        !dw3000_hal_pmsc_is_valid_clk_src(clock->tx_clk)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)clock->flags & ~DW3000_HAL_PMSC_CLK_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_validate_seq_flags(
    dw3000_pmsc_seq_ctrl_flags_t flags
) {
    if (((uint32_t)flags & ~DW3000_HAL_PMSC_SEQ_CONFIG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_validate_config(
    const dw3000_pmsc_config_t* config
) {
    dw3000_error_t err;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_pmsc_validate_clock_ctrl(&config->clk_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_validate_seq_flags(config->seq_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (((uint32_t)config->led_ctrl.flags & ~DW3000_HAL_PMSC_LED_CONFIG_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)config->bias_ctrl & ~DW3000_PMSC_BIAS_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_read_clock_ctrl(
    dw3000_device_t*        device,
    dw3000_pmsc_clk_ctrl_t* clock
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (clock == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CLK_CTRL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_pmsc_decode_clock_ctrl(raw, clock);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_configure_clock_ctrl(
    dw3000_device_t*              device,
    const dw3000_pmsc_clk_ctrl_t* clock
) {
    dw3000_error_t err;
    uint32_t       current;
    uint32_t       value;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (clock == NULL) {
        clock = &device->config.pmsc.clk_ctrl;
    }

    err = dw3000_hal_pmsc_validate_clock_ctrl(clock);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CLK_CTRL, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    value = (current & ~DW3000_HAL_PMSC_CLK_MASK) |
            dw3000_hal_pmsc_build_clock_ctrl(clock);

    err = dw3000_reg_write_u32(device, DW3000_REG_CLK_CTRL, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.clk_ctrl = *clock;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_set_clock_flags(
    dw3000_device_t*        device,
    dw3000_pmsc_clk_flags_t flags
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)flags & ~DW3000_HAL_PMSC_CLK_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    dw3000_error_t err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CLK_CTRL,
        (uint32_t)flags,
        (uint32_t)flags
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.clk_ctrl.flags = (dw3000_pmsc_clk_flags_t)(device->config.pmsc.clk_ctrl.flags | flags);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_clear_clock_flags(
    dw3000_device_t*        device,
    dw3000_pmsc_clk_flags_t flags
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)flags & ~DW3000_HAL_PMSC_CLK_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    dw3000_error_t err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CLK_CTRL,
        (uint32_t)flags,
        0U
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.clk_ctrl.flags = (dw3000_pmsc_clk_flags_t)(device->config.pmsc.clk_ctrl.flags & ~flags);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_read_seq_ctrl(
    dw3000_device_t*              device,
    dw3000_pmsc_seq_ctrl_flags_t* flags
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (flags == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_SEQ_CTRL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *flags = (dw3000_pmsc_seq_ctrl_flags_t)(raw & DW3000_HAL_PMSC_SEQ_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_configure_seq_ctrl(
    dw3000_device_t*             device,
    dw3000_pmsc_seq_ctrl_flags_t flags
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_pmsc_validate_seq_flags(flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_SEQ_CTRL,
        DW3000_HAL_PMSC_SEQ_CONFIG_MASK,
        (uint32_t)flags
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.seq_flags = flags;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_set_seq_flags(
    dw3000_device_t*             device,
    dw3000_pmsc_seq_ctrl_flags_t flags
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (((uint32_t)flags & ~DW3000_HAL_PMSC_SEQ_CONFIG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_pmsc_set_seq_flags_raw(device, flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.seq_flags = (dw3000_pmsc_seq_ctrl_flags_t)(device->config.pmsc.seq_flags | flags);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_clear_seq_flags(
    dw3000_device_t*             device,
    dw3000_pmsc_seq_ctrl_flags_t flags
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (((uint32_t)flags & ~DW3000_HAL_PMSC_SEQ_CONFIG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_pmsc_clear_seq_flags_raw(device, flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.seq_flags = (dw3000_pmsc_seq_ctrl_flags_t)(device->config.pmsc.seq_flags & ~flags);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_wait_for_pll_lock(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t err;
    uint32_t       status;

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
            err = dw3000_reg_read_u32(device, DW3000_REG_SYS_STATUS, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if ((status & (uint32_t)DW3000_TXRX_EVENT_CPLOCK) != 0U) {
                err = dw3000_reg_write_u32(
                    device,
                    DW3000_REG_SYS_STATUS,
                    (uint32_t)DW3000_TXRX_EVENT_CPLOCK
                );
                if (err != DW3000_ERROR_OK) {
                    return err;
                }

                dw3000_hal_pmsc_mark_idle_pll(device);
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_pmsc_delay_us(
                    device,
                    dw3000_hal_pmsc_min_u32(
                        remaining_us,
                        DW3000_HAL_PMSC_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_STATUS, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status & (uint32_t)DW3000_TXRX_EVENT_CPLOCK) != 0U) {
            err = dw3000_reg_write_u32(
                device,
                DW3000_REG_SYS_STATUS,
                (uint32_t)DW3000_TXRX_EVENT_CPLOCK
            );
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            dw3000_hal_pmsc_mark_idle_pll(device);
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_pmsc_min_u32(
            timeout_us,
            DW3000_HAL_PMSC_READY_POLL_DELAY_US
        );
        dw3000_hal_pmsc_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_pmsc_enter_idle_pll(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_pmsc_set_seq_flags(device, DW3000_PMSC_SEQ_AINIT2IDLE);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_pmsc_wait_for_pll_lock(device, timeout_us);
}

dw3000_error_t dw3000_hal_pmsc_force_idle_rc(dw3000_device_t* device) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_pmsc_write_sys_clock(device, DW3000_PMSC_CLK_SRC_FAST_RC);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_clear_seq_flags(device, DW3000_PMSC_SEQ_AINIT2IDLE);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_set_seq_flags_raw(device, DW3000_PMSC_SEQ_FORCE2INIT);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_clear_seq_flags_raw(device, DW3000_PMSC_SEQ_FORCE2INIT);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_write_sys_clock(device, DW3000_PMSC_CLK_SRC_AUTO);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.clk_ctrl.sys_clk = DW3000_PMSC_CLK_SRC_AUTO;
    dw3000_hal_pmsc_mark_idle_rc(device);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_soft_reset_blocks(
    dw3000_device_t*       device,
    dw3000_pmsc_soft_rst_t mask
) {
    dw3000_error_t err;
    uint32_t       clock_raw;
    uint16_t       current;
    uint16_t       reset_mask = (uint16_t)mask;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if ((reset_mask & ~DW3000_PMSC_SOFT_RST_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (reset_mask == 0U) {
        return DW3000_ERROR_OK;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CLK_CTRL, &clock_raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_write_sys_clock(device, DW3000_PMSC_CLK_SRC_FAST_RC_DIV4);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_SOFT_RST, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    current = (uint16_t)(current & DW3000_PMSC_SOFT_RST_MASK);
    err     = dw3000_reg_write_u16(
        device,
        DW3000_REG_SOFT_RST,
        (uint16_t)(current & ~reset_mask)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_SOFT_RST,
        (uint16_t)(current | reset_mask)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CLK_CTRL,
        DW3000_HAL_PMSC_CLK_SYS_MASK,
        clock_raw & DW3000_HAL_PMSC_CLK_SYS_MASK
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->sys_cfg_cache_valid  = false;
    device->enabled_events_valid = false;
    dw3000_hal_pmsc_mark_idle_rc(device);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_read_txfseq(
    dw3000_device_t*      device,
    dw3000_pmsc_txfseq_t* value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_TXFSEQ, value);
}

dw3000_error_t dw3000_hal_pmsc_set_txfseq(
    dw3000_device_t*     device,
    dw3000_pmsc_txfseq_t value
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write_u32(device, DW3000_REG_TXFSEQ, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.txfseq = value;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_read_led_ctrl(
    dw3000_device_t*        device,
    dw3000_pmsc_led_ctrl_t* led
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (led == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_LED_CTRL, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    led->blink_tim = (uint8_t)(raw & DW3000_HAL_PMSC_LED_BLINK_TIM_MASK);
    led->flags     = (dw3000_pmsc_led_flags_t)(raw & DW3000_HAL_PMSC_LED_FLAGS_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_configure_led_ctrl(
    dw3000_device_t*              device,
    const dw3000_pmsc_led_ctrl_t* led
) {
    dw3000_error_t err;
    uint32_t       value;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (led == NULL) {
        led = &device->config.pmsc.led_ctrl;
    }

    if (((uint32_t)led->flags & ~DW3000_HAL_PMSC_LED_CONFIG_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    value = ((uint32_t)led->blink_tim & DW3000_HAL_PMSC_LED_BLINK_TIM_MASK) |
            ((uint32_t)led->flags & DW3000_HAL_PMSC_LED_CONFIG_MASK);

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_LED_CTRL,
        DW3000_HAL_PMSC_LED_CONFIG_MASK,
        value
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.led_ctrl = *led;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_trigger_leds(
    dw3000_device_t*        device,
    dw3000_pmsc_led_flags_t leds
) {
    uint32_t value = (uint32_t)leds;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((value & ~DW3000_HAL_PMSC_LED_FORCE_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (value == 0U) {
        return DW3000_ERROR_OK;
    }

    return dw3000_reg_modify_u32(
        device,
        DW3000_REG_LED_CTRL,
        DW3000_HAL_PMSC_LED_FORCE_MASK,
        value
    );
}

dw3000_error_t dw3000_hal_pmsc_read_bias_ctrl(
    dw3000_device_t*         device,
    dw3000_pmsc_bias_ctrl_t* value
) {
    if ((device == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u16(device, DW3000_REG_BIAS_CTRL, value);
}

dw3000_error_t dw3000_hal_pmsc_set_bias_ctrl(
    dw3000_device_t*        device,
    dw3000_pmsc_bias_ctrl_t value
) {
    dw3000_error_t err;
    uint16_t       current;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)value & ~DW3000_PMSC_BIAS_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_BIAS_CTRL, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    current = (uint16_t)((current & ~DW3000_PMSC_BIAS_CTRL_MASK) |
                         (value & DW3000_PMSC_BIAS_CTRL_MASK));

    err = dw3000_reg_write_u16(device, DW3000_REG_BIAS_CTRL, current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.bias_ctrl = value;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_configure(
    dw3000_device_t*            device,
    const dw3000_pmsc_config_t* config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_pmsc_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (config == NULL) {
        config = &device->config.pmsc;
    }

    err = dw3000_hal_pmsc_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_configure_clock_ctrl(device, &config->clk_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_configure_seq_ctrl(device, config->seq_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_set_txfseq(device, config->txfseq);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_pmsc_configure_led_ctrl(device, &config->led_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (config->bias_ctrl != 0U) {
        err = dw3000_hal_pmsc_set_bias_ctrl(device, config->bias_ctrl);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    device->config.pmsc = *config;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_pmsc_configure_current(dw3000_device_t* device) {
    return dw3000_hal_pmsc_configure(device, NULL);
}
