#ifndef DW3000_REGISTER_H
#define DW3000_REGISTER_H

#include <stddef.h>
#include <stdint.h>

#include "dw3000_types/device.h"
#include "dw3000_types/err.h"
#include "dw3000_types/reg.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_err_t dw3000_reg_read(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    void* data,
    size_t data_len
);

dw3000_err_t dw3000_reg_write(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    const void* data,
    size_t data_len
);

dw3000_err_t dw3000_reg_read_u8(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint8_t* value
);

dw3000_err_t dw3000_reg_read_u16(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint16_t* value
);

dw3000_err_t dw3000_reg_read_u32(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint32_t* value
);

dw3000_err_t dw3000_reg_write_u8(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint8_t value
);

dw3000_err_t dw3000_reg_write_u16(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint16_t value
);

dw3000_err_t dw3000_reg_write_u32(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint32_t value
);

dw3000_err_t dw3000_reg_modify_u32(
    dw3000_device_t* device,
    dw3000_reg_desc_t reg,
    uint32_t mask,
    uint32_t value
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_REGISTER_H */
