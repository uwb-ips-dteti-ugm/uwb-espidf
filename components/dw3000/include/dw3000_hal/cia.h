#ifndef DW3000_HAL_CIA_H
#define DW3000_HAL_CIA_H

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/cia.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DW3000_HAL_CIA_PATH_IP = 0U,
    DW3000_HAL_CIA_PATH_STS,
    DW3000_HAL_CIA_PATH_STS1,
} dw3000_hal_cia_path_t;

dw3000_error_t dw3000_hal_cia_validate_config(
    const dw3000_cia_config_t* config
);

dw3000_error_t dw3000_hal_cia_read_config(
    dw3000_device_t*      device,
    dw3000_cia_config_t*  config
);

dw3000_error_t dw3000_hal_cia_read_conf(
    dw3000_device_t*              device,
    dw3000_cia_antenna_delay_t*   rx_antd,
    dw3000_cia_conf_flags_t*      flags
);

dw3000_error_t dw3000_hal_cia_configure_conf(
    dw3000_device_t*             device,
    dw3000_cia_antenna_delay_t   rx_antd,
    dw3000_cia_conf_flags_t      flags
);

dw3000_error_t dw3000_hal_cia_read_tx_antenna_delay(
    dw3000_device_t*             device,
    dw3000_cia_antenna_delay_t*  delay
);

dw3000_error_t dw3000_hal_cia_set_rx_antenna_delay(
    dw3000_device_t*            device,
    dw3000_cia_antenna_delay_t  delay
);

dw3000_error_t dw3000_hal_cia_set_tx_antenna_delay(
    dw3000_device_t*            device,
    dw3000_cia_antenna_delay_t  delay
);

dw3000_error_t dw3000_hal_cia_read_fp_conf(
    dw3000_device_t*      device,
    dw3000_cia_fp_conf_t* fp_conf
);

dw3000_error_t dw3000_hal_cia_configure_fp_conf(
    dw3000_device_t*            device,
    const dw3000_cia_fp_conf_t* fp_conf
);

dw3000_error_t dw3000_hal_cia_read_ip_conf(
    dw3000_device_t*      device,
    dw3000_cia_ip_conf_t* ip_conf
);

dw3000_error_t dw3000_hal_cia_configure_ip_conf(
    dw3000_device_t*            device,
    const dw3000_cia_ip_conf_t* ip_conf
);

dw3000_error_t dw3000_hal_cia_read_sts_conf(
    dw3000_device_t*       device,
    dw3000_cia_sts_conf_t* sts_conf
);

dw3000_error_t dw3000_hal_cia_configure_sts_conf(
    dw3000_device_t*             device,
    const dw3000_cia_sts_conf_t* sts_conf
);

dw3000_error_t dw3000_hal_cia_read_adjust(
    dw3000_device_t*       device,
    dw3000_cia_adjust_t*   adjust
);

dw3000_error_t dw3000_hal_cia_configure_adjust(
    dw3000_device_t*     device,
    dw3000_cia_adjust_t  adjust
);

dw3000_error_t dw3000_hal_cia_read_path_timestamp(
    dw3000_device_t*         device,
    dw3000_hal_cia_path_t    path,
    dw3000_cia_path_ts_t*    timestamp
);

dw3000_error_t dw3000_hal_cia_read_ip_timestamp(
    dw3000_device_t*       device,
    dw3000_cia_path_ts_t*  timestamp
);

dw3000_error_t dw3000_hal_cia_read_sts_timestamp(
    dw3000_device_t*       device,
    dw3000_cia_path_ts_t*  timestamp
);

dw3000_error_t dw3000_hal_cia_read_sts1_timestamp(
    dw3000_device_t*       device,
    dw3000_cia_path_ts_t*  timestamp
);

dw3000_error_t dw3000_hal_cia_read_tdoa(
    dw3000_device_t*    device,
    dw3000_cia_tdoa_t*  tdoa
);

dw3000_error_t dw3000_hal_cia_read_pdoa(
    dw3000_device_t*    device,
    dw3000_cia_pdoa_t*  pdoa
);

dw3000_error_t dw3000_hal_cia_read_diag(
    dw3000_device_t*  device,
    dw3000_cia_diag_t* diag
);

dw3000_error_t dw3000_hal_cia_read_path_diag(
    dw3000_device_t*          device,
    dw3000_hal_cia_path_t     path,
    dw3000_cia_path_diag_t*   diag
);

dw3000_error_t dw3000_hal_cia_read_ip_diag(
    dw3000_device_t*        device,
    dw3000_cia_path_diag_t* diag
);

dw3000_error_t dw3000_hal_cia_read_sts_diag(
    dw3000_device_t*        device,
    dw3000_cia_path_diag_t* diag
);

dw3000_error_t dw3000_hal_cia_read_sts1_diag(
    dw3000_device_t*        device,
    dw3000_cia_path_diag_t* diag
);

dw3000_error_t dw3000_hal_cia_configure(
    dw3000_device_t*           device,
    const dw3000_cia_config_t* config
);

dw3000_error_t dw3000_hal_cia_configure_current(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_CIA_H */
