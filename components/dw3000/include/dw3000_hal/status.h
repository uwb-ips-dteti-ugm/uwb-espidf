#ifndef DW3000_HAL_STATUS_H
#define DW3000_HAL_STATUS_H

#include <stdbool.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/txrx.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_txrx_event_t dw3000_hal_status_clearable_events(void);

dw3000_txrx_event_t dw3000_hal_status_enable_events(void);

bool dw3000_hal_status_has_event(
    dw3000_txrx_event_t status,
    dw3000_txrx_event_t event
);

dw3000_error_t dw3000_hal_status_read(
    dw3000_device_t*     device,
    dw3000_txrx_event_t* status
);

/* SYS_STATUS is write-1-to-clear. Reserved and read-only bits are masked out. */
dw3000_error_t dw3000_hal_status_clear(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
);

dw3000_error_t dw3000_hal_status_clear_all(dw3000_device_t* device);

dw3000_error_t dw3000_hal_status_read_enabled(
    dw3000_device_t*     device,
    dw3000_txrx_event_t* events
);

dw3000_error_t dw3000_hal_status_set_enabled(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
);

dw3000_error_t dw3000_hal_status_enable(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
);

dw3000_error_t dw3000_hal_status_disable(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_STATUS_H */
