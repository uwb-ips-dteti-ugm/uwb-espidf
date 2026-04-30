#include "dw3000_hal/cia.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_CIA_CONF_RX_ANTD_MASK 0x0000FFFFUL
#define DW3000_HAL_CIA_CONF_FLAGS_MASK   ((uint32_t)DW3000_CIA_CONF_MINDIAG)
#define DW3000_HAL_CIA_CONF_MASK         (DW3000_HAL_CIA_CONF_RX_ANTD_MASK | DW3000_HAL_CIA_CONF_FLAGS_MASK)

#define DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_SHIFT 8U
#define DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_MASK \
    (0x7UL << DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_SHIFT)
#define DW3000_HAL_CIA_FP_CONF_CAL_TEMP_SHIFT 11U
#define DW3000_HAL_CIA_FP_CONF_CAL_TEMP_MASK \
    (0xFFUL << DW3000_HAL_CIA_FP_CONF_CAL_TEMP_SHIFT)
#define DW3000_HAL_CIA_FP_CONF_MASK             \
    (DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_MASK | \
     DW3000_HAL_CIA_FP_CONF_CAL_TEMP_MASK)

#define DW3000_HAL_CIA_IP_CONF_NTM_MASK    0x0000001FUL
#define DW3000_HAL_CIA_IP_CONF_PMULT_SHIFT 5U
#define DW3000_HAL_CIA_IP_CONF_PMULT_MASK \
    (0x3UL << DW3000_HAL_CIA_IP_CONF_PMULT_SHIFT)
#define DW3000_HAL_CIA_IP_CONF_RTM_SHIFT 16U
#define DW3000_HAL_CIA_IP_CONF_RTM_MASK \
    (0x1FUL << DW3000_HAL_CIA_IP_CONF_RTM_SHIFT)
#define DW3000_HAL_CIA_IP_CONF_MASK      \
    (DW3000_HAL_CIA_IP_CONF_NTM_MASK |   \
     DW3000_HAL_CIA_IP_CONF_PMULT_MASK | \
     DW3000_HAL_CIA_IP_CONF_RTM_MASK)

#define DW3000_HAL_CIA_STS_CONF_0_NTM_MASK    0x0000001FUL
#define DW3000_HAL_CIA_STS_CONF_0_PMULT_SHIFT 5U
#define DW3000_HAL_CIA_STS_CONF_0_PMULT_MASK \
    (0x3UL << DW3000_HAL_CIA_STS_CONF_0_PMULT_SHIFT)
#define DW3000_HAL_CIA_STS_CONF_0_MNTH_SHIFT 16U
#define DW3000_HAL_CIA_STS_CONF_0_MNTH_MASK \
    (0x7FUL << DW3000_HAL_CIA_STS_CONF_0_MNTH_SHIFT)
#define DW3000_HAL_CIA_STS_CONF_0_MASK      \
    (DW3000_HAL_CIA_STS_CONF_0_NTM_MASK |   \
     DW3000_HAL_CIA_STS_CONF_0_PMULT_MASK | \
     DW3000_HAL_CIA_STS_CONF_0_MNTH_MASK)

#define DW3000_HAL_CIA_STS_CONF_1_CQ_EN_MASK (1UL << 29U)

#define DW3000_HAL_CIA_ADJUST_MASK 0x3FFFU

#define DW3000_HAL_CIA_TOA_MASK        UINT64_C(0xFFFFFFFFFF)
#define DW3000_HAL_CIA_POA_SHIFT       8U
#define DW3000_HAL_CIA_POA_BITS        14U
#define DW3000_HAL_CIA_IP_TOAST_SHIFT  24U
#define DW3000_HAL_CIA_IP_TOAST_MASK   0x00FFU
#define DW3000_HAL_CIA_STS_TOAST_SHIFT 23U
#define DW3000_HAL_CIA_STS_TOAST_MASK  0x01FFU

#define DW3000_HAL_CIA_TDOA_BITS          41U
#define DW3000_HAL_CIA_PDOA_BITS          14U
#define DW3000_HAL_CIA_PDOA_FP_TH_MD_MASK (1U << 14U)

#define DW3000_HAL_CIA_COE_PPM_MASK 0x1FFFUL
#define DW3000_HAL_CIA_COE_PPM_BITS 13U

