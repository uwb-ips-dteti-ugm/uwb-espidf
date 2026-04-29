#ifndef DW3000_TYPES_SYNC_H
#define DW3000_TYPES_SYNC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EC_CTRL (0x04:00) — external-sync arm bits, used together with the
   SYNC pin to align the system-time counter (or pulse a reset) across
   multiple chips. OSTSM / OSRSM are one-shot: they auto-clear after
   the next external pulse. */
typedef enum {
    DW3000_SYNC_OSTSM  = 1U << 0,  /* arm one-shot timestamp sync */
    DW3000_SYNC_OSRSM  = 1U << 1,  /* arm one-shot reset sync     */
    DW3000_SYNC_PLLLDT = 1U << 3,  /* PLL lock-detect tune        */
    DW3000_SYNC_OSTRM  = 1U << 12, /* one-shot TX reference mark  */
} dw3000_sync_ec_ctrl_flags_t;

/* EC_CTRL.WAIT (bits [11:4]) — 8-bit count of system clocks to wait
   between the external pulse and applying the sync. */
typedef uint8_t dw3000_sync_wait_t;

typedef struct {
    dw3000_sync_ec_ctrl_flags_t flags;
    dw3000_sync_wait_t          wait;
} dw3000_sync_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_SYNC_H */
