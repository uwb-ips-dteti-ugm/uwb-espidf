#include "dw3000_hal/sts.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_STS_CFG_CPS_LEN_MASK  0x00FFU
#define DW3000_HAL_STS_STS_ACC_QUAL_MASK 0x0FFFU

#define DW3000_HAL_STS_CTRL_MASK \
    (DW3000_STS_CTRL_LOAD_IV | DW3000_STS_CTRL_RST_LAST)

#define DW3000_HAL_STS_SYS_CFG_CIA_STS_MASK    ((uint32_t)DW3000_STS_SYS_CFG_CIA_STS)
#define DW3000_HAL_STS_SYS_CFG_CP_SPC_SHIFT    12U
#define DW3000_HAL_STS_SYS_CFG_CP_SPC_MASK     (0x3UL << DW3000_HAL_STS_SYS_CFG_CP_SPC_SHIFT)
#define DW3000_HAL_STS_SYS_CFG_CP_SDC_MASK     ((uint32_t)DW3000_STS_SYS_CFG_CP_SDC)
#define DW3000_HAL_STS_SYS_CFG_PDOA_MODE_SHIFT 16U
#define DW3000_HAL_STS_SYS_CFG_PDOA_MODE_MASK  (0x3UL << DW3000_HAL_STS_SYS_CFG_PDOA_MODE_SHIFT)
#define DW3000_HAL_STS_SYS_CFG_FLAGS_MASK      (DW3000_HAL_STS_SYS_CFG_CIA_STS_MASK | DW3000_HAL_STS_SYS_CFG_CP_SDC_MASK)
#define DW3000_HAL_STS_SYS_CFG_MASK            (DW3000_HAL_STS_SYS_CFG_FLAGS_MASK | DW3000_HAL_STS_SYS_CFG_CP_SPC_MASK | DW3000_HAL_STS_SYS_CFG_PDOA_MODE_MASK)

static bool dw3000_hal_sts_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static bool dw3000_hal_sts_has_pdoa(const dw3000_device_t* device) {
    return (device->capabilities & DW3000_DEVICE_CAP_PDOA) != 0U;
}

static bool dw3000_hal_sts_is_valid_packet_cfg(
    dw3000_sts_packet_cfg_t packet_cfg
) {
    return (packet_cfg == DW3000_STS_PACKET_CFG_SP0) ||
           (packet_cfg == DW3000_STS_PACKET_CFG_SP1) ||
           (packet_cfg == DW3000_STS_PACKET_CFG_SP2) ||
           (packet_cfg == DW3000_STS_PACKET_CFG_SP3);
}

static bool dw3000_hal_sts_is_valid_pdoa_mode(
    dw3000_sts_pdoa_mode_t pdoa_mode
) {
    return (pdoa_mode == DW3000_STS_PDOA_MODE_DISABLED) ||
           (pdoa_mode == DW3000_STS_PDOA_MODE_1) ||
           (pdoa_mode == DW3000_STS_PDOA_MODE_3);
}

static uint32_t dw3000_hal_sts_build_sys_cfg(
    dw3000_sts_packet_cfg_t    packet_cfg,
    dw3000_sts_pdoa_mode_t     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t flags
) {
    return ((uint32_t)flags & DW3000_HAL_STS_SYS_CFG_FLAGS_MASK) |
           ((uint32_t)packet_cfg << DW3000_HAL_STS_SYS_CFG_CP_SPC_SHIFT) |
           ((uint32_t)pdoa_mode << DW3000_HAL_STS_SYS_CFG_PDOA_MODE_SHIFT);
}

static void dw3000_hal_sts_decode_sys_cfg(
    uint32_t                    raw,
    dw3000_sts_packet_cfg_t*    packet_cfg,
    dw3000_sts_pdoa_mode_t*     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t* flags
) {
    *packet_cfg = (dw3000_sts_packet_cfg_t)((raw & DW3000_HAL_STS_SYS_CFG_CP_SPC_MASK) >>
                                            DW3000_HAL_STS_SYS_CFG_CP_SPC_SHIFT);
    *pdoa_mode  = (dw3000_sts_pdoa_mode_t)((raw & DW3000_HAL_STS_SYS_CFG_PDOA_MODE_MASK) >>
                                          DW3000_HAL_STS_SYS_CFG_PDOA_MODE_SHIFT);
    *flags      = (dw3000_sts_sys_cfg_flags_t)(raw & DW3000_HAL_STS_SYS_CFG_FLAGS_MASK);
}

