#include "dw3000_hal/rx.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/status.h"
#include "dw3000_register.h"

#define DW3000_HAL_RX_FWTO_MAX 0x000FFFFFUL

#define DW3000_HAL_RX_SYS_CFG_MASK \
    (DW3000_TXRX_SYS_CFG_DIS_FCS_TX | \
     DW3000_TXRX_SYS_CFG_DIS_FCE |    \
     DW3000_TXRX_SYS_CFG_DIS_DRXB |   \
     DW3000_TXRX_SYS_CFG_RXWTOE |     \
     DW3000_TXRX_SYS_CFG_RXAUTR)

#define DW3000_HAL_RX_FINFO_RXFLEN_MASK 0x000003FFUL
#define DW3000_HAL_RX_FINFO_RXNSPL_SHIFT 11U
#define DW3000_HAL_RX_FINFO_RXBR_BIT (1UL << 13U)
#define DW3000_HAL_RX_FINFO_RNG_BIT (1UL << 15U)
#define DW3000_HAL_RX_FINFO_RXPRF_SHIFT 16U
#define DW3000_HAL_RX_FINFO_RXPSR_SHIFT 18U
#define DW3000_HAL_RX_FINFO_RXPACC_SHIFT 20U

#define DW3000_HAL_RX_RDB_STATUS_MASK 0xFFU
#define DW3000_HAL_RX_RDB_DMODE_MASK  0x07U

#define DW3000_HAL_RX_SNIFF_ON_MASK       0x0000000FUL
#define DW3000_HAL_RX_SNIFF_OFF_SHIFT     8U
#define DW3000_HAL_RX_SNIFF_OFF_MASK      0x0000FF00UL
#define DW3000_HAL_RX_SNIFF_MASK          (DW3000_HAL_RX_SNIFF_ON_MASK | \
                                           DW3000_HAL_RX_SNIFF_OFF_MASK)
#define DW3000_HAL_RX_SNIFF_MIN_ON        2U
#define DW3000_HAL_RX_SNIFF_MIN_OFF_US    5U

#define DW3000_HAL_REG_RX_STAMP_LO \
    DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_0, 0x0064U, 4U)
#define DW3000_HAL_REG_RX_STAMP_HI \
    DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_0, 0x0068U, 1U)
#define DW3000_HAL_REG_RX_RAWST \
    DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_0, 0x0070U, 4U)

static bool dw3000_hal_rx_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_rx_mark_active(dw3000_device_t* device) {
    device->state_flags = (dw3000_device_state_flags_t)(
        (device->state_flags | DW3000_DEVICE_STATE_RX_ON) &
        ~(DW3000_DEVICE_STATE_IDLE_RC |
          DW3000_DEVICE_STATE_IDLE_PLL |
          DW3000_DEVICE_STATE_TX_PENDING |
          DW3000_DEVICE_STATE_SLEEPING)
    );
}

