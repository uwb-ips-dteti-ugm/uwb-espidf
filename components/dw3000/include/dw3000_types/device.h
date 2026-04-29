#ifndef DW3000_TYPES_DEVICE_H
#define DW3000_TYPES_DEVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DEV_ID (0x00:00) — 32-bit device identifier read on every cold boot
   to confirm silicon presence and dispatch revision-specific
   calibration paths. Layout:
     [ 3: 0] REV    silicon revision
     [ 7: 4] VER    variant
     [15: 8] MODEL  chip family   (DW3000 family = 0x03)
     [31:16] RIDTAG manufacturer  (Decawave   = 0xDECA) */
typedef struct {
    uint8_t  rev;
    uint8_t  ver;
    uint8_t  model;
    uint16_t ridtag;
} dw3000_device_id_t;

#define DW3000_DEVICE_RIDTAG_DECAWAVE 0xDECAU
#define DW3000_DEVICE_MODEL_DW3000    0x03U

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_DEVICE_H */
