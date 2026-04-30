#include "mock_dw3000_port.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dw3000_port.h"

#define DW3000_MOCK_SPI_WRITE_BIT      0x80U
#define DW3000_MOCK_SPI_SUBADDRESS_BIT 0x40U

static uint64_t dw3000_mock_read_le(const uint8_t* data, size_t data_len) {
    uint64_t value = 0U;

    for (size_t i = 0U; i < data_len; ++i) {
        value |= (uint64_t)data[i] << (8U * i);
    }

    return value;
}

static void dw3000_mock_write_le(uint8_t* data, size_t data_len, uint64_t value) {
    for (size_t i = 0U; i < data_len; ++i) {
        data[i] = (uint8_t)((value >> (8U * i)) & 0xFFU);
    }
}

static bool dw3000_mock_decode_header(
    const uint8_t* header,
    size_t         header_len,
    bool*          is_write,
    uint8_t*       file_id,
    uint16_t*      offset
) {
    if ((header == NULL) ||
        (is_write == NULL) ||
        (file_id == NULL) ||
        (offset == NULL) ||
        ((header_len != 1U) && (header_len != 2U))) {
        return false;
    }

    *is_write = (header[0] & DW3000_MOCK_SPI_WRITE_BIT) != 0U;
    *file_id  = (uint8_t)((header[0] & 0x3EU) >> 1U);

    if ((header[0] & DW3000_MOCK_SPI_SUBADDRESS_BIT) == 0U) {
        *offset = 0U;
        return header_len == 1U;
    }

    if (header_len != 2U) {
        return false;
    }

    *offset = (uint16_t)(((uint16_t)(header[0] & 0x01U) << 6U) |
                         ((uint16_t)header[1] >> 2U));
    return true;
}

static dw3000_mock_reg_t* dw3000_mock_find_reg(
    dw3000_mock_port_t* mock,
    uint8_t             file_id,
    uint16_t            offset
) {
    if (mock == NULL) {
        return NULL;
    }

    for (size_t i = 0U; i < DW3000_MOCK_PORT_MAX_REGS; ++i) {
        if (mock->regs[i].used &&
            (mock->regs[i].file_id == file_id) &&
            (mock->regs[i].offset == offset)) {
            return &mock->regs[i];
        }
    }

    return NULL;
}

static const dw3000_mock_reg_t* dw3000_mock_find_const_reg(
    const dw3000_mock_port_t* mock,
    uint8_t                   file_id,
    uint16_t                  offset
) {
    return dw3000_mock_find_reg((dw3000_mock_port_t*)mock, file_id, offset);
}

