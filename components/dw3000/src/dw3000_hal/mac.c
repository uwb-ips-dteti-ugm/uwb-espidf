#include "dw3000_hal/mac.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_MAC_FF_CFG_MASK 0xFFFFUL

#define DW3000_HAL_MAC_SYS_CFG_MASK \
    (DW3000_MAC_SYS_CFG_FF_ENABLE | \
     DW3000_MAC_SYS_CFG_AUTO_ACK |  \
     DW3000_MAC_SYS_CFG_FAST_AAT)

#define DW3000_HAL_MAC_ACK_RESP_W4R_MASK      0x000FFFFFUL
#define DW3000_HAL_MAC_ACK_RESP_ACK_TIM_SHIFT 24U
#define DW3000_HAL_MAC_ACK_RESP_ACK_TIM_MASK  0xFF000000UL
#define DW3000_HAL_MAC_ACK_RESP_MASK          (DW3000_HAL_MAC_ACK_RESP_W4R_MASK | DW3000_HAL_MAC_ACK_RESP_ACK_TIM_MASK)

static bool dw3000_hal_mac_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static uint64_t dw3000_hal_mac_unpack_u64(const uint8_t raw[8]) {
    uint64_t value = 0U;

    for (uint8_t i = 0U; i < 8U; ++i) {
        value |= (uint64_t)raw[i] << (8U * i);
    }

    return value;
}

static void dw3000_hal_mac_pack_u64(
    uint64_t value,
    uint8_t  raw[8]
) {
    for (uint8_t i = 0U; i < 8U; ++i) {
        raw[i] = (uint8_t)((value >> (8U * i)) & 0xFFU);
    }
}

static uint32_t dw3000_hal_mac_build_panadr(const dw3000_mac_panadr_t* panadr) {
    return (uint32_t)panadr->short_addr |
           ((uint32_t)panadr->pan_id << 16U);
}

static void dw3000_hal_mac_decode_panadr(
    uint32_t             raw,
    dw3000_mac_panadr_t* panadr
) {
    panadr->short_addr = (dw3000_mac_short_addr_t)(raw & 0xFFFFU);
    panadr->pan_id     = (dw3000_mac_pan_id_t)((raw >> 16U) & 0xFFFFU);
}

static uint32_t dw3000_hal_mac_build_ack_response(
    const dw3000_mac_ack_resp_t* ack_resp
) {
    return (ack_resp->w4r_tim & DW3000_HAL_MAC_ACK_RESP_W4R_MASK) |
           ((uint32_t)ack_resp->ack_tim << DW3000_HAL_MAC_ACK_RESP_ACK_TIM_SHIFT);
}

static void dw3000_hal_mac_decode_ack_response(
    uint32_t               raw,
    dw3000_mac_ack_resp_t* ack_resp
) {
    ack_resp->w4r_tim = raw & DW3000_HAL_MAC_ACK_RESP_W4R_MASK;
    ack_resp->ack_tim = (uint8_t)((raw & DW3000_HAL_MAC_ACK_RESP_ACK_TIM_MASK) >>
                                  DW3000_HAL_MAC_ACK_RESP_ACK_TIM_SHIFT);
}

static uint32_t dw3000_hal_mac_build_le_pend_pair(
    dw3000_mac_short_addr_t low,
    dw3000_mac_short_addr_t high
) {
    return (uint32_t)low | ((uint32_t)high << 16U);
}

static void dw3000_hal_mac_decode_le_pend_pair(
    uint32_t                 raw,
    dw3000_mac_short_addr_t* low,
    dw3000_mac_short_addr_t* high
) {
    *low  = (dw3000_mac_short_addr_t)(raw & 0xFFFFU);
    *high = (dw3000_mac_short_addr_t)((raw >> 16U) & 0xFFFFU);
}

