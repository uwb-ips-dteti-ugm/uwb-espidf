#include "dw3000_hal/core.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dw3000_register.h"

#define DW3000_HAL_DEVICE_ID_REV_PRODUCTION 0x02U
#define DW3000_HAL_DEVICE_ID_VER_NON_PDOA   0x00U
#define DW3000_HAL_DEVICE_ID_VER_PDOA       0x01U

#define DW3000_HAL_READY_POLL_DELAY_US 100U

#define DW3000_HAL_DGC_THR_64_SHIFT 9U
#define DW3000_HAL_DGC_THR_64_OPT   0x32U

#define DW3000_HAL_DEFAULT_STS_CPS_LEN 7U

static uint32_t dw3000_hal_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static void dw3000_hal_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static void dw3000_hal_clear_runtime_state(dw3000_device_t* device) {
    device->id           = (dw3000_device_id_t){0};
    device->capabilities = DW3000_DEVICE_CAP_NONE;
    device->state_flags  = (dw3000_device_state_flags_t)(
        device->state_flags & DW3000_DEVICE_STATE_PORT_READY
    );

    device->sys_cfg_cache          = 0U;
    device->sys_cfg_cache_valid    = false;
    device->enabled_events         = 0U;
    device->enabled_events_valid   = false;
    device->active_rx_buffer       = 0U;
}

static void dw3000_hal_mark_idle_rc(dw3000_device_t* device) {
    device->state_flags = (dw3000_device_state_flags_t)(
        (device->state_flags |
         DW3000_DEVICE_STATE_IDLE_RC) &
        ~(DW3000_DEVICE_STATE_IDLE_PLL |
          DW3000_DEVICE_STATE_RX_ON |
          DW3000_DEVICE_STATE_TX_PENDING |
          DW3000_DEVICE_STATE_SLEEPING)
    );
}

static dw3000_error_t dw3000_hal_clear_spi_ready(dw3000_device_t* device) {
    return dw3000_reg_write_u32(
        device,
        DW3000_REG_SYS_STATUS,
        (uint32_t)DW3000_TXRX_EVENT_SPIRDY
    );
}

void dw3000_hal_default_bringup(dw3000_hal_bringup_t* bringup) {
    if (bringup == NULL) {
        return;
    }

    bringup->reset_assert_us = DW3000_HAL_DEFAULT_RESET_ASSERT_US;
    bringup->reset_settle_us = DW3000_HAL_DEFAULT_RESET_SETTLE_US;
    bringup->ready_timeout_us = DW3000_HAL_DEFAULT_READY_TIMEOUT_US;
}

void dw3000_hal_default_config(dw3000_device_config_t* config) {
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));

    config->phy.channel          = DW3000_PHY_CHANNEL_5;
    config->phy.prf              = DW3000_PHY_PRF_64_MHZ;
    config->phy.data_rate        = DW3000_PHY_DATA_RATE_6M81;
    config->phy.preamble_length  = DW3000_PHY_PREAMBLE_LEN_64;
    config->phy.sfd_type         = DW3000_PHY_SFD_TYPE_IEEE_802154Z;
    config->phy.pac_size         = DW3000_PHY_PAC_SIZE_8;
    config->phy.phr_mode         = DW3000_PHY_PHR_MODE_STANDARD;
    config->phy.phr_rate         = DW3000_PHY_PHR_RATE_850K;
    config->phy.tx_preamble_code = 9U;
    config->phy.rx_preamble_code = 9U;

    config->mac.panadr.pan_id     = 0xFFFFU;
    config->mac.panadr.short_addr = 0xFFFFU;

    config->sts.cps_len       = DW3000_HAL_DEFAULT_STS_CPS_LEN;
    config->sts.iv.bytes[0]   = 1U;
    config->sts.packet_cfg    = DW3000_STS_PACKET_CFG_SP0;
    config->sts.pdoa_mode     = DW3000_STS_PDOA_MODE_DISABLED;
    config->sts.sys_cfg_flags = DW3000_STS_SYS_CFG_CIA_STS;

    config->rx_tune.sfd_toc = 65U;
    config->rx_tune.pre_toc = 0U;
    config->rx_tune.dtune3  = 0xAF5F35CCUL;
    config->rx_tune.dgc_cfg = (dw3000_rx_tune_dgc_cfg_t)(
        DW3000_RX_TUNE_DGC_RX_TUNE_EN |
        (DW3000_HAL_DGC_THR_64_OPT << DW3000_HAL_DGC_THR_64_SHIFT)
    );

    config->use_double_buffer     = true;
    config->load_otp_calibration  = true;
    config->auto_init_pll         = true;
}

