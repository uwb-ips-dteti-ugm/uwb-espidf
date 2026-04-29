#ifndef DW3000_TYPES_SYNC_H
#define DW3000_TYPES_SYNC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EC_CTRL (0x04:00) - external-sync control. OSTR mode resets the
   internal system-time counter at a deterministic delay after the SYNC
   input is sampled by the 38.4 MHz EXTCLK reference. */
typedef enum {
    DW3000_SYNC_OSTR_MODE = 1U << 11,
} dw3000_sync_ec_ctrl_flags_t;

#define DW3000_SYNC_EC_CTRL_FLAGS_MASK ((uint32_t)DW3000_SYNC_OSTR_MODE)

/* EC_CTRL.OSTS_WAIT (bits [10:3]) - 8-bit external-clock wait count.
   In OSTR mode the documented phase rule is wait % 4 == 1; 33 is the
   recommended value. */
typedef uint8_t dw3000_sync_osts_wait_t;
typedef dw3000_sync_osts_wait_t dw3000_sync_wait_t;

#define DW3000_SYNC_OSTS_WAIT_MAX         0xFFU
#define DW3000_SYNC_OSTS_WAIT_PHASE       1U
#define DW3000_SYNC_OSTS_WAIT_MODULUS     4U
#define DW3000_SYNC_OSTS_WAIT_RECOMMENDED 33U

typedef struct {
    dw3000_sync_ec_ctrl_flags_t flags;
    dw3000_sync_osts_wait_t     wait;
} dw3000_sync_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_SYNC_H */
