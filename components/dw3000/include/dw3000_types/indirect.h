#ifndef DW3000_TYPES_INDIRECT_H
#define DW3000_TYPES_INDIRECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DW3000 has two indirect-access windows, A and B, that translate
   transactions on file 0x1D / 0x1E into a write/read at the (file_id,
   offset) currently programmed in PTR_ADDR_x / PTR_OFFSET_x (file
   0x1F). The HAL uses these to reach registers whose native address
   isn't expressible in the 6-bit file_id of the standard SPI header
   (e.g. AON RAM contents reflected via 0x1A bank). */

typedef enum {
    DW3000_INDIRECT_WINDOW_A = 0U,
    DW3000_INDIRECT_WINDOW_B = 1U,
} dw3000_indirect_window_t;

/* PTR_ADDR_A/B (0x1F:04, 0x1F:0C) + PTR_OFFSET_A/B (0x1F:08, 0x1F:10). */
typedef struct {
    uint8_t  file_id;
    uint16_t offset;
} dw3000_indirect_pointer_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_INDIRECT_H */
