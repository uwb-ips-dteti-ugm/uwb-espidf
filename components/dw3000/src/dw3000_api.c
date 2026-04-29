#include "dw3000_api.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/rx.h"
#include "dw3000_hal/status.h"
#include "dw3000_hal/tx.h"

#define DW3000_API_POLL_DELAY_US 100U

#define DW3000_API_TX_CLEAR_EVENTS \
    (DW3000_TXRX_EVENT_AAT | \
     DW3000_TXRX_EVENT_TXFRB | \
     DW3000_TXRX_EVENT_TXPRS | \
     DW3000_TXRX_EVENT_TXPHS | \
     DW3000_TXRX_EVENT_TXFRS | \
     DW3000_TXRX_EVENT_HPDWARN | \
     DW3000_TXRX_EVENT_CCA_FAIL | \
     DW3000_TXRX_EVENT_CMD_ERR)

#define DW3000_API_SPI_ERROR_EVENTS \
    (DW3000_TXRX_EVENT_SPICRCE | \
     DW3000_TXRX_EVENT_SPI_OVF | \
     DW3000_TXRX_EVENT_SPI_UNF | \
     DW3000_TXRX_EVENT_SPIERR)

static uint32_t dw3000_api_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static void dw3000_api_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static void dw3000_api_tx_result_reset(dw3000_api_tx_result_t* result) {
    if (result != NULL) {
        memset(result, 0, sizeof(*result));
    }
}

static void dw3000_api_rx_result_reset(dw3000_api_rx_result_t* result) {
    if (result != NULL) {
        memset(result, 0, sizeof(*result));
    }
}

static dw3000_error_t dw3000_api_wait_for_status(
    dw3000_device_t*      device,
    dw3000_txrx_event_t   done_events,
    dw3000_txrx_event_t   stop_events,
    uint32_t              timeout_us,
    dw3000_txrx_event_t*  final_status
) {
    dw3000_error_t    err;
    dw3000_txrx_event_t status;
    dw3000_txrx_event_t watched = done_events | stop_events;

    if ((timeout_us != 0U) &&
        (device->port.get_time_us == NULL) &&
        (device->port.delay_us == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (device->port.get_time_us != NULL) {
        uint64_t start_us = device->port.get_time_us(device->port.ctx);

        do {
            err = dw3000_hal_status_read(device, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if ((status & watched) != 0U) {
                if (final_status != NULL) {
                    *final_status = status;
                }
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                if (final_status != NULL) {
                    *final_status = status;
                }
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_api_delay_us(
                    device,
                    dw3000_api_min_u32(remaining_us, DW3000_API_POLL_DELAY_US)
                );
            }
        } while (true);
    }

    do {
        err = dw3000_hal_status_read(device, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if ((status & watched) != 0U) {
            if (final_status != NULL) {
                *final_status = status;
            }
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            if (final_status != NULL) {
                *final_status = status;
            }
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_api_min_u32(timeout_us, DW3000_API_POLL_DELAY_US);
        dw3000_api_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

static dw3000_error_t dw3000_api_cleanup_radio(
    dw3000_device_t* device,
    dw3000_error_t   prior_err
) {
    dw3000_error_t err = dw3000_hal_fcmd_txrxoff(device);

    return (prior_err == DW3000_ERROR_OK) ? err : prior_err;
}

static dw3000_error_t dw3000_api_classify_tx_status(
    dw3000_txrx_event_t status
) {
    if ((status & DW3000_TXRX_EVENT_TXFRS) != 0U) {
        return DW3000_ERROR_OK;
    }

    if ((status & DW3000_TXRX_EVENT_HPDWARN) != 0U) {
        return DW3000_ERROR_TIMEOUT;
    }

    if ((status & DW3000_TXRX_EVENT_CCA_FAIL) != 0U) {
        return DW3000_ERROR_BUSY;
    }

    if ((status & DW3000_TXRX_EVENT_CMD_ERR) != 0U) {
        return DW3000_ERROR_INVALID_STATE;
    }

    return DW3000_ERROR_INVALID_STATE;
}

static dw3000_error_t dw3000_api_classify_rx_status(
    dw3000_txrx_event_t status
) {
    if ((status & DW3000_TXRX_EVENT_RXFCG) != 0U) {
        return DW3000_ERROR_OK;
    }

    if ((status & (DW3000_TXRX_EVENT_RXFTO |
                   DW3000_TXRX_EVENT_RXPTO |
                   DW3000_TXRX_EVENT_RXSTO)) != 0U) {
        return DW3000_ERROR_TIMEOUT;
    }

    if ((status & dw3000_hal_rx_error_events()) != 0U) {
        return DW3000_ERROR_INVALID_STATE;
    }

    return DW3000_ERROR_INVALID_STATE;
}

static dw3000_api_event_flags_t dw3000_api_event_flags(
    dw3000_txrx_event_t status
) {
    dw3000_api_event_flags_t flags = DW3000_API_EVENT_NONE;

    if ((status & DW3000_TXRX_EVENT_TXFRS) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_TX_DONE);
    }

    if ((status & DW3000_TXRX_EVENT_RXFCG) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_RX_DONE);
    }

    if ((status & dw3000_hal_rx_error_events()) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_RX_ERROR);
    }

    if ((status & (DW3000_TXRX_EVENT_RXFTO |
                   DW3000_TXRX_EVENT_RXPTO |
                   DW3000_TXRX_EVENT_RXSTO)) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_RX_TIMEOUT);
    }

    if ((status & DW3000_API_SPI_ERROR_EVENTS) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_SPI_ERROR);
    }

    if ((status & DW3000_TXRX_EVENT_AES_DONE) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_AES_DONE);
    }

    if ((status & DW3000_TXRX_EVENT_AES_ERR) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_AES_ERROR);
    }

    if ((status & (DW3000_TXRX_EVENT_PLL_HILO |
                   DW3000_TXRX_EVENT_VWARN)) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_CLOCK_WARNING);
    }

    if ((status & DW3000_TXRX_EVENT_GPIOIRQ) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_GPIO);
    }

    if ((status & DW3000_TXRX_EVENT_CMD_ERR) != 0U) {
        flags = (dw3000_api_event_flags_t)(flags | DW3000_API_EVENT_COMMAND_ERROR);
    }

    return flags;
}