static dw3000_error_t dw3000_hal_mac_validate_sys_cfg(
    dw3000_mac_sys_cfg_flags_t flags
) {
    uint32_t flag_value = (uint32_t)flags;

    if ((flag_value & ~DW3000_HAL_MAC_SYS_CFG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((flag_value & DW3000_MAC_SYS_CFG_AUTO_ACK) != 0U) &&
        ((flag_value & DW3000_MAC_SYS_CFG_FF_ENABLE) == 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_mac_write_sys_cfg(
    dw3000_device_t*           device,
    dw3000_mac_sys_cfg_flags_t flags
) {
    dw3000_error_t err;
    uint32_t       current;
    uint32_t       value;
    uint32_t       flag_value = (uint32_t)flags & DW3000_HAL_MAC_SYS_CFG_MASK;

    err = dw3000_hal_mac_validate_sys_cfg(flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (device->sys_cfg_cache_valid) {
        current = device->sys_cfg_cache;
    } else {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_CFG, &current);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    value = (current & ~DW3000_HAL_MAC_SYS_CFG_MASK) | flag_value;

    err = dw3000_reg_write_u32(device, DW3000_REG_SYS_CFG, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->sys_cfg_cache            = value;
    device->sys_cfg_cache_valid      = true;
    device->config.mac.sys_cfg_flags = (dw3000_mac_sys_cfg_flags_t)flag_value;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_validate_ack_response(
    const dw3000_mac_ack_resp_t* ack_resp
) {
    if (ack_resp == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (ack_resp->w4r_tim > DW3000_HAL_MAC_ACK_RESP_W4R_MASK) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_validate_config(
    const dw3000_mac_config_t* config
) {
    dw3000_error_t err;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_mac_validate_sys_cfg(config->sys_cfg_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (((uint32_t)config->ff_flags & ~DW3000_HAL_MAC_FF_CFG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_mac_validate_ack_response(&config->ack_resp);
}

dw3000_error_t dw3000_hal_mac_read_eui(
    dw3000_device_t*  device,
    dw3000_mac_eui_t* eui
) {
    dw3000_error_t err;
    uint8_t        raw[8];

    if ((device == NULL) || (eui == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_EUI_64, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *eui = dw3000_hal_mac_unpack_u64(raw);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_set_eui(
    dw3000_device_t* device,
    dw3000_mac_eui_t eui
) {
    uint8_t raw[8];

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    dw3000_hal_mac_pack_u64(eui, raw);

    dw3000_error_t err = dw3000_reg_write(device, DW3000_REG_EUI_64, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.mac.eui = eui;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_read_panadr(
    dw3000_device_t*     device,
    dw3000_mac_panadr_t* panadr
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (panadr == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_PANADR, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_mac_decode_panadr(raw, panadr);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_set_panadr(
    dw3000_device_t*           device,
    const dw3000_mac_panadr_t* panadr
) {
    dw3000_error_t err;

    if ((device == NULL) || (panadr == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_PANADR,
        dw3000_hal_mac_build_panadr(panadr)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.mac.panadr = *panadr;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_read_frame_filter(
    dw3000_device_t*       device,
    dw3000_mac_ff_flags_t* flags
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (flags == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_FF_CFG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *flags = (dw3000_mac_ff_flags_t)raw;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_configure_frame_filter(
    dw3000_device_t*      device,
    dw3000_mac_ff_flags_t flags
) {
    dw3000_error_t err;
    uint32_t       flag_value = (uint32_t)flags;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if ((flag_value & ~DW3000_HAL_MAC_FF_CFG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_FF_CFG, (uint16_t)flag_value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.mac.ff_flags = (dw3000_mac_ff_flags_t)flag_value;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_read_sys_cfg(
    dw3000_device_t*            device,
    dw3000_mac_sys_cfg_flags_t* flags
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (flags == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->sys_cfg_cache_valid) {
        raw = device->sys_cfg_cache;
    } else {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_CFG, &raw);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        device->sys_cfg_cache       = raw;
        device->sys_cfg_cache_valid = true;
    }

    *flags = (dw3000_mac_sys_cfg_flags_t)(raw & DW3000_HAL_MAC_SYS_CFG_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_configure_sys_cfg(
    dw3000_device_t*           device,
    dw3000_mac_sys_cfg_flags_t flags
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_hal_mac_write_sys_cfg(device, flags);
}

dw3000_error_t dw3000_hal_mac_read_ack_response(
    dw3000_device_t*       device,
    dw3000_mac_ack_resp_t* ack_resp
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (ack_resp == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_ACK_RESP_T, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_mac_decode_ack_response(raw, ack_resp);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_configure_ack_response(
    dw3000_device_t*             device,
    const dw3000_mac_ack_resp_t* ack_resp
) {
    dw3000_error_t err;
    uint32_t       value;

    if ((device == NULL) || (ack_resp == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_mac_validate_ack_response(ack_resp);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    value = dw3000_hal_mac_build_ack_response(ack_resp);

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_ACK_RESP_T,
        DW3000_HAL_MAC_ACK_RESP_MASK,
        value
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.mac.ack_resp = *ack_resp;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_read_pending_addresses(
    dw3000_device_t*      device,
    dw3000_mac_le_pend_t* le_pend
) {
    dw3000_error_t err;
    uint32_t       raw01;
    uint32_t       raw23;

    if ((device == NULL) || (le_pend == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_LE_PEND_01, &raw01);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_LE_PEND_23, &raw23);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_mac_decode_le_pend_pair(raw01, &le_pend->addr[0], &le_pend->addr[1]);
    dw3000_hal_mac_decode_le_pend_pair(raw23, &le_pend->addr[2], &le_pend->addr[3]);

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_configure_pending_addresses(
    dw3000_device_t*            device,
    const dw3000_mac_le_pend_t* le_pend
) {
    dw3000_error_t err;

    if ((device == NULL) || (le_pend == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_LE_PEND_01,
        dw3000_hal_mac_build_le_pend_pair(le_pend->addr[0], le_pend->addr[1])
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_LE_PEND_23,
        dw3000_hal_mac_build_le_pend_pair(le_pend->addr[2], le_pend->addr[3])
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.mac.le_pend = *le_pend;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_mac_configure(
    dw3000_device_t*           device,
    const dw3000_mac_config_t* config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_mac_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (config == NULL) {
        config = &device->config.mac;
    }

    err = dw3000_hal_mac_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_mac_set_eui(device, config->eui);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_mac_set_panadr(device, &config->panadr);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_mac_configure_pending_addresses(device, &config->le_pend);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_mac_configure_frame_filter(device, config->ff_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_mac_configure_ack_response(device, &config->ack_resp);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_mac_configure_sys_cfg(device, config->sys_cfg_flags);
}

dw3000_error_t dw3000_hal_mac_configure_current(dw3000_device_t* device) {
    return dw3000_hal_mac_configure(device, NULL);
}
