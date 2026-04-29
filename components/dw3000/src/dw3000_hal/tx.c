#include "dw3000_hal/tx.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_hal/fcmd.h"
#include "dw3000_register.h"

#define DW3000_HAL_REG_TX_FCTRL_HI \
    DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_0, 0x0028U, 2U)

#define DW3000_HAL_TX_FCTRL_TXFLEN_MASK      0x000003FFUL
#define DW3000_HAL_TX_FCTRL_TR_BIT           (1UL << 11U)
#define DW3000_HAL_TX_FCTRL_TXB_OFFSET_SHIFT 16U
#define DW3000_HAL_TX_FCTRL_TXB_OFFSET_MASK  (0x03FFUL << DW3000_HAL_TX_FCTRL_TXB_OFFSET_SHIFT)
#define DW3000_HAL_TX_FCTRL_FRAME_MASK       (DW3000_HAL_TX_FCTRL_TXFLEN_MASK | \
                                               DW3000_HAL_TX_FCTRL_TR_BIT | \
                                               DW3000_HAL_TX_FCTRL_TXB_OFFSET_MASK)

#define DW3000_HAL_TX_FCTRL_FINE_PLEN_SHIFT 8U
#define DW3000_HAL_TX_OFFSET_ERRATA_LIMIT   127U
#define DW3000_HAL_TX_OFFSET_ERRATA_ADJUST  128U
#define DW3000_HAL_TX_OFFSET_FIELD_MASK     0x03FFU

static bool dw3000_hal_tx_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static uint16_t dw3000_hal_tx_max_frame_len(const dw3000_device_t* device) {
    if (device->config.phy.phr_mode == DW3000_PHY_PHR_MODE_EXTENDED) {
        return DW3000_HAL_TX_MAX_FRAME_EXTENDED;
    }

    return DW3000_HAL_TX_MAX_FRAME_STANDARD;
}

static bool dw3000_hal_tx_encode_offset(
    uint16_t  offset,
    uint16_t* encoded_offset
) {
    uint32_t encoded = offset;

    if (offset > DW3000_HAL_TX_OFFSET_ERRATA_LIMIT) {
        encoded += DW3000_HAL_TX_OFFSET_ERRATA_ADJUST;
    }

    if (encoded > DW3000_HAL_TX_OFFSET_FIELD_MASK) {
        return false;
    }

    *encoded_offset = (uint16_t)encoded;
    return true;
}

static uint32_t dw3000_hal_tx_frame_bits(
    const dw3000_txrx_tx_frame_t* frame,
    uint16_t                      encoded_offset
) {
    uint32_t value = ((uint32_t)frame->tx_flen & DW3000_HAL_TX_FCTRL_TXFLEN_MASK) |
                     ((uint32_t)encoded_offset << DW3000_HAL_TX_FCTRL_TXB_OFFSET_SHIFT);

    if (frame->ranging) {
        value |= DW3000_HAL_TX_FCTRL_TR_BIT;
    }

    return value;
}

