#ifndef DW3000_TYPES_CORE_H
#define DW3000_TYPES_CORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t dw3000_timestamp_t;
typedef uint64_t dw3000_eui_t;

typedef enum {
    DW3000_CHANNEL_5 = 5,
    DW3000_CHANNEL_9 = 9,
} dw3000_channel_t;

typedef enum {
    DW3000_PRF_16_MHZ = 16,
    DW3000_PRF_64_MHZ = 64,
} dw3000_prf_t;

typedef enum {
    DW3000_DATA_RATE_850K = 850,
    DW3000_DATA_RATE_6M8  = 6800,
} dw3000_data_rate_t;

typedef enum {
    DW3000_START_IMMEDIATE = 0,
    DW3000_START_DELAYED_ABSOLUTE,
    DW3000_START_DELAYED_FROM_TX,
    DW3000_START_DELAYED_FROM_RX,
    DW3000_START_DELAYED_FROM_REFERENCE,
} dw3000_start_mode_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_CORE_H */
