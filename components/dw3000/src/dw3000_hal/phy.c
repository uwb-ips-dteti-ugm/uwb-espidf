#include "dw3000_hal/phy.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"
#include "dw3000_types/rf.h"

#define DW3000_PHY_SYS_CFG_PHR_MODE_BIT (1UL << 4U)
#define DW3000_PHY_SYS_CFG_PHR_6M8_BIT  (1UL << 5U)
#define DW3000_PHY_SYS_CFG_MASK         (DW3000_PHY_SYS_CFG_PHR_MODE_BIT | DW3000_PHY_SYS_CFG_PHR_6M8_BIT)

#define DW3000_PHY_TX_FCTRL_TXBR_BIT    (1UL << 10U)
#define DW3000_PHY_TX_FCTRL_TXPSR_SHIFT 12U
#define DW3000_PHY_TX_FCTRL_TXPSR_MASK  (0xFUL << DW3000_PHY_TX_FCTRL_TXPSR_SHIFT)
#define DW3000_PHY_TX_FCTRL_MASK        (DW3000_PHY_TX_FCTRL_TXBR_BIT | DW3000_PHY_TX_FCTRL_TXPSR_MASK)

#define DW3000_PHY_CHAN_SFD_TYPE_SHIFT 1U
#define DW3000_PHY_CHAN_TX_PCODE_SHIFT 3U
#define DW3000_PHY_CHAN_RX_PCODE_SHIFT 8U

#define DW3000_PHY_DTUNE0_PAC_MASK  0x0003U
#define DW3000_PHY_DTUNE0_DT0B4_BIT (1U << 4U)

#define DW3000_PHY_DGC_CFG_RX_TUNE_EN   (1U << 0U)
#define DW3000_PHY_DGC_CFG_THR_64_SHIFT 9U
#define DW3000_PHY_DGC_CFG_THR_64_MASK  (0x3FU << DW3000_PHY_DGC_CFG_THR_64_SHIFT)
#define DW3000_PHY_DGC_CFG_MASK         (DW3000_PHY_DGC_CFG_RX_TUNE_EN | DW3000_PHY_DGC_CFG_THR_64_MASK)

#define DW3000_PHY_RF_TX_CTRL_2_CH5 0x1C071134UL
#define DW3000_PHY_RF_TX_CTRL_2_CH9 0x1C010034UL
#define DW3000_PHY_PLL_CFG_CH5      0x1F3CU
#define DW3000_PHY_PLL_CFG_CH9      0x0F3CU
#define DW3000_PHY_PLL_CAL_CFG_LD   0x0081U

static bool dw3000_hal_phy_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static bool dw3000_hal_phy_is_valid_preamble_length(
    dw3000_phy_preamble_length_t preamble_length
) {
    return (preamble_length == DW3000_PHY_PREAMBLE_LEN_32) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_64) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_128) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_256) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_512) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_1024) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_1536) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_2048) ||
           (preamble_length == DW3000_PHY_PREAMBLE_LEN_4096);
}

static bool dw3000_hal_phy_preamble_code_matches_prf(
    dw3000_phy_prf_t prf,
    uint8_t          preamble_code
) {
    if (prf == DW3000_PHY_PRF_16_MHZ) {
        return (preamble_code >= 1U) && (preamble_code <= 8U);
    }

    return (preamble_code >= 9U) && (preamble_code <= 29U);
}

static uint16_t dw3000_hal_phy_build_chan_ctrl(const dw3000_phy_config_t* config) {
    return (uint16_t)(((uint16_t)config->channel & 0x1U) |
                      (((uint16_t)config->sfd_type & 0x3U) << DW3000_PHY_CHAN_SFD_TYPE_SHIFT) |
                      (((uint16_t)config->tx_preamble_code & 0x1FU) << DW3000_PHY_CHAN_TX_PCODE_SHIFT) |
                      (((uint16_t)config->rx_preamble_code & 0x1FU) << DW3000_PHY_CHAN_RX_PCODE_SHIFT));
}

static uint32_t dw3000_hal_phy_build_sys_cfg_bits(const dw3000_phy_config_t* config) {
    uint32_t value = 0U;

    if (config->phr_mode == DW3000_PHY_PHR_MODE_EXTENDED) {
        value |= DW3000_PHY_SYS_CFG_PHR_MODE_BIT;
    }

    if (config->phr_rate == DW3000_PHY_PHR_RATE_6M81) {
        value |= DW3000_PHY_SYS_CFG_PHR_6M8_BIT;
    }

    return value;
}

static uint32_t dw3000_hal_phy_build_tx_fctrl_bits(const dw3000_phy_config_t* config) {
    uint32_t value = ((uint32_t)config->preamble_length << DW3000_PHY_TX_FCTRL_TXPSR_SHIFT);

    if (config->data_rate == DW3000_PHY_DATA_RATE_6M81) {
        value |= DW3000_PHY_TX_FCTRL_TXBR_BIT;
    }

    return value;
}

