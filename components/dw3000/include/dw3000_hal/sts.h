#ifndef DW3000_HAL_STS_H
#define DW3000_HAL_STS_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/sts.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_STS_CPS_LEN_MIN     3U
#define DW3000_HAL_STS_CPS_LEN_DEFAULT 7U
#define DW3000_HAL_STS_CPS_LEN_MAX     255U

dw3000_error_t dw3000_hal_sts_validate_sys_cfg(
    dw3000_sts_packet_cfg_t    packet_cfg,
    dw3000_sts_pdoa_mode_t     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t flags
);

dw3000_error_t dw3000_hal_sts_validate_config(
    const dw3000_sts_config_t* config
);

uint16_t dw3000_hal_sts_length_units(uint8_t cps_len);

uint16_t dw3000_hal_sts_acc_qual_threshold(uint8_t cps_len);

bool dw3000_hal_sts_acc_qual_is_sufficient(
    uint8_t               cps_len,
    dw3000_sts_acc_qual_t acc_qual
);

dw3000_error_t dw3000_hal_sts_read_config(
    dw3000_device_t*     device,
    dw3000_sts_config_t* config
);

dw3000_error_t dw3000_hal_sts_read_length(
    dw3000_device_t* device,
    uint8_t*         cps_len
);

dw3000_error_t dw3000_hal_sts_set_length(
    dw3000_device_t* device,
    uint8_t          cps_len
);

dw3000_error_t dw3000_hal_sts_read_acc_qual(
    dw3000_device_t*       device,
    dw3000_sts_acc_qual_t* acc_qual
);

dw3000_error_t dw3000_hal_sts_read_key(
    dw3000_device_t*  device,
    dw3000_sts_key_t* key
);

dw3000_error_t dw3000_hal_sts_set_key(
    dw3000_device_t*        device,
    const dw3000_sts_key_t* key
);

dw3000_error_t dw3000_hal_sts_read_iv(
    dw3000_device_t* device,
    dw3000_sts_iv_t* iv
);

dw3000_error_t dw3000_hal_sts_set_iv(
    dw3000_device_t*       device,
    const dw3000_sts_iv_t* iv
);

dw3000_error_t dw3000_hal_sts_read_counter(
    dw3000_device_t* device,
    uint32_t*        counter
);

dw3000_error_t dw3000_hal_sts_read_sys_cfg(
    dw3000_device_t*            device,
    dw3000_sts_packet_cfg_t*    packet_cfg,
    dw3000_sts_pdoa_mode_t*     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t* flags
);

dw3000_error_t dw3000_hal_sts_configure_sys_cfg(
    dw3000_device_t*           device,
    dw3000_sts_packet_cfg_t    packet_cfg,
    dw3000_sts_pdoa_mode_t     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t flags
);

dw3000_error_t dw3000_hal_sts_issue_control(
    dw3000_device_t*        device,
    dw3000_sts_ctrl_flags_t flags
);

dw3000_error_t dw3000_hal_sts_load_iv(dw3000_device_t* device);

dw3000_error_t dw3000_hal_sts_restart_from_last(dw3000_device_t* device);

/* Apply STS length, key, IV, STS-owned SYS_CFG bits, then LOAD_IV when
   AES-based STS is enabled. */
dw3000_error_t dw3000_hal_sts_configure(
    dw3000_device_t*           device,
    const dw3000_sts_config_t* config
);

dw3000_error_t dw3000_hal_sts_configure_current(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_STS_H */
