#ifndef DW3000_TYPES_TXRX_H
#define DW3000_TYPES_TXRX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const void* payload;
    size_t length;
    dw3000_start_mode_t start_mode;
    dw3000_timestamp_t reference_time;
    bool expect_response;
    bool clear_channel_assessment;
} dw3000_txrx_tx_frame_t;

typedef struct {
    void* buffer;
    size_t buffer_size;
    dw3000_start_mode_t start_mode;
    dw3000_timestamp_t reference_time;
    uint32_t timeout_us;
    bool double_buffered;
} dw3000_txrx_rx_request_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_TXRX_H */