static dw3000_error_t dw3000_hal_sts_write_sys_cfg(
    dw3000_device_t*           device,
    dw3000_sts_packet_cfg_t    packet_cfg,
    dw3000_sts_pdoa_mode_t     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t flags
) {
    dw3000_error_t err;
    uint32_t       current;
    uint32_t       value;

    err = dw3000_hal_sts_validate_sys_cfg(packet_cfg, pdoa_mode, flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if ((pdoa_mode != DW3000_STS_PDOA_MODE_DISABLED) &&
        !dw3000_hal_sts_has_pdoa(device)) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    if (device->sys_cfg_cache_valid) {
        current = device->sys_cfg_cache;
    } else {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_CFG, &current);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    value = (current & ~DW3000_HAL_STS_SYS_CFG_MASK) |
            dw3000_hal_sts_build_sys_cfg(packet_cfg, pdoa_mode, flags);

    err = dw3000_reg_write_u32(device, DW3000_REG_SYS_CFG, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->sys_cfg_cache            = value;
    device->sys_cfg_cache_valid      = true;
    device->config.sts.packet_cfg    = packet_cfg;
    device->config.sts.pdoa_mode     = pdoa_mode;
    device->config.sts.sys_cfg_flags = flags;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_validate_sys_cfg(
    dw3000_sts_packet_cfg_t    packet_cfg,
    dw3000_sts_pdoa_mode_t     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t flags
) {
    if (!dw3000_hal_sts_is_valid_packet_cfg(packet_cfg) ||
        !dw3000_hal_sts_is_valid_pdoa_mode(pdoa_mode)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)flags & ~DW3000_HAL_STS_SYS_CFG_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((pdoa_mode != DW3000_STS_PDOA_MODE_DISABLED) &&
        (packet_cfg == DW3000_STS_PACKET_CFG_SP0)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((pdoa_mode == DW3000_STS_PDOA_MODE_3) &&
        (packet_cfg != DW3000_STS_PACKET_CFG_SP3)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_validate_config(
    const dw3000_sts_config_t* config
) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (config->cps_len < DW3000_HAL_STS_CPS_LEN_MIN) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_sts_validate_sys_cfg(
        config->packet_cfg,
        config->pdoa_mode,
        config->sys_cfg_flags
    );
}

uint16_t dw3000_hal_sts_length_units(uint8_t cps_len) {
    if (cps_len < DW3000_HAL_STS_CPS_LEN_MIN) {
        return 0U;
    }

    return (uint16_t)(((uint16_t)cps_len + 1U) * 8U);
}

uint16_t dw3000_hal_sts_acc_qual_threshold(uint8_t cps_len) {
    uint16_t units = dw3000_hal_sts_length_units(cps_len);

    if (units == 0U) {
        return 0U;
    }

    return (uint16_t)(((uint32_t)units * 3U + 4U) / 5U);
}

bool dw3000_hal_sts_acc_qual_is_sufficient(
    uint8_t               cps_len,
    dw3000_sts_acc_qual_t acc_qual
) {
    uint16_t threshold = dw3000_hal_sts_acc_qual_threshold(cps_len);

    return (threshold != 0U) && (acc_qual >= threshold);
}

bool dw3000_hal_sts_timestamp_is_reliable(
    uint8_t                     cps_len,
    const dw3000_cia_path_ts_t* timestamp,
    dw3000_sts_acc_qual_t      acc_qual
) {
    if (timestamp == NULL) {
        return false;
    }

    return (timestamp->toast == 0U) &&
           dw3000_hal_sts_acc_qual_is_sufficient(cps_len, acc_qual);
}

dw3000_error_t dw3000_hal_sts_read_config(
    dw3000_device_t*     device,
    dw3000_sts_config_t* config
) {
    dw3000_error_t err;

    if ((device == NULL) || (config == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_sts_read_length(device, &config->cps_len);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sts_read_key(device, &config->key);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sts_read_iv(device, &config->iv);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_sts_read_sys_cfg(
        device,
        &config->packet_cfg,
        &config->pdoa_mode,
        &config->sys_cfg_flags
    );
}

dw3000_error_t dw3000_hal_sts_read_length(
    dw3000_device_t* device,
    uint8_t*         cps_len
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (cps_len == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_STS_CFG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *cps_len = (uint8_t)(raw & DW3000_HAL_STS_CFG_CPS_LEN_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_set_length(
    dw3000_device_t* device,
    uint8_t          cps_len
) {
    dw3000_error_t err;
    uint16_t       raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (cps_len < DW3000_HAL_STS_CPS_LEN_MIN) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_STS_CFG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    raw = (uint16_t)((raw & ~DW3000_HAL_STS_CFG_CPS_LEN_MASK) |
                     (uint16_t)cps_len);

    err = dw3000_reg_write_u16(device, DW3000_REG_STS_CFG, raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.sts.cps_len = cps_len;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_read_acc_qual(
    dw3000_device_t*       device,
    dw3000_sts_acc_qual_t* acc_qual
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (acc_qual == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_STS_STS, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *acc_qual = (dw3000_sts_acc_qual_t)(raw & DW3000_HAL_STS_STS_ACC_QUAL_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_read_key(
    dw3000_device_t*  device,
    dw3000_sts_key_t* key
) {
    if ((device == NULL) || (key == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_read(device, DW3000_REG_STS_KEY, key->bytes, sizeof(key->bytes));
}

dw3000_error_t dw3000_hal_sts_set_key(
    dw3000_device_t*        device,
    const dw3000_sts_key_t* key
) {
    dw3000_error_t err;

    if ((device == NULL) || (key == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write(device, DW3000_REG_STS_KEY, key->bytes, sizeof(key->bytes));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.sts.key = *key;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_read_iv(
    dw3000_device_t* device,
    dw3000_sts_iv_t* iv
) {
    if ((device == NULL) || (iv == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_read(device, DW3000_REG_STS_IV, iv->bytes, sizeof(iv->bytes));
}

dw3000_error_t dw3000_hal_sts_set_iv(
    dw3000_device_t*       device,
    const dw3000_sts_iv_t* iv
) {
    dw3000_error_t err;

    if ((device == NULL) || (iv == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write(device, DW3000_REG_STS_IV, iv->bytes, sizeof(iv->bytes));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.sts.iv = *iv;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_read_counter(
    dw3000_device_t* device,
    uint32_t*        counter
) {
    if ((device == NULL) || (counter == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_read_u32(device, DW3000_REG_CTR_DBG, counter);
}

dw3000_error_t dw3000_hal_sts_read_sys_cfg(
    dw3000_device_t*            device,
    dw3000_sts_packet_cfg_t*    packet_cfg,
    dw3000_sts_pdoa_mode_t*     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t* flags
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) ||
        (packet_cfg == NULL) ||
        (pdoa_mode == NULL) ||
        (flags == NULL)) {
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

    dw3000_hal_sts_decode_sys_cfg(raw, packet_cfg, pdoa_mode, flags);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_configure_sys_cfg(
    dw3000_device_t*           device,
    dw3000_sts_packet_cfg_t    packet_cfg,
    dw3000_sts_pdoa_mode_t     pdoa_mode,
    dw3000_sts_sys_cfg_flags_t flags
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_hal_sts_write_sys_cfg(device, packet_cfg, pdoa_mode, flags);
}

dw3000_error_t dw3000_hal_sts_issue_control(
    dw3000_device_t*        device,
    dw3000_sts_ctrl_flags_t flags
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (((uint32_t)flags & ~DW3000_HAL_STS_CTRL_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint32_t)flags & DW3000_HAL_STS_CTRL_MASK) == 0U) {
        return DW3000_ERROR_OK;
    }

    return dw3000_reg_write_u8(
        device,
        DW3000_REG_STS_CTRL,
        (uint8_t)((uint32_t)flags & DW3000_HAL_STS_CTRL_MASK)
    );
}

dw3000_error_t dw3000_hal_sts_load_iv(dw3000_device_t* device) {
    return dw3000_hal_sts_issue_control(device, DW3000_STS_CTRL_LOAD_IV);
}

dw3000_error_t dw3000_hal_sts_restart_from_last(dw3000_device_t* device) {
    return dw3000_hal_sts_issue_control(device, DW3000_STS_CTRL_RST_LAST);
}

dw3000_error_t dw3000_hal_sts_configure(
    dw3000_device_t*           device,
    const dw3000_sts_config_t* config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_sts_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (config == NULL) {
        config = &device->config.sts;
    }

    err = dw3000_hal_sts_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sts_set_length(device, config->cps_len);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sts_set_key(device, &config->key);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sts_set_iv(device, &config->iv);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_sts_configure_sys_cfg(
        device,
        config->packet_cfg,
        config->pdoa_mode,
        config->sys_cfg_flags
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if ((config->packet_cfg != DW3000_STS_PACKET_CFG_SP0) &&
        ((config->sys_cfg_flags & DW3000_STS_SYS_CFG_CP_SDC) == 0U)) {
        err = dw3000_hal_sts_load_iv(device);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    device->config.sts = *config;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_sts_configure_current(dw3000_device_t* device) {
    return dw3000_hal_sts_configure(device, NULL);
}
