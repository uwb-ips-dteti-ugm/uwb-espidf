#ifndef DW3000_TYPES_ACC_H
#define DW3000_TYPES_ACC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ACC_MEM (0x15) — 12 288-byte channel-impulse-response accumulator.
   Each sample is one complex value (real + imaginary) sign-extended
   from the on-chip width into int32_t for arithmetic convenience. The
   sample width and on-the-wire stride depend on RX configuration (PRF
   and accumulator mode); a leading dummy byte is required on the SPI
   read of the first sample. The HAL handles those plumbing details
   and surfaces decoded samples to callers. */
#define DW3000_ACC_MEM_SIZE 12288U

typedef struct {
    int32_t real;
    int32_t imag;
} dw3000_acc_sample_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_ACC_H */
