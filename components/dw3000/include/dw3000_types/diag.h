#ifndef DW3000_TYPES_DIAG_H
#define DW3000_TYPES_DIAG_H

#include <stdbool.h>
#include <stdint.h>

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dw3000_timestamp_t rx_timestamp;
    dw3000_timestamp_t tx_timestamp;
    int16_t first_path_power_dbm_x100;
    int16_t rx_level_dbm_x100;
    uint16_t frame_length;
    bool sts_quality_ok;
} dw3000_diag_t;

typedef struct {
    bool rx_done;
    bool rx_fcs_good;
    bool rx_timeout;
    bool rx_overrun;
    bool tx_done;
    bool cia_done;
    bool sts_error;
    bool hpd_warn;
} dw3000_diag_status_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_DIAG_H */
