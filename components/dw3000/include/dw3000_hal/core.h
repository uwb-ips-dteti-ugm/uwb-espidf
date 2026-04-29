#ifndef DW3000_HAL_CORE_H
#define DW3000_HAL_CORE_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_DEFAULT_RESET_ASSERT_US  10U
#define DW3000_HAL_DEFAULT_RESET_SETTLE_US  2000U
#define DW3000_HAL_DEFAULT_READY_TIMEOUT_US 10000U

/* Host-side timing for the initial reset/probe path. The core bring-up
   step establishes communication and device identity only; subsystem
   configuration is applied by the later PHY/MAC/TX/RX/STS HAL modules. */
typedef struct {
    uint32_t reset_assert_us;
    uint32_t reset_settle_us;
    uint32_t ready_timeout_us;
} dw3000_hal_bringup_t;

void dw3000_hal_default_bringup(dw3000_hal_bringup_t* bringup);

void dw3000_hal_default_config(dw3000_device_config_t* config);

/* Initialize the host-side context. This copies the port and desired
   configuration, clears runtime caches, and does not touch the DW3000. */
dw3000_error_t dw3000_hal_init_context(
    dw3000_device_t*              device,
    const dw3000_port_t*          port,
    const dw3000_device_config_t* config
);

/* Drive the optional reset callback. Timings are exact values supplied by
   the caller; use dw3000_hal_default_bringup() for recommended defaults. */
dw3000_error_t dw3000_hal_hard_reset(
    dw3000_device_t* device,
    uint32_t         assert_us,
    uint32_t         settle_us
);

/* Read and decode DEV_ID. The caller owns writing it into device->id unless
   using dw3000_hal_probe(), which updates the device context on success. */
dw3000_error_t dw3000_hal_read_device_id(
    dw3000_device_t*    device,
    dw3000_device_id_t* id
);

bool dw3000_hal_is_supported_device_id(const dw3000_device_id_t* id);

dw3000_device_capabilities_t dw3000_hal_capabilities_from_device_id(
    const dw3000_device_id_t* id
);

/* Validate chip identity and update device->id, capabilities, and PRESENT. */
dw3000_error_t dw3000_hal_probe(dw3000_device_t* device);

/* Wait until SYS_STATUS.SPIRDY is observed. The implementation clears SPIRDY
   after observing it, leaving other status bits untouched. */
dw3000_error_t dw3000_hal_wait_for_spi_ready(
    dw3000_device_t* device,
    uint32_t         timeout_us
);

/* Convenience wrapper for context init + optional hardware reset + SPIRDY
   wait + probe. NULL bringup uses dw3000_hal_default_bringup() values. */
dw3000_error_t dw3000_hal_bringup(
    dw3000_device_t*              device,
    const dw3000_hal_bringup_t*   bringup,
    const dw3000_port_t*          port,
    const dw3000_device_config_t* config
);

static inline bool dw3000_hal_has_capability(
    const dw3000_device_t*       device,
    dw3000_device_capabilities_t capability
) {
    return (device != NULL) && ((device->capabilities & capability) == capability);
}

static inline bool dw3000_hal_has_state(
    const dw3000_device_t*      device,
    dw3000_device_state_flags_t state
) {
    return (device != NULL) && ((device->state_flags & state) == state);
}

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_CORE_H */
