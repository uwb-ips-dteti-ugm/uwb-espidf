#ifndef DW3000_TYPES_SYNC_H
#define DW3000_TYPES_SYNC_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DW3000_SYNC_MODE_DISABLED = 0,
    DW3000_SYNC_MODE_OSTR,
    DW3000_SYNC_MODE_EXTERNAL_TIMEBASE,
} dw3000_sync_mode_t;

typedef struct {
    dw3000_sync_mode_t mode;
    bool trigger_tx_timestamps;
    bool trigger_rx_timestamps;
} dw3000_sync_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_SYNC_H */
