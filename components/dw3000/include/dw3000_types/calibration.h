#ifndef DW3000_TYPES_CALIBRATION_H
#define DW3000_TYPES_CALIBRATION_H

#include <stdint.h>

#include "phy.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t crystal_trim;
    dw3000_phy_tx_power_t tx_power;
    dw3000_phy_antenna_delay_t antenna_delay;
} dw3000_calibration_data_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_CALIBRATION_H */