static dw3000_error_t dw3000_mock_spi_read(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    void*          data,
    size_t         data_len
) {
    dw3000_mock_port_t* mock = (dw3000_mock_port_t*)ctx;
    dw3000_mock_reg_t*  reg;
    bool                is_write;
    uint8_t             file_id;
    uint16_t            offset;

    if ((mock == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (mock->next_read_error != DW3000_ERROR_OK) {
        dw3000_error_t error  = mock->next_read_error;
        mock->next_read_error = DW3000_ERROR_OK;
        return error;
    }

    if (!dw3000_mock_decode_header(header, header_len, &is_write, &file_id, &offset) ||
        is_write) {
        return DW3000_ERROR_INVALID_ARG;
    }

    reg = dw3000_mock_find_reg(mock, file_id, offset);
    if ((reg == NULL) || (data_len > reg->len)) {
        return DW3000_ERROR_IO;
    }

    mock->read_count++;
    mock->last_read_file_id = file_id;
    mock->last_read_offset  = offset;
    mock->last_read_len     = data_len;

    if (data_len != 0U) {
        memcpy(data, reg->data, data_len);
    }

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_mock_spi_write(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    const void*    data,
    size_t         data_len
) {
    dw3000_mock_port_t* mock = (dw3000_mock_port_t*)ctx;
    dw3000_mock_reg_t*  reg;
    bool                is_write;
    uint8_t             file_id;
    uint16_t            offset;

    if ((mock == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (mock->next_write_error != DW3000_ERROR_OK) {
        dw3000_error_t error   = mock->next_write_error;
        mock->next_write_error = DW3000_ERROR_OK;
        return error;
    }

    if ((header_len == 1U) &&
        (data_len == 0U) &&
        ((header[0] & 0x81U) == 0x81U) &&
        ((header[0] & 0x40U) == 0U)) {
        mock->fast_command_count++;
        mock->last_fast_command_header = header[0];
        mock->last_fast_command        = (uint8_t)((header[0] & 0x3EU) >> 1U);
        return DW3000_ERROR_OK;
    }

    if (!dw3000_mock_decode_header(header, header_len, &is_write, &file_id, &offset) ||
        !is_write ||
        (data_len > DW3000_MOCK_PORT_MAX_REG_LEN)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    reg = dw3000_mock_find_reg(mock, file_id, offset);
    if ((reg == NULL) || (data_len > reg->len)) {
        return DW3000_ERROR_IO;
    }

    mock->write_count++;
    mock->last_write_file_id = file_id;
    mock->last_write_offset  = offset;
    mock->last_write_len     = data_len;
    if (data_len != 0U) {
        memcpy(mock->last_write_data, data, data_len);
    }

    if (reg->write_one_to_clear) {
        const uint8_t* raw = (const uint8_t*)data;

        for (size_t i = 0U; i < data_len; ++i) {
            reg->data[i] = (uint8_t)(reg->data[i] & (uint8_t)~raw[i]);
        }
    } else if (data_len != 0U) {
        memcpy(reg->data, data, data_len);
    }

    return DW3000_ERROR_OK;
}

static void dw3000_mock_lock(void* ctx) {
    dw3000_mock_port_t* mock = (dw3000_mock_port_t*)ctx;

    if (mock != NULL) {
        mock->lock_count++;
    }
}

static void dw3000_mock_unlock(void* ctx) {
    dw3000_mock_port_t* mock = (dw3000_mock_port_t*)ctx;

    if (mock != NULL) {
        mock->unlock_count++;
    }
}

void dw3000_mock_port_init(
    dw3000_mock_port_t* mock,
    dw3000_device_t*    device
) {
    if ((mock == NULL) || (device == NULL)) {
        return;
    }

    memset(mock, 0, sizeof(*mock));
    memset(device, 0, sizeof(*device));

    mock->next_read_error  = DW3000_ERROR_OK;
    mock->next_write_error = DW3000_ERROR_OK;

    device->port.ctx       = mock;
    device->port.spi_read  = dw3000_mock_spi_read;
    device->port.spi_write = dw3000_mock_spi_write;
    device->port.lock      = dw3000_mock_lock;
    device->port.unlock    = dw3000_mock_unlock;

    (void)dw3000_mock_port_define_reg(
        mock,
        DW3000_REG_SYS_STATUS,
        NULL,
        DW3000_REG_SYS_STATUS.length,
        true
    );
    (void)dw3000_mock_port_define_reg(
        mock,
        DW3000_REG_SYS_ENABLE,
        NULL,
        DW3000_REG_SYS_ENABLE.length,
        false
    );
}

dw3000_error_t dw3000_mock_port_define_reg(
    dw3000_mock_port_t* mock,
    dw3000_reg_desc_t   reg,
    const void*         data,
    size_t              data_len,
    bool                write_one_to_clear
) {
    dw3000_mock_reg_t* slot = NULL;

    if ((mock == NULL) || (data_len > DW3000_MOCK_PORT_MAX_REG_LEN)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    slot = dw3000_mock_find_reg(mock, reg.file_id, reg.offset);
    if (slot == NULL) {
        for (size_t i = 0U; i < DW3000_MOCK_PORT_MAX_REGS; ++i) {
            if (!mock->regs[i].used) {
                slot = &mock->regs[i];
                break;
            }
        }
    }

    if (slot == NULL) {
        return DW3000_ERROR_NO_MEMORY;
    }

    memset(slot, 0, sizeof(*slot));
    slot->used               = true;
    slot->write_one_to_clear = write_one_to_clear;
    slot->file_id            = reg.file_id;
    slot->offset             = reg.offset;
    slot->len                = data_len;
    if (data != NULL) {
        memcpy(slot->data, data, data_len);
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_mock_port_set_u48(
    dw3000_mock_port_t* mock,
    dw3000_reg_desc_t   reg,
    uint64_t            value
) {
    dw3000_mock_reg_t* slot;

    if (mock == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    slot = dw3000_mock_find_reg(mock, reg.file_id, reg.offset);
    if ((slot == NULL) || (slot->len < 6U)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    dw3000_mock_write_le(slot->data, 6U, value);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_mock_port_get_u48(
    const dw3000_mock_port_t* mock,
    dw3000_reg_desc_t         reg,
    uint64_t*                 value
) {
    const dw3000_mock_reg_t* slot;

    if ((mock == NULL) || (value == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    slot = dw3000_mock_find_const_reg(mock, reg.file_id, reg.offset);
    if ((slot == NULL) || (slot->len < 6U)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    *value = dw3000_mock_read_le(slot->data, 6U);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_mock_port_get_reg_data(
    const dw3000_mock_port_t* mock,
    dw3000_reg_desc_t         reg,
    void*                     data,
    size_t                    data_len
) {
    const dw3000_mock_reg_t* slot;

    if ((mock == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    slot = dw3000_mock_find_const_reg(mock, reg.file_id, reg.offset);
    if ((slot == NULL) || (data_len > slot->len)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (data_len != 0U) {
        memcpy(data, slot->data, data_len);
    }

    return DW3000_ERROR_OK;
}

uint64_t dw3000_mock_port_last_write_u48(const dw3000_mock_port_t* mock) {
    if ((mock == NULL) || (mock->last_write_len < 6U)) {
        return 0U;
    }

    return dw3000_mock_read_le(mock->last_write_data, 6U);
}

void dw3000_mock_port_set_next_read_error(
    dw3000_mock_port_t* mock,
    dw3000_error_t      error
) {
    if (mock != NULL) {
        mock->next_read_error = error;
    }
}

void dw3000_mock_port_set_next_write_error(
    dw3000_mock_port_t* mock,
    dw3000_error_t      error
) {
    if (mock != NULL) {
        mock->next_write_error = error;
    }
}
