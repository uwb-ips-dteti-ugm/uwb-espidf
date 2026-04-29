#include "dw3000_hal/acc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_hal/status.h"
#include "dw3000_register.h"

#define DW3000_HAL_ACC_CHUNK_SAMPLES 16U
#define DW3000_HAL_ACC_CHUNK_BYTES \
    (1U + (DW3000_HAL_ACC_CHUNK_SAMPLES * DW3000_ACC_SAMPLE_SIZE))

#define DW3000_HAL_ACC_CLOCK_FLAGS \
    ((uint32_t)DW3000_PMSC_CLK_ACC_CLK_EN | \
     (uint32_t)DW3000_PMSC_CLK_ACC_MCLK_EN)

static bool dw3000_hal_acc_can_access(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static size_t dw3000_hal_acc_min_size(size_t a, size_t b) {
    return (a < b) ? a : b;
}

static dw3000_error_t dw3000_hal_acc_set_clock_flags(
    dw3000_device_t* device,
    uint32_t         flags
) {
    dw3000_error_t err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CLK_CTRL,
        flags,
        flags
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.clk_ctrl.flags = (dw3000_pmsc_clk_flags_t)(
        device->config.pmsc.clk_ctrl.flags | (dw3000_pmsc_clk_flags_t)flags
    );
    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_acc_clear_clock_flags(
    dw3000_device_t* device,
    uint32_t         flags
) {
    dw3000_error_t err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CLK_CTRL,
        flags,
        0U
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.pmsc.clk_ctrl.flags = (dw3000_pmsc_clk_flags_t)(
        device->config.pmsc.clk_ctrl.flags & ~(dw3000_pmsc_clk_flags_t)flags
    );
    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_acc_prepare_read(
    dw3000_device_t* device,
    uint32_t*        added_clock_flags
) {
    dw3000_error_t err;
    uint32_t       clock_ctrl;

    if ((device == NULL) || (added_clock_flags == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_acc_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CLK_CTRL, &clock_ctrl);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *added_clock_flags = DW3000_HAL_ACC_CLOCK_FLAGS & ~clock_ctrl;
    if (*added_clock_flags == 0U) {
        return DW3000_ERROR_OK;
    }

    return dw3000_hal_acc_set_clock_flags(device, *added_clock_flags);
}

static dw3000_error_t dw3000_hal_acc_finish_read(
    dw3000_device_t* device,
    uint32_t         added_clock_flags
) {
    if ((device == NULL) || (added_clock_flags == 0U)) {
        return DW3000_ERROR_OK;
    }

    return dw3000_hal_acc_clear_clock_flags(device, added_clock_flags);
}

static dw3000_error_t dw3000_hal_acc_read_chunk(
    dw3000_device_t* device,
    uint16_t         start_sample,
    uint8_t*         raw,
    size_t           sample_count
) {
    dw3000_reg_desc_t reg = DW3000_REG_ACC_MEM;

    reg.offset = start_sample;
    reg.length = 0U;

    return dw3000_reg_read(
        device,
        reg,
        raw,
        1U + (sample_count * DW3000_ACC_SAMPLE_SIZE)
    );
}

bool dw3000_hal_acc_sample_span_is_valid(
    uint16_t start_sample,
    size_t   sample_count
) {
    return (start_sample < DW3000_ACC_SAMPLE_COUNT) &&
           (sample_count <= DW3000_ACC_SAMPLE_COUNT) &&
           (((size_t)start_sample + sample_count) <= DW3000_ACC_SAMPLE_COUNT);
}

size_t dw3000_hal_acc_ipatov_sample_count(const dw3000_device_t* device) {
    if ((device != NULL) && (device->config.phy.prf == DW3000_PHY_PRF_16_MHZ)) {
        return DW3000_ACC_IPATOV_16M_SAMPLES;
    }

    return DW3000_ACC_IPATOV_64M_SAMPLES;
}

dw3000_error_t dw3000_hal_acc_cir_span(
    const dw3000_device_t* device,
    dw3000_acc_cir_t       cir,
    uint16_t*              start_sample,
    size_t*                sample_count
) {
    if ((start_sample == NULL) || (sample_count == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    switch (cir) {
        case DW3000_ACC_CIR_IPATOV:
            *start_sample = DW3000_ACC_IPATOV_START_SAMPLE;
            *sample_count = dw3000_hal_acc_ipatov_sample_count(device);
            return DW3000_ERROR_OK;

        case DW3000_ACC_CIR_STS0:
            *start_sample = DW3000_ACC_STS0_START_SAMPLE;
            *sample_count = DW3000_ACC_STS_SAMPLES;
            return DW3000_ERROR_OK;

        case DW3000_ACC_CIR_STS1:
            *start_sample = DW3000_ACC_STS1_START_SAMPLE;
            *sample_count = DW3000_ACC_STS_SAMPLES;
            return DW3000_ERROR_OK;

        default:
            return DW3000_ERROR_INVALID_ARG;
    }
}

int32_t dw3000_hal_acc_decode_i24(const uint8_t bytes[3]) {
    uint32_t value;

    if (bytes == NULL) {
        return 0;
    }

    value = (uint32_t)bytes[0] |
            ((uint32_t)bytes[1] << 8U) |
            ((uint32_t)bytes[2] << 16U);

    if ((value & 0x00800000UL) != 0U) {
        value |= 0xFF000000UL;
    }

    return (int32_t)value;
}

dw3000_acc_sample_t dw3000_hal_acc_decode_sample(
    const uint8_t bytes[DW3000_ACC_SAMPLE_SIZE]
) {
    dw3000_acc_sample_t sample = {0};

    if (bytes == NULL) {
        return sample;
    }

    sample.real = dw3000_hal_acc_decode_i24(&bytes[0]);
    sample.imag = dw3000_hal_acc_decode_i24(&bytes[3]);
    return sample;
}

dw3000_error_t dw3000_hal_acc_read_raw_samples(
    dw3000_device_t* device,
    uint16_t         start_sample,
    void*            data,
    size_t           sample_count
) {
    dw3000_error_t err = DW3000_ERROR_OK;
    dw3000_error_t finish_err;
    uint32_t       added_clock_flags = 0U;
    uint8_t        raw[DW3000_HAL_ACC_CHUNK_BYTES];
    uint8_t*       out = (uint8_t*)data;
    size_t         remaining = sample_count;
    uint16_t       sample_index = start_sample;

    if ((device == NULL) || ((sample_count != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (sample_count == 0U) {
        return DW3000_ERROR_OK;
    }

    if (!dw3000_hal_acc_sample_span_is_valid(start_sample, sample_count)) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    err = dw3000_hal_acc_prepare_read(device, &added_clock_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    while (remaining != 0U) {
        size_t chunk_samples = dw3000_hal_acc_min_size(
            remaining,
            DW3000_HAL_ACC_CHUNK_SAMPLES
        );
        size_t chunk_bytes = chunk_samples * DW3000_ACC_SAMPLE_SIZE;

        err = dw3000_hal_acc_read_chunk(device, sample_index, raw, chunk_samples);
        if (err != DW3000_ERROR_OK) {
            break;
        }

        for (size_t i = 0U; i < chunk_bytes; ++i) {
            out[i] = raw[i + 1U];
        }

        out += chunk_bytes;
        sample_index = (uint16_t)(sample_index + (uint16_t)chunk_samples);
        remaining -= chunk_samples;
    }

    finish_err = dw3000_hal_acc_finish_read(device, added_clock_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return finish_err;
}

dw3000_error_t dw3000_hal_acc_read_samples(
    dw3000_device_t*      device,
    uint16_t              start_sample,
    dw3000_acc_sample_t*  samples,
    size_t                sample_count
) {
    dw3000_error_t err = DW3000_ERROR_OK;
    dw3000_error_t finish_err;
    uint32_t       added_clock_flags = 0U;
    uint8_t        raw[DW3000_HAL_ACC_CHUNK_BYTES];
    size_t         remaining = sample_count;
    uint16_t       sample_index = start_sample;

    if ((device == NULL) || ((sample_count != 0U) && (samples == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (sample_count == 0U) {
        return DW3000_ERROR_OK;
    }

    if (!dw3000_hal_acc_sample_span_is_valid(start_sample, sample_count)) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    err = dw3000_hal_acc_prepare_read(device, &added_clock_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    while (remaining != 0U) {
        size_t chunk_samples = dw3000_hal_acc_min_size(
            remaining,
            DW3000_HAL_ACC_CHUNK_SAMPLES
        );

        err = dw3000_hal_acc_read_chunk(device, sample_index, raw, chunk_samples);
        if (err != DW3000_ERROR_OK) {
            break;
        }

        for (size_t i = 0U; i < chunk_samples; ++i) {
            samples[i] = dw3000_hal_acc_decode_sample(
                &raw[1U + (i * DW3000_ACC_SAMPLE_SIZE)]
            );
        }

        samples += chunk_samples;
        sample_index = (uint16_t)(sample_index + (uint16_t)chunk_samples);
        remaining -= chunk_samples;
    }

    finish_err = dw3000_hal_acc_finish_read(device, added_clock_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return finish_err;
}

dw3000_error_t dw3000_hal_acc_read_sample(
    dw3000_device_t*      device,
    uint16_t              sample_index,
    dw3000_acc_sample_t*  sample
) {
    return dw3000_hal_acc_read_samples(device, sample_index, sample, 1U);
}

dw3000_error_t dw3000_hal_acc_read_cir(
    dw3000_device_t*      device,
    dw3000_acc_cir_t      cir,
    dw3000_acc_sample_t*  samples,
    size_t                sample_count
) {
    dw3000_error_t err;
    uint16_t       start_sample;
    size_t         max_samples;

    err = dw3000_hal_acc_cir_span(device, cir, &start_sample, &max_samples);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (sample_count > max_samples) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    return dw3000_hal_acc_read_samples(device, start_sample, samples, sample_count);
}

dw3000_error_t dw3000_hal_acc_read_ipatov_cir(
    dw3000_device_t*      device,
    dw3000_acc_sample_t*  samples,
    size_t                sample_count
) {
    return dw3000_hal_acc_read_cir(
        device,
        DW3000_ACC_CIR_IPATOV,
        samples,
        sample_count
    );
}

dw3000_error_t dw3000_hal_acc_read_sts_cir(
    dw3000_device_t*      device,
    uint8_t               sts_segment,
    dw3000_acc_sample_t*  samples,
    size_t                sample_count
) {
    if (sts_segment > 1U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_acc_read_cir(
        device,
        (sts_segment == 0U) ? DW3000_ACC_CIR_STS0 : DW3000_ACC_CIR_STS1,
        samples,
        sample_count
    );
}

dw3000_error_t dw3000_hal_acc_check_cia_done(dw3000_device_t* device) {
    dw3000_error_t      err;
    dw3000_txrx_event_t status;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_status_read(device, &status);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return ((status & DW3000_TXRX_EVENT_CIADONE) != 0U) ?
        DW3000_ERROR_OK :
        DW3000_ERROR_INVALID_STATE;
}

dw3000_error_t dw3000_hal_acc_read_samples_checked(
    dw3000_device_t*      device,
    uint16_t              start_sample,
    dw3000_acc_sample_t*  samples,
    size_t                sample_count
) {
    dw3000_error_t err = dw3000_hal_acc_check_cia_done(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_acc_read_samples(device, start_sample, samples, sample_count);
}
