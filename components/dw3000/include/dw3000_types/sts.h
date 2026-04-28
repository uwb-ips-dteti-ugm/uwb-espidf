#ifndef DW3000_TYPES_STS_H
#define DW3000_TYPES_STS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DW3000_STS_MODE_OFF = 0,
    DW3000_STS_MODE_1   = 1,
    DW3000_STS_MODE_2   = 2,
    DW3000_STS_MODE_3   = 3,
} dw3000_sts_mode_t;

typedef enum {
    DW3000_STS_PDOA_MODE_OFF = 0,
    DW3000_STS_PDOA_MODE_1   = 1,
    DW3000_STS_PDOA_MODE_3   = 3,
} dw3000_sts_pdoa_mode_t;

typedef struct {
    bool enabled;
    dw3000_sts_mode_t mode;
    dw3000_sts_pdoa_mode_t pdoa_mode;
    bool super_deterministic_code;
    uint8_t key[16];
    uint8_t iv[16];
} dw3000_sts_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_STS_H */