static dw3000_error_t dw3000_hal_tx_write_fine_plen(
    dw3000_device_t*              device,
    const dw3000_txrx_tx_frame_t* frame
) {
    dw3000_error_t err;
    uint16_t       value;

    err = dw3000_reg_read_u16(device, DW3000_HAL_REG_TX_FCTRL_HI, &value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    value = (uint16_t)((value & 0x00FFU) |
                       ((uint16_t)frame->fine_plen << DW3000_HAL_TX_FCTRL_FINE_PLEN_SHIFT));

    return dw3000_reg_write_u16(device, DW3000_HAL_REG_TX_FCTRL_HI, value);
}

static dw3000_error_t dw3000_hal_tx_apply_offset_errata(
    dw3000_device_t*              device,
    const dw3000_txrx_tx_frame_t* frame
) {
    uint8_t dummy;

    if (frame->tx_b_offset <= DW3000_HAL_TX_OFFSET_ERRATA_LIMIT) {
        return DW3000_ERROR_OK;
    }

    return dw3000_reg_read_u8(device, DW3000_REG_SAR_CTRL, &dummy);
}

dw3000_error_t dw3000_hal_tx_validate_frame(
    const dw3000_device_t*        device,
    const dw3000_txrx_tx_frame_t* frame
) {
    uint16_t encoded_offset;

    if ((device == NULL) || (frame == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (frame->tx_flen > dw3000_hal_tx_max_frame_len(device)) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    if (((size_t)frame->tx_b_offset + (size_t)frame->tx_flen) >
        DW3000_HAL_TX_BUFFER_SIZE) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    if (!dw3000_hal_tx_encode_offset(frame->tx_b_offset, &encoded_offset)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_write_buffer(
    dw3000_device_t* device,
    uint16_t         offset,
    const void*      data,
    size_t           data_len
) {
    dw3000_reg_desc_t reg = DW3000_REG_TX_BUFFER;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (data == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((offset >= DW3000_HAL_TX_BUFFER_SIZE) ||
        (data_len > (DW3000_HAL_TX_BUFFER_SIZE - (size_t)offset))) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    reg.offset = offset;
    reg.length = DW3000_HAL_TX_BUFFER_SIZE - (size_t)offset;

    return dw3000_reg_write(device, reg, data, data_len);
}

dw3000_error_t dw3000_hal_tx_configure_frame(
    dw3000_device_t*              device,
    const dw3000_txrx_tx_frame_t* frame
) {
    dw3000_error_t err;
    uint16_t       encoded_offset;
    uint32_t       value;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_tx_validate_frame(device, frame);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    (void)dw3000_hal_tx_encode_offset(frame->tx_b_offset, &encoded_offset);
    value = dw3000_hal_tx_frame_bits(frame, encoded_offset);

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_TX_FCTRL,
        DW3000_HAL_TX_FCTRL_FRAME_MASK,
        value
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_tx_write_fine_plen(device, frame);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_tx_apply_offset_errata(device, frame);
}

dw3000_error_t dw3000_hal_tx_prepare_frame(
    dw3000_device_t*              device,
    const void*                   data,
    size_t                        data_len,
    const dw3000_txrx_tx_frame_t* frame
) {
    dw3000_error_t err;

    if ((device == NULL) || (frame == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (data_len > frame->tx_flen) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    err = dw3000_hal_tx_write_buffer(device, frame->tx_b_offset, data, data_len);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_tx_configure_frame(device, frame);
}

dw3000_error_t dw3000_hal_tx_set_delayed_time(
    dw3000_device_t*           device,
    dw3000_txrx_delayed_time_t time
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_DX_TIME, time);
}

dw3000_error_t dw3000_hal_tx_set_reference_time(
    dw3000_device_t*           device,
    dw3000_txrx_delayed_time_t time
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_DREF_TIME, time);
}

dw3000_error_t dw3000_hal_tx_set_antenna_delay(
    dw3000_device_t*           device,
    dw3000_cia_antenna_delay_t delay
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u16(device, DW3000_REG_TX_ANTD, delay);
}

dw3000_error_t dw3000_hal_tx_read_timestamp(
    dw3000_device_t*         device,
    dw3000_txrx_timestamp_t* timestamp
) {
    dw3000_error_t err;
    uint8_t        raw[5];

    if ((device == NULL) || (timestamp == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_TX_TIME, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *timestamp = (dw3000_txrx_timestamp_t)raw[0] |
                 ((dw3000_txrx_timestamp_t)raw[1] << 8U) |
                 ((dw3000_txrx_timestamp_t)raw[2] << 16U) |
                 ((dw3000_txrx_timestamp_t)raw[3] << 24U) |
                 ((dw3000_txrx_timestamp_t)raw[4] << 32U);

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_tx_read_raw_timestamp(
    dw3000_device_t*            device,
    dw3000_txrx_delayed_time_t* timestamp
) {
    return dw3000_reg_read_u32(device, DW3000_REG_TX_RAWST, timestamp);
}

bool dw3000_hal_tx_is_start_command(dw3000_fcmd_t command) {
    return (command == DW3000_FCMD_TX) ||
           (command == DW3000_FCMD_DTX) ||
           (command == DW3000_FCMD_DTX_TS) ||
           (command == DW3000_FCMD_DTX_RS) ||
           (command == DW3000_FCMD_DTX_REF) ||
           (command == DW3000_FCMD_CCA_TX) ||
           (command == DW3000_FCMD_TX_W4R) ||
           (command == DW3000_FCMD_DTX_W4R) ||
           (command == DW3000_FCMD_DTX_TS_W4R) ||
           (command == DW3000_FCMD_DTX_RS_W4R) ||
           (command == DW3000_FCMD_DTX_REF_W4R) ||
           (command == DW3000_FCMD_CCA_TX_W4R);
}

dw3000_error_t dw3000_hal_tx_start(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_start_command(command)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_tx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_hal_fcmd_issue(device, command);
}

dw3000_error_t dw3000_hal_tx_start_immediate(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_TX);
}

dw3000_error_t dw3000_hal_tx_start_immediate_w4r(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_TX_W4R);
}

dw3000_error_t dw3000_hal_tx_start_delayed(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX);
}

dw3000_error_t dw3000_hal_tx_start_delayed_w4r(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_W4R);
}

dw3000_error_t dw3000_hal_tx_start_delayed_ts(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_TS);
}

dw3000_error_t dw3000_hal_tx_start_delayed_ts_w4r(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_TS_W4R);
}

dw3000_error_t dw3000_hal_tx_start_delayed_rs(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_RS);
}

dw3000_error_t dw3000_hal_tx_start_delayed_rs_w4r(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_RS_W4R);
}

dw3000_error_t dw3000_hal_tx_start_delayed_ref(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_REF);
}

dw3000_error_t dw3000_hal_tx_start_delayed_ref_w4r(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_DTX_REF_W4R);
}

dw3000_error_t dw3000_hal_tx_start_cca(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_CCA_TX);
}

dw3000_error_t dw3000_hal_tx_start_cca_w4r(dw3000_device_t* device) {
    return dw3000_hal_tx_start(device, DW3000_FCMD_CCA_TX_W4R);
}
