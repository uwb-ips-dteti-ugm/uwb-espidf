#ifndef DW3000_API_H
#define DW3000_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_types/fcmd.h"
#include "dw3000_types/txrx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_API_DEFAULT_WAIT_TIMEOUT_US 10000U
#define DW3000_API_DEFAULT_RX_TIMEOUT_US   100000U

typedef struct {
    dw3000_hal_bringup_t      bringup;
    dw3000_hal_init_options_t init;
} dw3000_api_config_t;

typedef struct {
    dw3000_fcmd_t              command;
    uint16_t                   tx_buffer_offset;
    bool                       ranging;
    uint8_t                    fine_plen;
    bool                       auto_fcs;
    bool                       wait_complete;
    uint32_t                   timeout_us;
    bool                       clear_status_before;
    bool                       clear_status_after;
    bool                       read_timestamp;
    bool                       delayed_time_enabled;
    dw3000_txrx_delayed_time_t delayed_time;
    bool                       reference_time_enabled;
    dw3000_txrx_delayed_time_t reference_time;
} dw3000_api_tx_options_t;

typedef struct {
    dw3000_txrx_event_t     status;
    dw3000_txrx_timestamp_t timestamp;
    bool                    timestamp_valid;
} dw3000_api_tx_result_t;

typedef struct {
    dw3000_fcmd_t              command;
    bool                       wait_complete;
    uint32_t                   timeout_us;
    bool                       clear_status_before;
    bool                       clear_status_after;
    bool                       read_timestamp;
    bool                       discard_fcs;
    bool                       release_double_buffer;
    bool                       frame_wait_timeout_enabled;
    dw3000_txrx_fwto_t         frame_wait_timeout;
    bool                       delayed_time_enabled;
    dw3000_txrx_delayed_time_t delayed_time;
    bool                       reference_time_enabled;
    dw3000_txrx_delayed_time_t reference_time;
} dw3000_api_rx_options_t;

typedef struct {
    dw3000_txrx_event_t      status;
    dw3000_txrx_rx_finfo_t   finfo;
    size_t                   frame_len;
    size_t                   payload_len;
    dw3000_txrx_timestamp_t  timestamp;
    bool                     timestamp_valid;
    bool                     frame_valid;
    bool                     rx_error;
    bool                     rx_timeout;
} dw3000_api_rx_result_t;

typedef enum {
    DW3000_API_EVENT_NONE          = 0U,
    DW3000_API_EVENT_TX_DONE       = 1U << 0,
    DW3000_API_EVENT_RX_DONE       = 1U << 1,
    DW3000_API_EVENT_RX_ERROR      = 1U << 2,
    DW3000_API_EVENT_RX_TIMEOUT    = 1U << 3,
    DW3000_API_EVENT_SPI_ERROR     = 1U << 4,
    DW3000_API_EVENT_AES_DONE      = 1U << 5,
    DW3000_API_EVENT_AES_ERROR     = 1U << 6,
    DW3000_API_EVENT_CLOCK_WARNING = 1U << 7,
    DW3000_API_EVENT_GPIO          = 1U << 8,
    DW3000_API_EVENT_COMMAND_ERROR = 1U << 9,
} dw3000_api_event_flags_t;

typedef struct {
    dw3000_txrx_event_t       status;
    dw3000_api_event_flags_t  flags;
} dw3000_api_events_t;

void dw3000_api_default_config(dw3000_api_config_t* config);

void dw3000_api_default_tx_options(dw3000_api_tx_options_t* options);

void dw3000_api_default_rx_options(dw3000_api_rx_options_t* options);

dw3000_error_t dw3000_api_init(
    dw3000_device_t*              device,
    const dw3000_port_t*          port,
    const dw3000_device_config_t* device_config,
    const dw3000_api_config_t*    api_config
);

dw3000_error_t dw3000_api_deinit(dw3000_device_t* device);

dw3000_error_t dw3000_api_reset_recover(
    dw3000_device_t*           device,
    const dw3000_api_config_t* api_config
);

dw3000_error_t dw3000_api_send_frame(
    dw3000_device_t*                 device,
    const void*                      data,
    size_t                           data_len,
    const dw3000_api_tx_options_t*   options,
    dw3000_api_tx_result_t*          result
);

dw3000_error_t dw3000_api_receive_frame(
    dw3000_device_t*                 device,
    void*                            buffer,
    size_t                           buffer_len,
    const dw3000_api_rx_options_t*   options,
    dw3000_api_rx_result_t*          result
);

dw3000_error_t dw3000_api_handle_events(
    dw3000_device_t*       device,
    bool                   clear,
    dw3000_api_events_t*   events
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_API_H */