#define DW3000_HAL_CIA_DIAG_PEAK_AMPL_MASK   0x001FFFFFUL
#define DW3000_HAL_CIA_DIAG_PEAK_INDEX_SHIFT 21U
#define DW3000_HAL_CIA_DIAG_FP_AMPL_MASK     0x003FFFFFUL

typedef struct {
    dw3000_reg_desc_t reg;
    uint8_t           toast_shift;
    uint16_t          toast_mask;
} dw3000_hal_cia_path_ts_regs_t;

typedef struct {
    dw3000_reg_desc_t peak;
    dw3000_reg_desc_t power;
    dw3000_reg_desc_t fp1;
    dw3000_reg_desc_t fp2;
    dw3000_reg_desc_t fp3;
    dw3000_reg_desc_t fp_index;
    dw3000_reg_desc_t pacc;
    uint32_t          power_mask;
    uint16_t          peak_index_mask;
    uint16_t          fp_index_mask;
    uint16_t          pacc_mask;
} dw3000_hal_cia_path_diag_regs_t;

static bool dw3000_hal_cia_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static bool dw3000_hal_cia_has_pdoa(const dw3000_device_t* device) {
    return (device->capabilities & DW3000_DEVICE_CAP_PDOA) != 0U;
}

static int64_t dw3000_hal_cia_sign_extend(
    uint64_t value,
    uint8_t  bits
) {
    uint64_t sign_bit = UINT64_C(1) << (bits - 1U);
    uint64_t mask     = (sign_bit << 1U) - 1U;

    value &= mask;
    if ((value & sign_bit) == 0U) {
        return (int64_t)value;
    }

    return -(int64_t)(((~value) & mask) + 1U);
}

static uint64_t dw3000_hal_cia_unpack_le(
    const uint8_t* raw,
    size_t         raw_len
) {
    uint64_t value = 0U;

    for (size_t i = 0U; i < raw_len; ++i) {
        value |= (uint64_t)raw[i] << (8U * i);
    }

    return value;
}

