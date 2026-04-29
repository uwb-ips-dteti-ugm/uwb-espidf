#ifndef DW3000_HAL_ACC_H
#define DW3000_HAL_ACC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/acc.h"

#ifdef __cplusplus
extern "C" {
#endif

bool dw3000_hal_acc_sample_span_is_valid(
    uint16_t start_sample,
    size_t   sample_count
);

size_t dw3000_hal_acc_ipatov_sample_count(const dw3000_device_t* device);

dw3000_error_t dw3000_hal_acc_cir_span(
    const dw3000_device_t* device,
    dw3000_acc_cir_t       cir,
    uint16_t*              start_sample,
    size_t*                sample_count
);

int32_t dw3000_hal_acc_decode_i24(const uint8_t bytes[3]);

dw3000_acc_sample_t dw3000_hal_acc_decode_sample(
    const uint8_t bytes[DW3000_ACC_SAMPLE_SIZE]
);

dw3000_error_t dw3000_hal_acc_read_raw_samples(
    dw3000_device_t* device,
    uint16_t         start_sample,
    void*            data,
    size_t           sample_count
);

dw3000_error_t dw3000_hal_acc_read_samples(
    dw3000_device_t*       device,
    uint16_t               start_sample,
    dw3000_acc_sample_t*   samples,
    size_t                 sample_count
);

dw3000_error_t dw3000_hal_acc_read_sample(
    dw3000_device_t*      device,
    uint16_t              sample_index,
    dw3000_acc_sample_t*  sample
);

dw3000_error_t dw3000_hal_acc_read_cir(
    dw3000_device_t*       device,
    dw3000_acc_cir_t       cir,
    dw3000_acc_sample_t*   samples,
    size_t                 sample_count
);

dw3000_error_t dw3000_hal_acc_read_ipatov_cir(
    dw3000_device_t*       device,
    dw3000_acc_sample_t*   samples,
    size_t                 sample_count
);

dw3000_error_t dw3000_hal_acc_read_sts_cir(
    dw3000_device_t*       device,
    uint8_t                sts_segment,
    dw3000_acc_sample_t*   samples,
    size_t                 sample_count
);

dw3000_error_t dw3000_hal_acc_check_cia_done(dw3000_device_t* device);

dw3000_error_t dw3000_hal_acc_read_samples_checked(
    dw3000_device_t*       device,
    uint16_t               start_sample,
    dw3000_acc_sample_t*   samples,
    size_t                 sample_count
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_ACC_H */
