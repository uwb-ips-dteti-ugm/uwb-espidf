#ifndef DW3000_TYPES_DB_DIAG_H
#define DW3000_TYPES_DB_DIAG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DB_DIAG (0x18) — 464-byte extended per-buffer diagnostic snapshot
   used with double-buffered RX. The block is split into two equal
   halves, one per RX buffer, populated automatically when the HW
   commits a frame. The content of each half is determined by
   RDB_DIAG.RDB_DMODE (see dw3000_txrx_rdb_dmode_t in txrx.h):
     MINIMAL — RX status only
     MEDIUM  — adds RX_TIME / RX_FINFO snapshots
     FULL    — adds IP_TS, STS_TS, and full CIA diagnostics
   Layout-dependent decoding stays in the HAL; here we only expose the
   block size primitives. */
#define DW3000_DB_DIAG_BUFFER_SIZE 232U
#define DW3000_DB_DIAG_TOTAL_SIZE  (2U * DW3000_DB_DIAG_BUFFER_SIZE)

typedef uint8_t dw3000_db_diag_block_t[DW3000_DB_DIAG_BUFFER_SIZE];

typedef enum {
    DW3000_DB_DIAG_BUFFER_0 = 0U,
    DW3000_DB_DIAG_BUFFER_1 = 1U,
} dw3000_db_diag_buffer_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_DB_DIAG_H */