static dw3000_error_t dw3000_hal_cia_validate_flags(
    dw3000_cia_conf_flags_t flags
) {
    if (((uint32_t)flags & ~DW3000_HAL_CIA_CONF_FLAGS_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_cia_validate_fp_conf(
    const dw3000_cia_fp_conf_t* fp_conf
) {
    if (fp_conf == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (fp_conf->fp_agreed_th > 0x7U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_cia_validate_ip_conf(
    const dw3000_cia_ip_conf_t* ip_conf
) {
    if (ip_conf == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((ip_conf->ntm > 0x1FU) ||
        (ip_conf->pmult > 0x3U) ||
        (ip_conf->rtm > 0x1FU)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_cia_validate_sts_conf(
    const dw3000_cia_sts_conf_t* sts_conf
) {
    if (sts_conf == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((sts_conf->ntm > 0x1FU) ||
        (sts_conf->pmult > 0x3U) ||
        (sts_conf->mnth > 0x7FU) ||
        (sts_conf->rtm != 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_cia_validate_adjust(
    dw3000_cia_adjust_t adjust
) {
    if (adjust > DW3000_HAL_CIA_ADJUST_MASK) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static uint32_t dw3000_hal_cia_build_conf(
    dw3000_cia_antenna_delay_t rx_antd,
    dw3000_cia_conf_flags_t    flags
) {
    return ((uint32_t)rx_antd & DW3000_HAL_CIA_CONF_RX_ANTD_MASK) |
           ((uint32_t)flags & DW3000_HAL_CIA_CONF_FLAGS_MASK);
}

static uint32_t dw3000_hal_cia_build_fp_conf(
    const dw3000_cia_fp_conf_t* fp_conf
) {
    return ((uint32_t)fp_conf->fp_agreed_th << DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_SHIFT) |
           ((uint32_t)fp_conf->tc_rcfg << DW3000_HAL_CIA_FP_CONF_CAL_TEMP_SHIFT);
}

static uint32_t dw3000_hal_cia_build_ip_conf(
    const dw3000_cia_ip_conf_t* ip_conf
) {
    return ((uint32_t)ip_conf->ntm & DW3000_HAL_CIA_IP_CONF_NTM_MASK) |
           ((uint32_t)ip_conf->pmult << DW3000_HAL_CIA_IP_CONF_PMULT_SHIFT) |
           ((uint32_t)ip_conf->rtm << DW3000_HAL_CIA_IP_CONF_RTM_SHIFT);
}

static uint32_t dw3000_hal_cia_build_sts_conf_0(
    const dw3000_cia_sts_conf_t* sts_conf
) {
    return ((uint32_t)sts_conf->ntm & DW3000_HAL_CIA_STS_CONF_0_NTM_MASK) |
           ((uint32_t)sts_conf->pmult << DW3000_HAL_CIA_STS_CONF_0_PMULT_SHIFT) |
           ((uint32_t)sts_conf->mnth << DW3000_HAL_CIA_STS_CONF_0_MNTH_SHIFT);
}

static uint32_t dw3000_hal_cia_build_sts_conf_1(
    const dw3000_cia_sts_conf_t* sts_conf
) {
    return sts_conf->cq_en ? DW3000_HAL_CIA_STS_CONF_1_CQ_EN_MASK : 0U;
}

static void dw3000_hal_cia_decode_conf(
    uint32_t                    raw,
    dw3000_cia_antenna_delay_t* rx_antd,
    dw3000_cia_conf_flags_t*    flags
) {
    *rx_antd = (dw3000_cia_antenna_delay_t)(raw & DW3000_HAL_CIA_CONF_RX_ANTD_MASK);
    *flags   = (dw3000_cia_conf_flags_t)(raw & DW3000_HAL_CIA_CONF_FLAGS_MASK);
}

static void dw3000_hal_cia_decode_fp_conf(
    uint32_t              raw,
    dw3000_cia_fp_conf_t* fp_conf
) {
    fp_conf->fp_agreed_th =
        (uint8_t)((raw & DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_MASK) >>
                  DW3000_HAL_CIA_FP_CONF_FP_AGREED_TH_SHIFT);
    fp_conf->tc_rcfg =
        (uint8_t)((raw & DW3000_HAL_CIA_FP_CONF_CAL_TEMP_MASK) >>
                  DW3000_HAL_CIA_FP_CONF_CAL_TEMP_SHIFT);
}

static void dw3000_hal_cia_decode_ip_conf(
    uint32_t              raw,
    dw3000_cia_ip_conf_t* ip_conf
) {
    ip_conf->ntm   = (uint8_t)(raw & DW3000_HAL_CIA_IP_CONF_NTM_MASK);
    ip_conf->pmult = (uint8_t)((raw & DW3000_HAL_CIA_IP_CONF_PMULT_MASK) >>
                               DW3000_HAL_CIA_IP_CONF_PMULT_SHIFT);
    ip_conf->rtm   = (uint8_t)((raw & DW3000_HAL_CIA_IP_CONF_RTM_MASK) >>
                             DW3000_HAL_CIA_IP_CONF_RTM_SHIFT);
}

static void dw3000_hal_cia_decode_sts_conf(
    uint32_t               raw0,
    uint32_t               raw1,
    dw3000_cia_sts_conf_t* sts_conf
) {
    sts_conf->ntm   = (uint8_t)(raw0 & DW3000_HAL_CIA_STS_CONF_0_NTM_MASK);
    sts_conf->pmult = (uint8_t)((raw0 & DW3000_HAL_CIA_STS_CONF_0_PMULT_MASK) >>
                                DW3000_HAL_CIA_STS_CONF_0_PMULT_SHIFT);
    sts_conf->rtm   = 0U;
    sts_conf->mnth  = (uint16_t)((raw0 & DW3000_HAL_CIA_STS_CONF_0_MNTH_MASK) >>
                                DW3000_HAL_CIA_STS_CONF_0_MNTH_SHIFT);
    sts_conf->cq_en = (raw1 & DW3000_HAL_CIA_STS_CONF_1_CQ_EN_MASK) != 0U;
}

static void dw3000_hal_cia_decode_path_timestamp(
    const uint8_t         raw[8],
    uint8_t               toast_shift,
    uint16_t              toast_mask,
    dw3000_cia_path_ts_t* timestamp
) {
    uint32_t upper = (uint32_t)raw[4] |
                     ((uint32_t)raw[5] << 8U) |
                     ((uint32_t)raw[6] << 16U) |
                     ((uint32_t)raw[7] << 24U);
    uint16_t poa = (uint16_t)((upper >> DW3000_HAL_CIA_POA_SHIFT) &
                              ((1U << DW3000_HAL_CIA_POA_BITS) - 1U));

    timestamp->toa = dw3000_hal_cia_unpack_le(raw, 5U) & DW3000_HAL_CIA_TOA_MASK;
    timestamp->poa = (int16_t)dw3000_hal_cia_sign_extend(
        poa,
        DW3000_HAL_CIA_POA_BITS
    );
    timestamp->toast = (uint16_t)((upper >> toast_shift) & toast_mask);
}

static dw3000_error_t dw3000_hal_cia_path_timestamp_regs(
    dw3000_hal_cia_path_t          path,
    dw3000_hal_cia_path_ts_regs_t* regs
) {
    switch (path) {
        case DW3000_HAL_CIA_PATH_IP:
            regs->reg         = DW3000_REG_IP_TS;
            regs->toast_shift = DW3000_HAL_CIA_IP_TOAST_SHIFT;
            regs->toast_mask  = DW3000_HAL_CIA_IP_TOAST_MASK;
            return DW3000_ERROR_OK;
        case DW3000_HAL_CIA_PATH_STS:
            regs->reg         = DW3000_REG_STS_TS;
            regs->toast_shift = DW3000_HAL_CIA_STS_TOAST_SHIFT;
            regs->toast_mask  = DW3000_HAL_CIA_STS_TOAST_MASK;
            return DW3000_ERROR_OK;
        case DW3000_HAL_CIA_PATH_STS1:
            regs->reg         = DW3000_REG_STS1_TS;
            regs->toast_shift = DW3000_HAL_CIA_STS_TOAST_SHIFT;
            regs->toast_mask  = DW3000_HAL_CIA_STS_TOAST_MASK;
            return DW3000_ERROR_OK;
        default:
            return DW3000_ERROR_INVALID_ARG;
    }
}

static dw3000_error_t dw3000_hal_cia_path_diag_regs(
    dw3000_hal_cia_path_t            path,
    dw3000_hal_cia_path_diag_regs_t* regs
) {
    switch (path) {
        case DW3000_HAL_CIA_PATH_IP:
            regs->peak            = DW3000_REG_IP_DIAG_0;
            regs->power           = DW3000_REG_IP_DIAG_1;
            regs->fp1             = DW3000_REG_IP_DIAG_2;
            regs->fp2             = DW3000_REG_IP_DIAG_3;
            regs->fp3             = DW3000_REG_IP_DIAG_4;
            regs->fp_index        = DW3000_REG_IP_DIAG_8;
            regs->pacc            = DW3000_REG_IP_DIAG_12;
            regs->power_mask      = 0x0001FFFFUL;
            regs->peak_index_mask = 0x03FFU;
            regs->fp_index_mask   = 0xFFFFU;
            regs->pacc_mask       = 0x0FFFU;
            return DW3000_ERROR_OK;
        case DW3000_HAL_CIA_PATH_STS:
            regs->peak            = DW3000_REG_STS_DIAG_0;
            regs->power           = DW3000_REG_STS_DIAG_1;
            regs->fp1             = DW3000_REG_STS_DIAG_2;
            regs->fp2             = DW3000_REG_STS_DIAG_3;
            regs->fp3             = DW3000_REG_STS_DIAG_4;
            regs->fp_index        = DW3000_REG_STS_DIAG_8;
            regs->pacc            = DW3000_REG_STS_DIAG_12;
            regs->power_mask      = 0x0000FFFFUL;
            regs->peak_index_mask = 0x01FFU;
            regs->fp_index_mask   = 0x7FFFU;
            regs->pacc_mask       = 0x07FFU;
            return DW3000_ERROR_OK;
        case DW3000_HAL_CIA_PATH_STS1:
            regs->peak            = DW3000_REG_STS1_DIAG_0;
            regs->power           = DW3000_REG_STS1_DIAG_1;
            regs->fp1             = DW3000_REG_STS1_DIAG_2;
            regs->fp2             = DW3000_REG_STS1_DIAG_3;
            regs->fp3             = DW3000_REG_STS1_DIAG_4;
            regs->fp_index        = DW3000_REG_STS1_DIAG_8;
            regs->pacc            = DW3000_REG_STS1_DIAG_12;
            regs->power_mask      = 0x0000FFFFUL;
            regs->peak_index_mask = 0x01FFU;
            regs->fp_index_mask   = 0x7FFFU;
            regs->pacc_mask       = 0x07FFU;
            return DW3000_ERROR_OK;
        default:
            return DW3000_ERROR_INVALID_ARG;
    }
}

static dw3000_error_t dw3000_hal_cia_check_path_supported(
    const dw3000_device_t* device,
    dw3000_hal_cia_path_t  path
) {
    if ((path == DW3000_HAL_CIA_PATH_STS1) && !dw3000_hal_cia_has_pdoa(device)) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_validate_config(
    const dw3000_cia_config_t* config
) {
    dw3000_error_t err;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_cia_validate_flags(config->conf_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_validate_fp_conf(&config->fp_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_validate_ip_conf(&config->ip_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_validate_sts_conf(&config->sts_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_cia_validate_adjust(config->adjust);
}

dw3000_error_t dw3000_hal_cia_read_config(
    dw3000_device_t*     device,
    dw3000_cia_config_t* config
) {
    dw3000_error_t err;

    if ((device == NULL) || (config == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_cia_read_conf(device, &config->rx_antd, &config->conf_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_read_tx_antenna_delay(device, &config->tx_antd);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_read_fp_conf(device, &config->fp_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_read_ip_conf(device, &config->ip_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_read_sts_conf(device, &config->sts_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_cia_read_adjust(device, &config->adjust);
}

dw3000_error_t dw3000_hal_cia_read_conf(
    dw3000_device_t*            device,
    dw3000_cia_antenna_delay_t* rx_antd,
    dw3000_cia_conf_flags_t*    flags
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (rx_antd == NULL) || (flags == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CIA_CONF, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_cia_decode_conf(raw, rx_antd, flags);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_configure_conf(
    dw3000_device_t*           device,
    dw3000_cia_antenna_delay_t rx_antd,
    dw3000_cia_conf_flags_t    flags
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_cia_validate_flags(flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CIA_CONF,
        DW3000_HAL_CIA_CONF_MASK,
        dw3000_hal_cia_build_conf(rx_antd, flags)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.rx_antd    = rx_antd;
    device->config.cia.conf_flags = flags;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_tx_antenna_delay(
    dw3000_device_t*            device,
    dw3000_cia_antenna_delay_t* delay
) {
    if ((device == NULL) || (delay == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read_u16(device, DW3000_REG_TX_ANTD, delay);
}

dw3000_error_t dw3000_hal_cia_set_rx_antenna_delay(
    dw3000_device_t*           device,
    dw3000_cia_antenna_delay_t delay
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_CIA_CONF,
        DW3000_HAL_CIA_CONF_RX_ANTD_MASK,
        (uint32_t)delay
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.rx_antd = delay;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_set_tx_antenna_delay(
    dw3000_device_t*           device,
    dw3000_cia_antenna_delay_t delay
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_TX_ANTD, delay);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.tx_antd = delay;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_fp_conf(
    dw3000_device_t*      device,
    dw3000_cia_fp_conf_t* fp_conf
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (fp_conf == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_FP_CONF, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_cia_decode_fp_conf(raw, fp_conf);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_configure_fp_conf(
    dw3000_device_t*            device,
    const dw3000_cia_fp_conf_t* fp_conf
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_cia_validate_fp_conf(fp_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_FP_CONF,
        DW3000_HAL_CIA_FP_CONF_MASK,
        dw3000_hal_cia_build_fp_conf(fp_conf)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.fp_conf = *fp_conf;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_ip_conf(
    dw3000_device_t*      device,
    dw3000_cia_ip_conf_t* ip_conf
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (ip_conf == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_IP_CONF, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_cia_decode_ip_conf(raw, ip_conf);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_configure_ip_conf(
    dw3000_device_t*            device,
    const dw3000_cia_ip_conf_t* ip_conf
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_cia_validate_ip_conf(ip_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_IP_CONF,
        DW3000_HAL_CIA_IP_CONF_MASK,
        dw3000_hal_cia_build_ip_conf(ip_conf)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.ip_conf = *ip_conf;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_sts_conf(
    dw3000_device_t*       device,
    dw3000_cia_sts_conf_t* sts_conf
) {
    dw3000_error_t err;
    uint32_t       raw0;
    uint32_t       raw1;

    if ((device == NULL) || (sts_conf == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_STS_CONF_0, &raw0);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_STS_CONF_1, &raw1);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_cia_decode_sts_conf(raw0, raw1, sts_conf);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_configure_sts_conf(
    dw3000_device_t*             device,
    const dw3000_cia_sts_conf_t* sts_conf
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_cia_validate_sts_conf(sts_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_STS_CONF_0,
        DW3000_HAL_CIA_STS_CONF_0_MASK,
        dw3000_hal_cia_build_sts_conf_0(sts_conf)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_modify_u32(
        device,
        DW3000_REG_STS_CONF_1,
        DW3000_HAL_CIA_STS_CONF_1_CQ_EN_MASK,
        dw3000_hal_cia_build_sts_conf_1(sts_conf)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.sts_conf = *sts_conf;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_adjust(
    dw3000_device_t*     device,
    dw3000_cia_adjust_t* adjust
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (adjust == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_CIA_ADJUST, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *adjust = (dw3000_cia_adjust_t)(raw & DW3000_HAL_CIA_ADJUST_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_configure_adjust(
    dw3000_device_t*    device,
    dw3000_cia_adjust_t adjust
) {
    dw3000_error_t err;
    uint16_t       raw;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_cia_validate_adjust(adjust);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_CIA_ADJUST, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    raw = (uint16_t)((raw & ~DW3000_HAL_CIA_ADJUST_MASK) |
                     (adjust & DW3000_HAL_CIA_ADJUST_MASK));

    err = dw3000_reg_write_u16(device, DW3000_REG_CIA_ADJUST, raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia.adjust = adjust;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_path_timestamp(
    dw3000_device_t*      device,
    dw3000_hal_cia_path_t path,
    dw3000_cia_path_ts_t* timestamp
) {
    dw3000_error_t                err;
    dw3000_hal_cia_path_ts_regs_t regs;
    uint8_t                       raw[8];

    if ((device == NULL) || (timestamp == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_cia_path_timestamp_regs(path, &regs);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_check_path_supported(device, path);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read(device, regs.reg, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_cia_decode_path_timestamp(
        raw,
        regs.toast_shift,
        regs.toast_mask,
        timestamp
    );
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_ip_timestamp(
    dw3000_device_t*      device,
    dw3000_cia_path_ts_t* timestamp
) {
    return dw3000_hal_cia_read_path_timestamp(
        device,
        DW3000_HAL_CIA_PATH_IP,
        timestamp
    );
}

dw3000_error_t dw3000_hal_cia_read_sts_timestamp(
    dw3000_device_t*      device,
    dw3000_cia_path_ts_t* timestamp
) {
    return dw3000_hal_cia_read_path_timestamp(
        device,
        DW3000_HAL_CIA_PATH_STS,
        timestamp
    );
}

dw3000_error_t dw3000_hal_cia_read_sts1_timestamp(
    dw3000_device_t*      device,
    dw3000_cia_path_ts_t* timestamp
) {
    return dw3000_hal_cia_read_path_timestamp(
        device,
        DW3000_HAL_CIA_PATH_STS1,
        timestamp
    );
}

dw3000_error_t dw3000_hal_cia_read_tdoa(
    dw3000_device_t*   device,
    dw3000_cia_tdoa_t* tdoa
) {
    dw3000_error_t err;
    uint8_t        raw[6];
    uint64_t       value;

    if ((device == NULL) || (tdoa == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_TDOA, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    value = dw3000_hal_cia_unpack_le(raw, sizeof(raw));
    *tdoa = (dw3000_cia_tdoa_t)dw3000_hal_cia_sign_extend(
        value,
        DW3000_HAL_CIA_TDOA_BITS
    );

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_pdoa(
    dw3000_device_t*   device,
    dw3000_cia_pdoa_t* pdoa
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (pdoa == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_has_pdoa(device)) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_PDOA, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    pdoa->value    = (int16_t)dw3000_hal_cia_sign_extend(raw, DW3000_HAL_CIA_PDOA_BITS);
    pdoa->fp_th_md = (raw & DW3000_HAL_CIA_PDOA_FP_TH_MD_MASK) != 0U;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_diag(
    dw3000_device_t*   device,
    dw3000_cia_diag_t* diag
) {
    dw3000_error_t err;
    uint32_t       raw0;
    uint32_t       raw1;

    if ((device == NULL) || (diag == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CIA_DIAG_0, &raw0);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_CIA_DIAG_1, &raw1);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    diag->clock_offset = (int16_t)dw3000_hal_cia_sign_extend(
        raw0 & DW3000_HAL_CIA_COE_PPM_MASK,
        DW3000_HAL_CIA_COE_PPM_BITS
    );
    diag->reserved = raw1;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_path_diag(
    dw3000_device_t*        device,
    dw3000_hal_cia_path_t   path,
    dw3000_cia_path_diag_t* diag
) {
    dw3000_error_t                  err;
    dw3000_hal_cia_path_diag_regs_t regs;
    uint32_t                        peak;
    uint32_t                        power;
    uint32_t                        fp1;
    uint32_t                        fp2;
    uint32_t                        fp3;
    uint32_t                        fp_index;
    uint32_t                        pacc;

    if ((device == NULL) || (diag == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_cia_path_diag_regs(path, &regs);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_check_path_supported(device, path);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.peak, &peak);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.power, &power);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.fp1, &fp1);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.fp2, &fp2);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.fp3, &fp3);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.fp_index, &fp_index);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_read_u32(device, regs.pacc, &pacc);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    diag->fp_index       = (uint16_t)(fp_index & regs.fp_index_mask);
    diag->cir_pwr        = power & regs.power_mask;
    diag->fp_ampl1       = fp1 & DW3000_HAL_CIA_DIAG_FP_AMPL_MASK;
    diag->fp_ampl2       = fp2 & DW3000_HAL_CIA_DIAG_FP_AMPL_MASK;
    diag->fp_ampl3       = fp3 & DW3000_HAL_CIA_DIAG_FP_AMPL_MASK;
    diag->peak_amplitude = peak & DW3000_HAL_CIA_DIAG_PEAK_AMPL_MASK;
    diag->peak_index     = (uint16_t)((peak >> DW3000_HAL_CIA_DIAG_PEAK_INDEX_SHIFT) &
                                  regs.peak_index_mask);
    diag->pacc_nosat     = (uint16_t)(pacc & regs.pacc_mask);

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_read_ip_diag(
    dw3000_device_t*        device,
    dw3000_cia_path_diag_t* diag
) {
    return dw3000_hal_cia_read_path_diag(
        device,
        DW3000_HAL_CIA_PATH_IP,
        diag
    );
}

dw3000_error_t dw3000_hal_cia_read_sts_diag(
    dw3000_device_t*        device,
    dw3000_cia_path_diag_t* diag
) {
    return dw3000_hal_cia_read_path_diag(
        device,
        DW3000_HAL_CIA_PATH_STS,
        diag
    );
}

dw3000_error_t dw3000_hal_cia_read_sts1_diag(
    dw3000_device_t*        device,
    dw3000_cia_path_diag_t* diag
) {
    return dw3000_hal_cia_read_path_diag(
        device,
        DW3000_HAL_CIA_PATH_STS1,
        diag
    );
}

dw3000_error_t dw3000_hal_cia_configure(
    dw3000_device_t*           device,
    const dw3000_cia_config_t* config
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_cia_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (config == NULL) {
        config = &device->config.cia;
    }

    err = dw3000_hal_cia_validate_config(config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_configure_conf(device, config->rx_antd, config->conf_flags);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_set_tx_antenna_delay(device, config->tx_antd);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_configure_fp_conf(device, &config->fp_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_configure_ip_conf(device, &config->ip_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_configure_sts_conf(device, &config->sts_conf);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_cia_configure_adjust(device, config->adjust);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.cia = *config;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_cia_configure_current(dw3000_device_t* device) {
    return dw3000_hal_cia_configure(device, NULL);
}
