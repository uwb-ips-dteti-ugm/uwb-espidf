#ifndef DW3000_HAL_TX_H
#define DW3000_HAL_TX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/cia.h"
#include "dw3000_types/fcmd.h"
#include "dw3000_types/txrx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_TX_BUFFER_SIZE        1024U
#define DW3000_HAL_TX_MAX_FRAME_STANDARD 127U
#define DW3000_HAL_TX_MAX_FRAME_EXTENDED 1023U
#define DW3000_HAL_TX_AUTO_FCS_LEN       2U

dw3000_error_t dw3000_hal_tx_validate_frame(
    const dw3000_device_t*        device,
    const dw3000_txrx_tx_frame_t* frame
);

dw3000_error_t dw3000_hal_tx_write_buffer(
    dw3000_device_t* device,
    uint16_t         offset,
    const void*      data,
    size_t           data_len
);

dw3000_error_t dw3000_hal_tx_configure_frame(
    dw3000_device_t*              device,
    const dw3000_txrx_tx_frame_t* frame
);

dw3000_error_t dw3000_hal_tx_prepare_frame(
    dw3000_device_t*              device,
    const void*                   data,
    size_t                        data_len,
    const dw3000_txrx_tx_frame_t* frame
);

dw3000_error_t dw3000_hal_tx_set_delayed_time(
    dw3000_device_t*           device,
    dw3000_txrx_delayed_time_t time
);

dw3000_error_t dw3000_hal_tx_set_reference_time(
    dw3000_device_t*           device,
    dw3000_txrx_delayed_time_t time
);

dw3000_error_t dw3000_hal_tx_set_antenna_delay(
    dw3000_device_t*           device,
    dw3000_cia_antenna_delay_t delay
);

dw3000_error_t dw3000_hal_tx_read_timestamp(
    dw3000_device_t*         device,
    dw3000_txrx_timestamp_t* timestamp
);

dw3000_error_t dw3000_hal_tx_read_raw_timestamp(
    dw3000_device_t*            device,
    dw3000_txrx_delayed_time_t* timestamp
);

bool dw3000_hal_tx_is_start_command(dw3000_fcmd_t command);

dw3000_error_t dw3000_hal_tx_start(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
);

dw3000_error_t dw3000_hal_tx_start_immediate(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_immediate_w4r(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_w4r(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_ts(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_ts_w4r(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_rs(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_rs_w4r(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_ref(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_delayed_ref_w4r(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_cca(dw3000_device_t* device);

dw3000_error_t dw3000_hal_tx_start_cca_w4r(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_TX_H */
