#include "dw3000_register.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DW3000_SPI_WRITE_BIT         0x80U
#define DW3000_SPI_SUBADDRESS_BIT    0x40U
#define DW3000_SPI_FILE_ID_SHIFT     1U
#define DW3000_SPI_SUBADDRESS_MSB    0x01U
#define DW3000_SPI_SUBADDRESS_MASK   0x3FU
#define DW3000_SPI_SUBADDRESS_SHIFT  2U
#define DW3000_SPI_MAX_DIRECT_OFFSET 0x7FU
#define DW3000_SPI_MAX_INDIRECT_OFS  0x7FFFU

static inline void dw3000_port_lock(const dw3000_device_t* device) {
    if (device->port.lock != NULL) {
        device->port.lock(device->port.ctx);
    }
}

static inline void dw3000_port_unlock(const dw3000_device_t* device) {
    if (device->port.unlock != NULL) {
        device->port.unlock(device->port.ctx);
    }
}

static uint16_t dw3000_read_le16(const uint8_t* data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t dw3000_read_le32(const uint8_t* data) {
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static void dw3000_write_le16(uint8_t* data, uint16_t value) {
    data[0] = (uint8_t)(value & 0xFFU);
    data[1] = (uint8_t)((value >> 8) & 0xFFU);
}

static void dw3000_write_le32(uint8_t* data, uint32_t value) {
    data[0] = (uint8_t)(value & 0xFFU);
    data[1] = (uint8_t)((value >> 8) & 0xFFU);
    data[2] = (uint8_t)((value >> 16) & 0xFFU);
    data[3] = (uint8_t)((value >> 24) & 0xFFU);
}

static dw3000_error_t dw3000_validate_access(
    const dw3000_device_t* device,
    dw3000_reg_desc_t      reg,
    const void*            data,
    size_t                 data_len,
    bool                   is_write
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (reg.file_id > DW3000_REG_FILE_FINT || reg.offset > DW3000_SPI_MAX_INDIRECT_OFS) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (data == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((reg.length != 0U) && (data_len > reg.length)) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    if (is_write && (device->port.spi_write == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (!is_write && (device->port.spi_read == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if ((reg.offset > DW3000_SPI_MAX_DIRECT_OFFSET) && (device->port.spi_write == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    return DW3000_ERROR_OK;
}

static size_t dw3000_build_direct_header(
    dw3000_reg_desc_t reg,
    bool              is_write,
    uint8_t           header[2]
) {
    uint8_t command = (uint8_t)(reg.file_id << DW3000_SPI_FILE_ID_SHIFT);

    if (is_write) {
        command |= DW3000_SPI_WRITE_BIT;
    }

    if (reg.offset == 0U) {
        header[0] = command;
        return 1U;
    }

    header[0] = (uint8_t)(command |
                          DW3000_SPI_SUBADDRESS_BIT |
                          ((reg.offset >> 6U) & DW3000_SPI_SUBADDRESS_MSB));
    header[1] = (uint8_t)((reg.offset & DW3000_SPI_SUBADDRESS_MASK) << DW3000_SPI_SUBADDRESS_SHIFT);

    return 2U;
}

static dw3000_error_t dw3000_reg_read_direct_locked(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    void*             data,
    size_t            data_len
) {
    uint8_t header[2];
    size_t  header_len = dw3000_build_direct_header(reg, false, header);

    return device->port.spi_read(device->port.ctx, header, header_len, data, data_len);
}

static dw3000_error_t dw3000_reg_write_direct_locked(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    const void*       data,
    size_t            data_len
) {
    uint8_t header[2];
    size_t  header_len = dw3000_build_direct_header(reg, true, header);

    return device->port.spi_write(device->port.ctx, header, header_len, data, data_len);
}

static dw3000_error_t dw3000_select_indirect_pointer_a_locked(
    dw3000_device_t*  device,
    dw3000_reg_desc_t target
) {
    dw3000_error_t err;
    uint8_t        file_id = target.file_id;
    uint8_t        offset[2];

    dw3000_write_le16(offset, target.offset);

    err = dw3000_reg_write_direct_locked(device, DW3000_REG_PTR_ADDR_A, &file_id, sizeof(file_id));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_reg_write_direct_locked(device, DW3000_REG_PTR_OFFSET_A, offset, sizeof(offset));
}

static dw3000_error_t dw3000_reg_read_locked(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    void*             data,
    size_t            data_len
) {
    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (reg.offset <= DW3000_SPI_MAX_DIRECT_OFFSET) {
        return dw3000_reg_read_direct_locked(device, reg, data, data_len);
    }

    dw3000_error_t err = dw3000_select_indirect_pointer_a_locked(device, reg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_reg_read_direct_locked(device, DW3000_REG_INDIRECT_PTR_A, data, data_len);
}

static dw3000_error_t dw3000_reg_write_locked(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    const void*       data,
    size_t            data_len
) {
    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (reg.offset <= DW3000_SPI_MAX_DIRECT_OFFSET) {
        return dw3000_reg_write_direct_locked(device, reg, data, data_len);
    }

    dw3000_error_t err = dw3000_select_indirect_pointer_a_locked(device, reg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_reg_write_direct_locked(device, DW3000_REG_INDIRECT_PTR_A, data, data_len);
}

dw3000_error_t dw3000_reg_read(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    void*             data,
    size_t            data_len
) {
    dw3000_error_t err = dw3000_validate_access(device, reg, data, data_len, false);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_port_lock(device);
    err = dw3000_reg_read_locked(device, reg, data, data_len);
    dw3000_port_unlock(device);

    return err;
}

dw3000_error_t dw3000_reg_write(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    const void*       data,
    size_t            data_len
) {
    dw3000_error_t err = dw3000_validate_access(device, reg, data, data_len, true);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_port_lock(device);
    err = dw3000_reg_write_locked(device, reg, data, data_len);
    dw3000_port_unlock(device);

    return err;
}

dw3000_error_t dw3000_reg_read_u8(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint8_t*          value
) {
    return dw3000_reg_read(device, reg, value, sizeof(*value));
}

dw3000_error_t dw3000_reg_read_u16(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint16_t*         value
) {
    dw3000_error_t err;
    uint8_t        raw[sizeof(*value)];

    if (value == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, reg, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *value = dw3000_read_le16(raw);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_reg_read_u32(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint32_t*         value
) {
    dw3000_error_t err;
    uint8_t        raw[sizeof(*value)];

    if (value == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, reg, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *value = dw3000_read_le32(raw);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_reg_write_u8(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint8_t           value
) {
    return dw3000_reg_write(device, reg, &value, sizeof(value));
}

dw3000_error_t dw3000_reg_write_u16(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint16_t          value
) {
    uint8_t raw[sizeof(value)];

    dw3000_write_le16(raw, value);
    return dw3000_reg_write(device, reg, raw, sizeof(raw));
}

dw3000_error_t dw3000_reg_write_u32(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint32_t          value
) {
    uint8_t raw[sizeof(value)];

    dw3000_write_le32(raw, value);
    return dw3000_reg_write(device, reg, raw, sizeof(raw));
}

dw3000_error_t dw3000_reg_modify_u32(
    dw3000_device_t*  device,
    dw3000_reg_desc_t reg,
    uint32_t          mask,
    uint32_t          value
) {
    dw3000_error_t err = dw3000_validate_access(device, reg, &value, sizeof(value), true);
    uint8_t        raw[sizeof(value)];
    uint32_t       current;

    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (device->port.spi_read == NULL) {
        return DW3000_ERROR_INVALID_STATE;
    }

    dw3000_port_lock(device);

    err = dw3000_reg_read_locked(device, reg, raw, sizeof(raw));
    if (err == DW3000_ERROR_OK) {
        current = dw3000_read_le32(raw);
        current = (current & ~mask) | (value & mask);
        dw3000_write_le32(raw, current);
        err = dw3000_reg_write_locked(device, reg, raw, sizeof(raw));
    }

    dw3000_port_unlock(device);
    return err;
}