dw3000_error_t dw3000_hal_init_context(
    dw3000_device_t*              device,
    const dw3000_port_t*          port,
    const dw3000_device_config_t* config
) {
    dw3000_device_config_t default_config;

    if ((device == NULL) || (port == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((port->spi_read == NULL) || (port->spi_write == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (config == NULL) {
        dw3000_hal_default_config(&default_config);
        config = &default_config;
    }

    memset(device, 0, sizeof(*device));
    device->port        = *port;
    device->config      = *config;
    device->state_flags = DW3000_DEVICE_STATE_PORT_READY;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_hard_reset(
    dw3000_device_t* device,
    uint32_t         assert_us,
    uint32_t         settle_us
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->port.reset == NULL) {
        return DW3000_ERROR_OK;
    }

    if (((assert_us != 0U) || (settle_us != 0U)) &&
        (device->port.delay_us == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    device->port.reset(device->port.ctx, true);
    dw3000_hal_delay_us(device, assert_us);
    device->port.reset(device->port.ctx, false);
    dw3000_hal_delay_us(device, settle_us);

    dw3000_hal_clear_runtime_state(device);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_read_device_id(
    dw3000_device_t*    device,
    dw3000_device_id_t* id
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (id == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_DEV_ID, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    id->rev    = (uint8_t)(raw & 0x0FU);
    id->ver    = (uint8_t)((raw >> 4U) & 0x0FU);
    id->model  = (uint8_t)((raw >> 8U) & 0xFFU);
    id->ridtag = (uint16_t)((raw >> 16U) & 0xFFFFU);

    return DW3000_ERROR_OK;
}

bool dw3000_hal_is_supported_device_id(const dw3000_device_id_t* id) {
    if (id == NULL) {
        return false;
    }

    if ((id->ridtag != DW3000_DEVICE_RIDTAG_DECAWAVE) ||
        (id->model != DW3000_DEVICE_MODEL_DW3000) ||
        (id->rev != DW3000_HAL_DEVICE_ID_REV_PRODUCTION)) {
        return false;
    }

    return (id->ver == DW3000_HAL_DEVICE_ID_VER_NON_PDOA) ||
           (id->ver == DW3000_HAL_DEVICE_ID_VER_PDOA);
}

dw3000_device_capabilities_t dw3000_hal_capabilities_from_device_id(
    const dw3000_device_id_t* id
) {
    if (!dw3000_hal_is_supported_device_id(id)) {
        return DW3000_DEVICE_CAP_NONE;
    }

    if (id->ver == DW3000_HAL_DEVICE_ID_VER_PDOA) {
        return DW3000_DEVICE_CAP_PDOA;
    }

    return DW3000_DEVICE_CAP_NONE;
}

dw3000_error_t dw3000_hal_probe(dw3000_device_t* device) {
    dw3000_error_t     err;
    dw3000_device_id_t id;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_read_device_id(device, &id);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (!dw3000_hal_is_supported_device_id(&id)) {
        device->id = (dw3000_device_id_t){0};
        device->state_flags = (dw3000_device_state_flags_t)(
            device->state_flags &
            ~(DW3000_DEVICE_STATE_PRESENT | DW3000_DEVICE_STATE_INITIALIZED)
        );
        device->capabilities = DW3000_DEVICE_CAP_NONE;
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    device->id           = id;
    device->capabilities = dw3000_hal_capabilities_from_device_id(&id);
    device->state_flags  = (dw3000_device_state_flags_t)(
        device->state_flags | DW3000_DEVICE_STATE_PRESENT
    );

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_wait_for_spi_ready(
    dw3000_device_t* device,
    uint32_t         timeout_us
) {
    dw3000_error_t err;
    uint32_t       status;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->port.spi_read == NULL) {
        return DW3000_ERROR_INVALID_STATE;
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

            if ((status & (uint32_t)DW3000_TXRX_EVENT_SPIRDY) != 0U) {
                err = dw3000_hal_clear_spi_ready(device);
                if (err != DW3000_ERROR_OK) {
                    return err;
                }

                dw3000_hal_mark_idle_rc(device);
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_delay_us(
                    device,
                    dw3000_hal_min_u32(remaining_us, DW3000_HAL_READY_POLL_DELAY_US)
                );
            }
        } while (true);
    }

    do {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_STATUS, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status & (uint32_t)DW3000_TXRX_EVENT_SPIRDY) != 0U) {
            err = dw3000_hal_clear_spi_ready(device);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            dw3000_hal_mark_idle_rc(device);
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_min_u32(timeout_us, DW3000_HAL_READY_POLL_DELAY_US);
        dw3000_hal_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_bringup(
    dw3000_device_t*              device,
    const dw3000_hal_bringup_t*   bringup,
    const dw3000_port_t*          port,
    const dw3000_device_config_t* config
) {
    dw3000_error_t        err;
    dw3000_hal_bringup_t  default_bringup;
    const dw3000_hal_bringup_t* actual_bringup = bringup;

    if (actual_bringup == NULL) {
        dw3000_hal_default_bringup(&default_bringup);
        actual_bringup = &default_bringup;
    }

    err = dw3000_hal_init_context(device, port, config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_hard_reset(
        device,
        actual_bringup->reset_assert_us,
        actual_bringup->reset_settle_us
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_wait_for_spi_ready(device, actual_bringup->ready_timeout_us);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_probe(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->state_flags = (dw3000_device_state_flags_t)(
        device->state_flags | DW3000_DEVICE_STATE_INITIALIZED
    );

    return DW3000_ERROR_OK;
}