static dw3000_error_t dw3000_hal_rx_write_sys_cfg(
    dw3000_device_t*            device,
    dw3000_txrx_sys_cfg_flags_t flags
) {
    dw3000_error_t err;
    uint32_t       current;
    uint32_t       value;
    uint32_t       flag_value = (uint32_t)flags & DW3000_HAL_RX_SYS_CFG_MASK;

    if (((flag_value & DW3000_TXRX_SYS_CFG_RXAUTR) != 0U) &&
        ((flag_value & DW3000_TXRX_SYS_CFG_DIS_DRXB) == 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->sys_cfg_cache_valid) {
        current = device->sys_cfg_cache;
    } else {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_CFG, &current);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    value = (current & ~DW3000_HAL_RX_SYS_CFG_MASK) | flag_value;

    err = dw3000_reg_write_u32(device, DW3000_REG_SYS_CFG, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->sys_cfg_cache       = value;
    device->sys_cfg_cache_valid = true;
    device->config.use_double_buffer =
        (flag_value & DW3000_TXRX_SYS_CFG_DIS_DRXB) == 0U;

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_rx_decode_finfo(
    uint32_t                 raw,
    dw3000_txrx_rx_finfo_t*  finfo
) {
    uint8_t prf = (uint8_t)((raw >> DW3000_HAL_RX_FINFO_RXPRF_SHIFT) & 0x3U);

    if (prf > (uint8_t)DW3000_PHY_PRF_64_MHZ) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    finfo->rx_flen   = (uint16_t)(raw & DW3000_HAL_RX_FINFO_RXFLEN_MASK);
    finfo->ranging   = (raw & DW3000_HAL_RX_FINFO_RNG_BIT) != 0U;
    finfo->data_rate = ((raw & DW3000_HAL_RX_FINFO_RXBR_BIT) != 0U) ?
        DW3000_PHY_DATA_RATE_6M81 :
        DW3000_PHY_DATA_RATE_850K;
    finfo->prf       = (dw3000_phy_prf_t)prf;
    finfo->rx_psr    = (uint8_t)((raw >> DW3000_HAL_RX_FINFO_RXPSR_SHIFT) & 0x3U);
    finfo->rx_nspl   = (uint8_t)((raw >> DW3000_HAL_RX_FINFO_RXNSPL_SHIFT) & 0x3U);
    finfo->rx_pacc   = (uint16_t)((raw >> DW3000_HAL_RX_FINFO_RXPACC_SHIFT) & 0x0FFFU);

    return DW3000_ERROR_OK;
}

static dw3000_reg_desc_t dw3000_hal_rx_buffer_reg(uint8_t buffer_index) {
    return (buffer_index == 0U) ? DW3000_REG_RX_BUFFER_0 : DW3000_REG_RX_BUFFER_1;
}

static bool dw3000_hal_rx_is_valid_rdb_mode(dw3000_txrx_rdb_dmode_t mode) {
    return (mode == DW3000_TXRX_RDB_DMODE_MINIMAL) ||
           (mode == DW3000_TXRX_RDB_DMODE_MEDIUM) ||
           (mode == DW3000_TXRX_RDB_DMODE_FULL);
}

static bool dw3000_hal_rx_sniff_is_disabled(const dw3000_txrx_sniff_t* sniff) {
    return (sniff->on == 0U) && (sniff->off == 0U);
}

static uint32_t dw3000_hal_rx_sniff_bits(const dw3000_txrx_sniff_t* sniff) {
    return ((uint32_t)sniff->on & DW3000_HAL_RX_SNIFF_ON_MASK) |
           ((uint32_t)sniff->off << DW3000_HAL_RX_SNIFF_OFF_SHIFT);
}

dw3000_txrx_event_t dw3000_hal_rx_success_events(void) {
    return DW3000_TXRX_EVENT_RXFR | DW3000_TXRX_EVENT_RXFCG;
}

dw3000_txrx_event_t dw3000_hal_rx_error_events(void) {
    return DW3000_TXRX_EVENT_RXPHE |
           DW3000_TXRX_EVENT_RXFCE |
           DW3000_TXRX_EVENT_RXFSL |
           DW3000_TXRX_EVENT_RXFTO |
           DW3000_TXRX_EVENT_CIAERR |
           DW3000_TXRX_EVENT_RXOVRR |
           DW3000_TXRX_EVENT_RXPTO |
           DW3000_TXRX_EVENT_RXSTO |
           DW3000_TXRX_EVENT_CPERR |
           DW3000_TXRX_EVENT_ARFE |
           DW3000_TXRX_EVENT_RXPREJ;
}

dw3000_txrx_event_t dw3000_hal_rx_all_events(void) {
    return DW3000_TXRX_EVENT_RXPRD |
           DW3000_TXRX_EVENT_RXSFDD |
           DW3000_TXRX_EVENT_CIADONE |
           DW3000_TXRX_EVENT_RXPHD |
           dw3000_hal_rx_success_events() |
           dw3000_hal_rx_error_events();
}

dw3000_error_t dw3000_hal_rx_configure_sys_cfg(
    dw3000_device_t*            device,
    dw3000_txrx_sys_cfg_flags_t flags
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_hal_rx_write_sys_cfg(device, flags);
}

dw3000_error_t dw3000_hal_rx_configure_default(dw3000_device_t* device) {
    dw3000_txrx_sys_cfg_flags_t flags = 0;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!device->config.use_double_buffer) {
        flags = (dw3000_txrx_sys_cfg_flags_t)(flags | DW3000_TXRX_SYS_CFG_DIS_DRXB);
    }

    return dw3000_hal_rx_configure_sys_cfg(device, flags);
}

dw3000_error_t dw3000_hal_rx_set_frame_wait_timeout(
    dw3000_device_t*   device,
    dw3000_txrx_fwto_t timeout
) {
    uint8_t raw[3];

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (timeout > DW3000_HAL_RX_FWTO_MAX) {
        return DW3000_ERROR_INVALID_ARG;
    }

    raw[0] = (uint8_t)(timeout & 0xFFU);
    raw[1] = (uint8_t)((timeout >> 8U) & 0xFFU);
    raw[2] = (uint8_t)((timeout >> 16U) & 0x0FU);

    return dw3000_reg_write(device, DW3000_REG_RX_FWTO, raw, sizeof(raw));
}

dw3000_error_t dw3000_hal_rx_set_delayed_time(
    dw3000_device_t*           device,
    dw3000_txrx_delayed_time_t time
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_DX_TIME, time);
}

dw3000_error_t dw3000_hal_rx_set_reference_time(
    dw3000_device_t*           device,
    dw3000_txrx_delayed_time_t time
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_DREF_TIME, time);
}

