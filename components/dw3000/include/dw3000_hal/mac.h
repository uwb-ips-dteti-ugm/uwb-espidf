#ifndef DW3000_HAL_MAC_H
#define DW3000_HAL_MAC_H

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/mac.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_error_t dw3000_hal_mac_validate_ack_response(
    const dw3000_mac_ack_resp_t* ack_resp
);

dw3000_error_t dw3000_hal_mac_validate_config(
    const dw3000_mac_config_t* config
);

dw3000_error_t dw3000_hal_mac_read_eui(
    dw3000_device_t*  device,
    dw3000_mac_eui_t* eui
);

dw3000_error_t dw3000_hal_mac_set_eui(
    dw3000_device_t* device,
    dw3000_mac_eui_t eui
);

dw3000_error_t dw3000_hal_mac_read_panadr(
    dw3000_device_t*     device,
    dw3000_mac_panadr_t* panadr
);

dw3000_error_t dw3000_hal_mac_set_panadr(
    dw3000_device_t*           device,
    const dw3000_mac_panadr_t* panadr
);

dw3000_error_t dw3000_hal_mac_read_frame_filter(
    dw3000_device_t*       device,
    dw3000_mac_ff_flags_t* flags
);

dw3000_error_t dw3000_hal_mac_configure_frame_filter(
    dw3000_device_t*      device,
    dw3000_mac_ff_flags_t flags
);

dw3000_error_t dw3000_hal_mac_read_sys_cfg(
    dw3000_device_t*            device,
    dw3000_mac_sys_cfg_flags_t* flags
);

dw3000_error_t dw3000_hal_mac_configure_sys_cfg(
    dw3000_device_t*           device,
    dw3000_mac_sys_cfg_flags_t flags
);

dw3000_error_t dw3000_hal_mac_read_ack_response(
    dw3000_device_t*       device,
    dw3000_mac_ack_resp_t* ack_resp
);

dw3000_error_t dw3000_hal_mac_configure_ack_response(
    dw3000_device_t*             device,
    const dw3000_mac_ack_resp_t* ack_resp
);

dw3000_error_t dw3000_hal_mac_read_pending_addresses(
    dw3000_device_t*      device,
    dw3000_mac_le_pend_t* le_pend
);

dw3000_error_t dw3000_hal_mac_configure_pending_addresses(
    dw3000_device_t*            device,
    const dw3000_mac_le_pend_t* le_pend
);

/* Apply EUI, PANADR, pending-address entries, frame-filter rules,
   ACK/W4R timing, then enable MAC-owned SYS_CFG bits. */
dw3000_error_t dw3000_hal_mac_configure(
    dw3000_device_t*           device,
    const dw3000_mac_config_t* config
);

dw3000_error_t dw3000_hal_mac_configure_current(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_MAC_H */
