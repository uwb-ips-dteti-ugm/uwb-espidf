#ifndef DW3000_HAL_RX_H
#define DW3000_HAL_RX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/fcmd.h"
#include "dw3000_types/txrx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_RX_BUFFER_SIZE 1024U

dw3000_txrx_event_t dw3000_hal_rx_success_events(void);

dw3000_txrx_event_t dw3000_hal_rx_error_events(void);

dw3000_txrx_event_t dw3000_hal_rx_all_events(void);

dw3000_error_t dw3000_hal_rx_configure_sys_cfg(
    dw3000_device_t*          device,
    dw3000_txrx_sys_cfg_flags_t flags
);

dw3000_error_t dw3000_hal_rx_configure_default(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rx_set_frame_wait_timeout(
    dw3000_device_t*  device,
    dw3000_txrx_fwto_t timeout
);

dw3000_error_t dw3000_hal_rx_set_delayed_time(
    dw3000_device_t*             device,
    dw3000_txrx_delayed_time_t   time
);

dw3000_error_t dw3000_hal_rx_set_reference_time(
    dw3000_device_t*             device,
    dw3000_txrx_delayed_time_t   time
);

dw3000_error_t dw3000_hal_rx_read_finfo(
    dw3000_device_t*          device,
    dw3000_txrx_rx_finfo_t*   finfo
);

dw3000_error_t dw3000_hal_rx_read_buffer(
    dw3000_device_t* device,
    uint8_t          buffer_index,
    uint16_t         offset,
    void*            data,
    size_t           data_len
);

dw3000_error_t dw3000_hal_rx_read_active_buffer(
    dw3000_device_t* device,
    uint16_t         offset,
    void*            data,
    size_t           data_len
);

dw3000_error_t dw3000_hal_rx_read_timestamp(
    dw3000_device_t*          device,
    dw3000_txrx_timestamp_t*  timestamp
);

dw3000_error_t dw3000_hal_rx_read_raw_timestamp(
    dw3000_device_t*             device,
    dw3000_txrx_delayed_time_t*  timestamp
);

dw3000_error_t dw3000_hal_rx_read_double_buffer_status(
    dw3000_device_t*          device,
    dw3000_txrx_rdb_status_t* status
);

dw3000_error_t dw3000_hal_rx_clear_double_buffer_status(
    dw3000_device_t*         device,
    dw3000_txrx_rdb_status_t status
);

dw3000_error_t dw3000_hal_rx_read_double_buffer_diag_mode(
    dw3000_device_t*        device,
    dw3000_txrx_rdb_dmode_t* mode
);

dw3000_error_t dw3000_hal_rx_set_double_buffer_diag_mode(
    dw3000_device_t*       device,
    dw3000_txrx_rdb_dmode_t mode
);

dw3000_error_t dw3000_hal_rx_read_sniff(
    dw3000_device_t*     device,
    dw3000_txrx_sniff_t* sniff
);

dw3000_error_t dw3000_hal_rx_configure_sniff(
    dw3000_device_t*           device,
    const dw3000_txrx_sniff_t* sniff
);

dw3000_error_t dw3000_hal_rx_disable_sniff(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rx_clear_events(
    dw3000_device_t*     device,
    dw3000_txrx_event_t  events
);

bool dw3000_hal_rx_is_start_command(dw3000_fcmd_t command);

dw3000_error_t dw3000_hal_rx_start(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
);

dw3000_error_t dw3000_hal_rx_start_immediate(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rx_start_delayed(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rx_start_delayed_ts(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rx_start_delayed_rs(dw3000_device_t* device);

dw3000_error_t dw3000_hal_rx_start_delayed_ref(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_RX_H */
