#ifndef DW3000_HAL_FCMD_H
#define DW3000_HAL_FCMD_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/fcmd.h"

#ifdef __cplusplus
extern "C" {
#endif

bool dw3000_hal_fcmd_is_valid(dw3000_fcmd_t command);

uint8_t dw3000_hal_fcmd_header(dw3000_fcmd_t command);

/* Issue a single-octet fast command. The SPI transaction has no data phase. */
dw3000_error_t dw3000_hal_fcmd_issue(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
);

dw3000_error_t dw3000_hal_fcmd_txrxoff(dw3000_device_t* device);

dw3000_error_t dw3000_hal_fcmd_clear_irqs(dw3000_device_t* device);

dw3000_error_t dw3000_hal_fcmd_db_toggle(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_FCMD_H */