void dw3000_api_default_config(dw3000_api_config_t* config) {
    if (config == NULL) {
        return;
    }

    dw3000_hal_default_bringup(&config->bringup);
    dw3000_hal_default_init_options(&config->init);
}

void dw3000_api_default_tx_options(dw3000_api_tx_options_t* options) {
    if (options == NULL) {
        return;
    }

    memset(options, 0, sizeof(*options));
    options->command             = DW3000_FCMD_TX;
    options->auto_fcs            = true;
    options->wait_complete       = true;
    options->timeout_us          = DW3000_API_DEFAULT_WAIT_TIMEOUT_US;
    options->clear_status_before = true;
    options->clear_status_after  = true;
    options->read_timestamp      = true;
}

void dw3000_api_default_rx_options(dw3000_api_rx_options_t* options) {
    if (options == NULL) {
        return;
    }

    memset(options, 0, sizeof(*options));
    options->command               = DW3000_FCMD_RX;
    options->wait_complete         = true;
    options->timeout_us            = DW3000_API_DEFAULT_RX_TIMEOUT_US;
    options->clear_status_before   = true;
    options->clear_status_after    = true;
    options->read_timestamp        = true;
    options->discard_fcs           = true;
    options->release_double_buffer = true;
}

dw3000_error_t dw3000_api_init(
    dw3000_device_t*              device,
    const dw3000_port_t*          port,
    const dw3000_device_config_t* device_config,
    const dw3000_api_config_t*    api_config
) {
    dw3000_api_config_t default_config;
    const dw3000_api_config_t* actual_config = api_config;

    if ((device == NULL) || (port == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (actual_config == NULL) {
        dw3000_api_default_config(&default_config);
        actual_config = &default_config;
    }

    return dw3000_hal_initialize(
        device,
        &actual_config->bringup,
        &actual_config->init,
        port,
        device_config
    );
}

dw3000_error_t dw3000_api_deinit(dw3000_device_t* device) {
    dw3000_error_t err = DW3000_ERROR_OK;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((device->port.spi_write != NULL) &&
        ((device->state_flags & DW3000_DEVICE_STATE_PRESENT) != 0U)) {
        err = dw3000_hal_fcmd_txrxoff(device);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    memset(device, 0, sizeof(*device));
    return err;
}

dw3000_error_t dw3000_api_reset_recover(
    dw3000_device_t*           device,
    const dw3000_api_config_t* api_config
) {
    dw3000_port_t          port;
    dw3000_device_config_t device_config;
    dw3000_api_config_t    default_config;
    const dw3000_api_config_t* actual_config = api_config;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->port.spi_read == NULL || device->port.spi_write == NULL) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (actual_config == NULL) {
        dw3000_api_default_config(&default_config);
        actual_config = &default_config;
    }

    port = device->port;
    device_config = device->config;

    return dw3000_hal_initialize(
        device,
        &actual_config->bringup,
        &actual_config->init,
        &port,
        &device_config
    );
}

dw3000_error_t dw3000_api_send_frame(
    dw3000_device_t*               device,
    const void*                    data,
    size_t                         data_len,
    const dw3000_api_tx_options_t* options,
    dw3000_api_tx_result_t*        result
) {
    dw3000_api_tx_options_t actual_options;
    dw3000_txrx_tx_frame_t  frame;
    dw3000_error_t          err;
    dw3000_txrx_event_t     status = 0U;
    size_t                  fcs_len;
    size_t                  frame_len;

    dw3000_api_tx_result_reset(result);

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((data == NULL) && (data_len != 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (options == NULL) {
        dw3000_api_default_tx_options(&actual_options);
    } else {
        actual_options = *options;
    }

    if (!dw3000_hal_tx_is_start_command(actual_options.command)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    fcs_len = actual_options.auto_fcs ? (size_t)DW3000_HAL_TX_AUTO_FCS_LEN : 0U;
    if (data_len > ((size_t)UINT16_MAX - fcs_len)) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    frame_len = data_len + fcs_len;
    frame.tx_flen     = (uint16_t)frame_len;
    frame.ranging     = actual_options.ranging;
    frame.tx_b_offset = actual_options.tx_buffer_offset;
    frame.fine_plen   = actual_options.fine_plen;

    if (actual_options.clear_status_before) {
        err = dw3000_hal_status_clear(device, DW3000_API_TX_CLEAR_EVENTS);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (actual_options.delayed_time_enabled) {
        err = dw3000_hal_tx_set_delayed_time(device, actual_options.delayed_time);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (actual_options.reference_time_enabled) {
        err = dw3000_hal_tx_set_reference_time(device, actual_options.reference_time);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    err = dw3000_hal_tx_prepare_frame(device, data, data_len, &frame);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_tx_start(device, actual_options.command);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (!actual_options.wait_complete) {
        return DW3000_ERROR_OK;
    }

    err = dw3000_api_wait_for_status(
        device,
        DW3000_TXRX_EVENT_TXFRS,
        DW3000_TXRX_EVENT_HPDWARN |
            DW3000_TXRX_EVENT_CCA_FAIL |
            DW3000_TXRX_EVENT_CMD_ERR,
        actual_options.timeout_us,
        &status
    );

    if (result != NULL) {
        result->status = status;
    }

    if (err == DW3000_ERROR_OK) {
        err = dw3000_api_classify_tx_status(status);
    }

    if ((err == DW3000_ERROR_OK) && actual_options.read_timestamp &&
        (result != NULL)) {
        err = dw3000_hal_tx_read_timestamp(device, &result->timestamp);
        if (err == DW3000_ERROR_OK) {
            result->timestamp_valid = true;
        }
    }

    err = dw3000_api_cleanup_radio(device, err);

    if (actual_options.clear_status_after && (status != 0U)) {
        dw3000_error_t clear_err = dw3000_hal_status_clear(
            device,
            status & DW3000_API_TX_CLEAR_EVENTS
        );
        if (err == DW3000_ERROR_OK) {
            err = clear_err;
        }
    }

    return err;
}

dw3000_error_t dw3000_api_receive_frame(
    dw3000_device_t*               device,
    void*                          buffer,
    size_t                         buffer_len,
    const dw3000_api_rx_options_t* options,
    dw3000_api_rx_result_t*        result
) {
    dw3000_api_rx_options_t actual_options;
    dw3000_error_t          err;
    dw3000_txrx_event_t     status = 0U;
    dw3000_txrx_event_t     stop_events;
    size_t                  payload_len = 0U;

    dw3000_api_rx_result_reset(result);

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((buffer == NULL) && (buffer_len != 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (options == NULL) {
        dw3000_api_default_rx_options(&actual_options);
    } else {
        actual_options = *options;
    }

    if (!dw3000_hal_rx_is_start_command(actual_options.command)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (actual_options.clear_status_before) {
        err = dw3000_hal_status_clear(device, dw3000_hal_rx_all_events());
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (actual_options.frame_wait_timeout_enabled) {
        dw3000_txrx_sys_cfg_flags_t flags = DW3000_TXRX_SYS_CFG_RXWTOE;

        if (!device->config.use_double_buffer) {
            flags = (dw3000_txrx_sys_cfg_flags_t)(
                flags | DW3000_TXRX_SYS_CFG_DIS_DRXB
            );
        }

        err = dw3000_hal_rx_configure_sys_cfg(device, flags);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        err = dw3000_hal_rx_set_frame_wait_timeout(
            device,
            actual_options.frame_wait_timeout
        );
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (actual_options.delayed_time_enabled) {
        err = dw3000_hal_rx_set_delayed_time(device, actual_options.delayed_time);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if (actual_options.reference_time_enabled) {
        err = dw3000_hal_rx_set_reference_time(device, actual_options.reference_time);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    err = dw3000_hal_rx_start(device, actual_options.command);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (!actual_options.wait_complete) {
        return DW3000_ERROR_OK;
    }

    stop_events = dw3000_hal_rx_error_events();
    err = dw3000_api_wait_for_status(
        device,
        DW3000_TXRX_EVENT_RXFCG,
        stop_events,
        actual_options.timeout_us,
        &status
    );

    if (result != NULL) {
        result->status     = status;
        result->rx_error   = (status & dw3000_hal_rx_error_events()) != 0U;
        result->rx_timeout = (status & (DW3000_TXRX_EVENT_RXFTO |
                                        DW3000_TXRX_EVENT_RXPTO |
                                        DW3000_TXRX_EVENT_RXSTO)) != 0U;
    }

    if (err == DW3000_ERROR_OK) {
        err = dw3000_api_classify_rx_status(status);
    }

    if (err == DW3000_ERROR_OK) {
        dw3000_txrx_rx_finfo_t finfo;

        err = dw3000_hal_rx_read_finfo(device, &finfo);
        if (err == DW3000_ERROR_OK) {
            payload_len = finfo.rx_flen;
            if (actual_options.discard_fcs) {
                if (payload_len < DW3000_HAL_TX_AUTO_FCS_LEN) {
                    err = DW3000_ERROR_INVALID_SIZE;
                } else {
                    payload_len -= DW3000_HAL_TX_AUTO_FCS_LEN;
                }
            }
        }

        if ((err == DW3000_ERROR_OK) && (payload_len > buffer_len)) {
            err = DW3000_ERROR_INVALID_SIZE;
        }

        if ((err == DW3000_ERROR_OK) && (payload_len != 0U)) {
            err = dw3000_hal_rx_read_active_buffer(
                device,
                0U,
                buffer,
                payload_len
            );
        }

        if ((err == DW3000_ERROR_OK) && actual_options.read_timestamp &&
            (result != NULL)) {
            err = dw3000_hal_rx_read_timestamp(device, &result->timestamp);
            if (err == DW3000_ERROR_OK) {
                result->timestamp_valid = true;
            }
        }

        if ((err == DW3000_ERROR_OK) && (result != NULL)) {
            result->finfo       = finfo;
            result->frame_len   = finfo.rx_flen;
            result->payload_len = payload_len;
            result->frame_valid = true;
        }
    }

    if ((err == DW3000_ERROR_OK) && actual_options.release_double_buffer &&
        device->config.use_double_buffer) {
        err = dw3000_hal_fcmd_db_toggle(device);
    }

    err = dw3000_api_cleanup_radio(device, err);

    if (actual_options.clear_status_after && (status != 0U)) {
        dw3000_error_t clear_err = dw3000_hal_status_clear(
            device,
            status & dw3000_hal_rx_all_events()
        );
        if (err == DW3000_ERROR_OK) {
            err = clear_err;
        }
    }

    return err;
}

dw3000_error_t dw3000_api_handle_events(
    dw3000_device_t*      device,
    bool                  clear,
    dw3000_api_events_t*  events
) {
    dw3000_error_t    err;
    dw3000_txrx_event_t status;

    if ((device == NULL) || (events == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    memset(events, 0, sizeof(*events));

    err = dw3000_hal_status_read(device, &status);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    events->status = status;
    events->flags  = dw3000_api_event_flags(status);

    if (clear && (status != 0U)) {
        err = dw3000_hal_status_clear(device, status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return DW3000_ERROR_OK;
}
