#include "dw3000_device.h"

#include <stdlib.h>

#include "dw3000_hal/core.h"

dw3000_device_t* dw3000_device_new(
    const dw3000_port_t*          port,
    const dw3000_device_config_t* config
) {
    dw3000_device_t* device;

    device = (dw3000_device_t*)malloc(sizeof(*device));
    if (device == NULL) {
        return NULL;
    }

    if (dw3000_hal_init_context(device, port, config) != DW3000_ERROR_OK) {
        free(device);
        return NULL;
    }

    return device;
}

void dw3000_device_delete(dw3000_device_t* device) {
    free(device);
}