dw3000_error_t dw3000_hal_rx_read_finfo(
    dw3000_device_t*         device,
    dw3000_txrx_rx_finfo_t*  finfo
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (finfo == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_RX_FINFO, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_rx_decode_finfo(raw, finfo);
}

dw3000_error_t dw3000_hal_rx_read_buffer(
    dw3000_device_t* device,
    uint8_t          buffer_index,
    uint16_t         offset,
    void*            data,
    size_t           data_len
) {
    dw3000_reg_desc_t reg;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((buffer_index > 1U) ||
        ((buffer_index == 1U) && !device->config.use_double_buffer)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (data == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((offset >= DW3000_HAL_RX_BUFFER_SIZE) ||
        (data_len > (DW3000_HAL_RX_BUFFER_SIZE - (size_t)offset))) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    reg = dw3000_hal_rx_buffer_reg(buffer_index);
    reg.offset = offset;
    reg.length = DW3000_HAL_RX_BUFFER_SIZE - (size_t)offset;

    return dw3000_reg_read(device, reg, data, data_len);
}

dw3000_error_t dw3000_hal_rx_read_active_buffer(
    dw3000_device_t* device,
    uint16_t         offset,
    void*            data,
    size_t           data_len
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_rx_read_buffer(
        device,
        device->config.use_double_buffer ? device->active_rx_buffer : 0U,
        offset,
        data,
        data_len
    );
}

dw3000_error_t dw3000_hal_rx_read_timestamp(
    dw3000_device_t*         device,
    dw3000_txrx_timestamp_t* timestamp
) {
    dw3000_error_t err;
    uint32_t       low;
    uint8_t        high;

    if ((device == NULL) || (timestamp == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_HAL_REG_RX_STAMP_LO, &low);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u8(device, DW3000_HAL_REG_RX_STAMP_HI, &high);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *timestamp = (dw3000_txrx_timestamp_t)low |
                 ((dw3000_txrx_timestamp_t)high << 32U);

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_rx_read_raw_timestamp(
    dw3000_device_t*            device,
    dw3000_txrx_delayed_time_t* timestamp
) {
    if ((device == NULL) || (timestamp == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u32(device, DW3000_HAL_REG_RX_RAWST, timestamp);
}

dw3000_error_t dw3000_hal_rx_read_double_buffer_status(
    dw3000_device_t*         device,
    dw3000_txrx_rdb_status_t* status
) {
    uint8_t raw;

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!device->config.use_double_buffer) {
        return DW3000_ERROR_INVALID_STATE;
    }

    dw3000_error_t err = dw3000_reg_read_u8(device, DW3000_REG_RDB_STATUS, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *status = (dw3000_txrx_rdb_status_t)(raw & DW3000_HAL_RX_RDB_STATUS_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_rx_clear_double_buffer_status(
    dw3000_device_t*        device,
    dw3000_txrx_rdb_status_t status
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!device->config.use_double_buffer) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (((uint8_t)status & DW3000_HAL_RX_RDB_STATUS_MASK) == 0U) {
        return DW3000_ERROR_OK;
    }

    return dw3000_reg_write_u8(
        device,
        DW3000_REG_RDB_STATUS,
        (uint8_t)status & DW3000_HAL_RX_RDB_STATUS_MASK
    );
}

dw3000_error_t dw3000_hal_rx_read_double_buffer_diag_mode(
    dw3000_device_t*        device,
    dw3000_txrx_rdb_dmode_t* mode
) {
    uint8_t raw;

    if ((device == NULL) || (mode == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!device->config.use_double_buffer) {
        return DW3000_ERROR_INVALID_STATE;
    }

    dw3000_error_t err = dw3000_reg_read_u8(device, DW3000_REG_RDB_DIAG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *mode = (dw3000_txrx_rdb_dmode_t)(raw & DW3000_HAL_RX_RDB_DMODE_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_rx_set_double_buffer_diag_mode(
    dw3000_device_t*       device,
    dw3000_txrx_rdb_dmode_t mode
) {
    dw3000_error_t err;
    uint8_t        current;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!device->config.use_double_buffer) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (!dw3000_hal_rx_is_valid_rdb_mode(mode)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u8(device, DW3000_REG_RDB_DIAG, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    current = (uint8_t)((current & ~DW3000_HAL_RX_RDB_DMODE_MASK) |
                        ((uint8_t)mode & DW3000_HAL_RX_RDB_DMODE_MASK));

    return dw3000_reg_write_u8(device, DW3000_REG_RDB_DIAG, current);
}

dw3000_error_t dw3000_hal_rx_read_sniff(
    dw3000_device_t*     device,
    dw3000_txrx_sniff_t* sniff
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (sniff == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_RX_SNIFF, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    sniff->on  = (uint8_t)(raw & DW3000_HAL_RX_SNIFF_ON_MASK);
    sniff->off = (uint8_t)((raw & DW3000_HAL_RX_SNIFF_OFF_MASK) >>
                           DW3000_HAL_RX_SNIFF_OFF_SHIFT);

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_rx_configure_sniff(
    dw3000_device_t*          device,
    const dw3000_txrx_sniff_t* sniff
) {
    if ((device == NULL) || (sniff == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (!dw3000_hal_rx_sniff_is_disabled(sniff) &&
        ((sniff->on < DW3000_HAL_RX_SNIFF_MIN_ON) ||
         (sniff->on > DW3000_HAL_RX_SNIFF_ON_MASK) ||
         (sniff->off < DW3000_HAL_RX_SNIFF_MIN_OFF_US))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_modify_u32(
        device,
        DW3000_REG_RX_SNIFF,
        DW3000_HAL_RX_SNIFF_MASK,
        dw3000_hal_rx_sniff_bits(sniff)
    );
}

dw3000_error_t dw3000_hal_rx_disable_sniff(dw3000_device_t* device) {
    const dw3000_txrx_sniff_t sniff = {0};

    return dw3000_hal_rx_configure_sniff(device, &sniff);
}

dw3000_error_t dw3000_hal_rx_clear_events(
    dw3000_device_t*     device,
    dw3000_txrx_event_t  events
) {
    return dw3000_hal_status_clear(device, events & dw3000_hal_rx_all_events());
}

bool dw3000_hal_rx_is_start_command(dw3000_fcmd_t command) {
    return (command == DW3000_FCMD_RX) ||
           (command == DW3000_FCMD_DRX) ||
           (command == DW3000_FCMD_DRX_TS) ||
           (command == DW3000_FCMD_DRX_RS) ||
           (command == DW3000_FCMD_DRX_REF);
}

dw3000_error_t dw3000_hal_rx_start(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_start_command(command)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_rx_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_fcmd_issue(device, command);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_rx_mark_active(device);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_rx_start_immediate(dw3000_device_t* device) {
    return dw3000_hal_rx_start(device, DW3000_FCMD_RX);
}

dw3000_error_t dw3000_hal_rx_start_delayed(dw3000_device_t* device) {
    return dw3000_hal_rx_start(device, DW3000_FCMD_DRX);
}

dw3000_error_t dw3000_hal_rx_start_delayed_ts(dw3000_device_t* device) {
    return dw3000_hal_rx_start(device, DW3000_FCMD_DRX_TS);
}

dw3000_error_t dw3000_hal_rx_start_delayed_rs(dw3000_device_t* device) {
    return dw3000_hal_rx_start(device, DW3000_FCMD_DRX_RS);
}

dw3000_error_t dw3000_hal_rx_start_delayed_ref(dw3000_device_t* device) {
    return dw3000_hal_rx_start(device, DW3000_FCMD_DRX_REF);
}
