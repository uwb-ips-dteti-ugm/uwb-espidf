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
#define DW3000_ACC_SAMPLE_SIZE 6U
#define DW3000_ACC_SAMPLE_COUNT (DW3000_ACC_MEM_SIZE / DW3000_ACC_SAMPLE_SIZE)

#define DW3000_ACC_IPATOV_START_SAMPLE 0U
#define DW3000_ACC_IPATOV_16M_SAMPLES  992U
#define DW3000_ACC_IPATOV_64M_SAMPLES  1016U
#define DW3000_ACC_STS0_START_SAMPLE   1024U
#define DW3000_ACC_STS1_START_SAMPLE   1536U
#define DW3000_ACC_STS_SAMPLES         512U

typedef struct {
    int32_t real;
    int32_t imag;
} dw3000_acc_sample_t;

typedef enum {
    DW3000_ACC_CIR_IPATOV = 0U,
    DW3000_ACC_CIR_STS0   = 1U,
    DW3000_ACC_CIR_STS1   = 2U,
} dw3000_acc_cir_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_ACC_H */
