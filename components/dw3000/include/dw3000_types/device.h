#ifndef DW3000_TYPES_DEVICE_H
#define DW3000_TYPES_DEVICE_H

#include <stdint.h>

#include "mac.h"
#include "phy.h"
#include "port.h"
#include "sleep.h"
#include "sts.h"
#include "sync.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dw3000_phy_config_t phy;
    dw3000_mac_config_t mac;
    dw3000_sts_config_t sts;
    dw3000_sync_config_t sync;
    dw3000_sleep_config_t sleep;
    dw3000_address_t address;
} dw3000_device_config_t;

typedef struct {
    dw3000_port_t port;
    dw3000_device_config_t config;
    uint32_t state_flags;
} dw3000_device_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_DEVICE_H */
