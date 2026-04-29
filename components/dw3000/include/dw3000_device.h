#ifndef DW3000_DEVICE_H
#define DW3000_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_port.h"
#include "dw3000_types/aon.h"
#include "dw3000_types/calib.h"
#include "dw3000_types/cia.h"
#include "dw3000_types/device.h"
#include "dw3000_types/mac.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/pmsc.h"
#include "dw3000_types/rx_tune.h"
#include "dw3000_types/sts.h"
#include "dw3000_types/txrx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dw3000_phy_config_t     phy;
    dw3000_mac_config_t     mac;
    dw3000_sts_config_t     sts;
    dw3000_cia_config_t     cia;
    dw3000_pmsc_config_t    pmsc;
    dw3000_aon_config_t     aon;
    dw3000_calib_config_t   calib;
    dw3000_rx_tune_config_t rx_tune;

    bool use_double_buffer;
    bool load_otp_calibration;
    bool auto_init_pll;
} dw3000_device_config_t;

typedef enum {
    DW3000_DEVICE_CAP_NONE = 0U,
    DW3000_DEVICE_CAP_PDOA = 1U << 0,
} dw3000_device_capabilities_t;

typedef enum {
    DW3000_DEVICE_STATE_NONE        = 0U,
    DW3000_DEVICE_STATE_PORT_READY  = 1U << 0,
    DW3000_DEVICE_STATE_PRESENT     = 1U << 1,
    DW3000_DEVICE_STATE_INITIALIZED = 1U << 2,
    DW3000_DEVICE_STATE_IDLE_RC     = 1U << 3,
    DW3000_DEVICE_STATE_IDLE_PLL    = 1U << 4,
    DW3000_DEVICE_STATE_RX_ON       = 1U << 5,
    DW3000_DEVICE_STATE_TX_PENDING  = 1U << 6,
    DW3000_DEVICE_STATE_SLEEPING    = 1U << 7,
} dw3000_device_state_flags_t;

typedef struct {
    dw3000_port_t                port;
    dw3000_device_config_t       config;
    dw3000_device_id_t           id;
    dw3000_device_capabilities_t capabilities;
    dw3000_device_state_flags_t  state_flags;

    uint32_t            sys_cfg_cache;
    bool                sys_cfg_cache_valid;
    dw3000_txrx_event_t enabled_events;
    bool                enabled_events_valid;

    uint8_t active_rx_buffer;
} dw3000_device_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_DEVICE_H */