static uint32_t dw3000_hal_phy_rf_tx_ctrl_2(dw3000_phy_channel_t channel) {
    return (channel == DW3000_PHY_CHANNEL_9) ? DW3000_PHY_RF_TX_CTRL_2_CH9 : DW3000_PHY_RF_TX_CTRL_2_CH5;
}

static uint16_t dw3000_hal_phy_pll_cfg(dw3000_phy_channel_t channel) {
    return (channel == DW3000_PHY_CHANNEL_9) ? DW3000_PHY_PLL_CFG_CH9 : DW3000_PHY_PLL_CFG_CH5;
}

static dw3000_error_t dw3000_hal_phy_write_sys_cfg(
    dw3000_device_t*           device,
    const dw3000_phy_config_t* config
) {
    dw3000_error_t err;
    uint32_t       current;
    uint32_t       value;

    if (device->sys_cfg_cache_valid) {
        current = device->sys_cfg_cache;
    } else {
        err = dw3000_reg_read_u32(device, DW3000_REG_SYS_CFG, &current);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    value = (current & ~DW3000_PHY_SYS_CFG_MASK) |
            dw3000_hal_phy_build_sys_cfg_bits(config);

    err = dw3000_reg_write_u32(device, DW3000_REG_SYS_CFG, value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->sys_cfg_cache       = value;
    device->sys_cfg_cache_valid = true;

    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_phy_write_tx_fctrl(
    dw3000_device_t*           device,
    const dw3000_phy_config_t* config
) {
    uint32_t value = dw3000_hal_phy_build_tx_fctrl_bits(config);

    return dw3000_reg_modify_u32(
        device,
        DW3000_REG_TX_FCTRL,
        DW3000_PHY_TX_FCTRL_MASK,
        value
    );
}

static dw3000_error_t dw3000_hal_phy_write_dtune0(
    dw3000_device_t*           device,
    const dw3000_phy_config_t* config
) {
    dw3000_error_t err;
    uint16_t       value;

    err = dw3000_reg_read_u16(device, DW3000_REG_DTUNE0, &value);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    value = (uint16_t)((value & ~(DW3000_PHY_DTUNE0_PAC_MASK | DW3000_PHY_DTUNE0_DT0B4_BIT)) |
                       ((uint16_t)config->pac_size & DW3000_PHY_DTUNE0_PAC_MASK));

    return dw3000_reg_write_u16(device, DW3000_REG_DTUNE0, value);
}

static dw3000_error_t dw3000_hal_phy_write_dgc_cfg(
    dw3000_device_t*               device,
    const dw3000_phy_config_t*     phy_config,
    const dw3000_rx_tune_config_t* rx_tune_config
) {
    dw3000_error_t err;
    uint16_t       current;
    uint16_t       dgc_cfg = (uint16_t)rx_tune_config->dgc_cfg;

    if (phy_config->prf == DW3000_PHY_PRF_16_MHZ) {
        dgc_cfg = (uint16_t)(dgc_cfg & ~DW3000_PHY_DGC_CFG_RX_TUNE_EN);
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_DGC_CFG, &current);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    current = (uint16_t)((current & ~DW3000_PHY_DGC_CFG_MASK) |
                         (dgc_cfg & DW3000_PHY_DGC_CFG_MASK));

    return dw3000_reg_write_u16(device, DW3000_REG_DGC_CFG, current);
}

static dw3000_error_t dw3000_hal_phy_write_rx_tune(
    dw3000_device_t*               device,
    const dw3000_phy_config_t*     phy_config,
    const dw3000_rx_tune_config_t* rx_tune_config
) {
    dw3000_error_t err;

    err = dw3000_hal_phy_write_dtune0(device, phy_config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_RX_SFD_TOC, rx_tune_config->sfd_toc);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_PRE_TOC, rx_tune_config->pre_toc);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u32(device, DW3000_REG_DTUNE3, rx_tune_config->dtune3);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_phy_write_dgc_cfg(device, phy_config, rx_tune_config);
}

dw3000_error_t dw3000_hal_phy_validate_config(const dw3000_phy_config_t* config) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->channel != DW3000_PHY_CHANNEL_5) &&
        (config->channel != DW3000_PHY_CHANNEL_9)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->prf != DW3000_PHY_PRF_16_MHZ) &&
        (config->prf != DW3000_PHY_PRF_64_MHZ)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->data_rate != DW3000_PHY_DATA_RATE_850K) &&
        (config->data_rate != DW3000_PHY_DATA_RATE_6M81)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_phy_is_valid_preamble_length(config->preamble_length)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((uint32_t)config->sfd_type > (uint32_t)DW3000_PHY_SFD_TYPE_IEEE_802154Z) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((uint32_t)config->pac_size > (uint32_t)DW3000_PHY_PAC_SIZE_4) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->phr_mode != DW3000_PHY_PHR_MODE_STANDARD) &&
        (config->phr_mode != DW3000_PHY_PHR_MODE_EXTENDED)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->phr_rate != DW3000_PHY_PHR_RATE_850K) &&
        (config->phr_rate != DW3000_PHY_PHR_RATE_6M81)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_phy_preamble_code_matches_prf(config->prf, config->tx_preamble_code) ||
        !dw3000_hal_phy_preamble_code_matches_prf(config->prf, config->rx_preamble_code)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_phy_validate_rx_tune(
    const dw3000_rx_tune_config_t* config
) {
    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->sfd_toc == 0U) || (config->dtune3 == 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

uint16_t dw3000_hal_phy_preamble_symbols(
    dw3000_phy_preamble_length_t preamble_length
) {
    switch (preamble_length) {
        case DW3000_PHY_PREAMBLE_LEN_32:
            return 32U;
        case DW3000_PHY_PREAMBLE_LEN_64:
            return 64U;
        case DW3000_PHY_PREAMBLE_LEN_128:
            return 128U;
        case DW3000_PHY_PREAMBLE_LEN_256:
            return 256U;
        case DW3000_PHY_PREAMBLE_LEN_512:
            return 512U;
        case DW3000_PHY_PREAMBLE_LEN_1024:
            return 1024U;
        case DW3000_PHY_PREAMBLE_LEN_1536:
            return 1536U;
        case DW3000_PHY_PREAMBLE_LEN_2048:
            return 2048U;
        case DW3000_PHY_PREAMBLE_LEN_4096:
            return 4096U;
        default:
            return 0U;
    }
}

uint8_t dw3000_hal_phy_pac_symbols(dw3000_phy_pac_size_t pac_size) {
    switch (pac_size) {
        case DW3000_PHY_PAC_SIZE_8:
            return 8U;
        case DW3000_PHY_PAC_SIZE_16:
            return 16U;
        case DW3000_PHY_PAC_SIZE_32:
            return 32U;
        case DW3000_PHY_PAC_SIZE_4:
            return 4U;
        default:
            return 0U;
    }
}

uint8_t dw3000_hal_phy_sfd_symbols(dw3000_phy_sfd_type_t sfd_type) {
    if (sfd_type == DW3000_PHY_SFD_TYPE_DECAWAVE_16) {
        return 16U;
    }

    if ((uint32_t)sfd_type <= (uint32_t)DW3000_PHY_SFD_TYPE_IEEE_802154Z) {
        return 8U;
    }

    return 0U;
}

uint16_t dw3000_hal_phy_sfd_timeout(const dw3000_phy_config_t* config) {
    uint16_t preamble_symbols;
    uint8_t  pac_symbols;
    uint8_t  sfd_symbols;

    if (dw3000_hal_phy_validate_config(config) != DW3000_ERROR_OK) {
        return 0U;
    }

    preamble_symbols = dw3000_hal_phy_preamble_symbols(config->preamble_length);
    pac_symbols      = dw3000_hal_phy_pac_symbols(config->pac_size);
    sfd_symbols      = dw3000_hal_phy_sfd_symbols(config->sfd_type);

    return (uint16_t)(preamble_symbols + 1U + sfd_symbols - pac_symbols);
}

dw3000_error_t dw3000_hal_phy_configure(
    dw3000_device_t*               device,
    const dw3000_phy_config_t*     phy_config,
    const dw3000_rx_tune_config_t* rx_tune_config
) {
    dw3000_error_t          err;
    dw3000_rx_tune_config_t effective_rx_tune;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_phy_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (phy_config == NULL) {
        phy_config = &device->config.phy;
    }

    if (rx_tune_config == NULL) {
        rx_tune_config = &device->config.rx_tune;
    }

    err = dw3000_hal_phy_validate_config(phy_config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_phy_validate_rx_tune(rx_tune_config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    effective_rx_tune = *rx_tune_config;
    if (phy_config->prf == DW3000_PHY_PRF_16_MHZ) {
        effective_rx_tune.dgc_cfg = (dw3000_rx_tune_dgc_cfg_t)(effective_rx_tune.dgc_cfg & ~DW3000_PHY_DGC_CFG_RX_TUNE_EN);
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_CHAN_CTRL,
        dw3000_hal_phy_build_chan_ctrl(phy_config)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_phy_write_sys_cfg(device, phy_config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_phy_write_tx_fctrl(device, phy_config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_RF_TX_CTRL_2,
        dw3000_hal_phy_rf_tx_ctrl_2(phy_config->channel)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u8(
        device,
        DW3000_REG_RF_TX_CTRL_1,
        DW3000_RF_TX_CTRL_1_OPT
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u8(
        device,
        DW3000_REG_LDO_RLOAD,
        DW3000_RF_LDO_RLOAD_OPT
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_PLL_CFG,
        dw3000_hal_phy_pll_cfg(phy_config->channel)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(device, DW3000_REG_PLL_CAL, DW3000_PHY_PLL_CAL_CFG_LD);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_phy_write_rx_tune(device, phy_config, &effective_rx_tune);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.phy     = *phy_config;
    device->config.rx_tune = effective_rx_tune;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_phy_configure_current(dw3000_device_t* device) {
    return dw3000_hal_phy_configure(device, NULL, NULL);
}
