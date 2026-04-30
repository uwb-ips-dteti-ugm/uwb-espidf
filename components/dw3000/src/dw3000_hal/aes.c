#include "dw3000_hal/aes.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dw3000_hal/status.h"
#include "dw3000_register.h"

#define DW3000_HAL_AES_READY_POLL_DELAY_US 100U

#define DW3000_HAL_AES_CFG_MODE_MASK      0x0001U
#define DW3000_HAL_AES_CFG_KEY_SIZE_SHIFT 1U
#define DW3000_HAL_AES_CFG_KEY_SIZE_MASK  0x0006U
#define DW3000_HAL_AES_CFG_KEY_ADDR_SHIFT 3U
#define DW3000_HAL_AES_CFG_KEY_ADDR_MASK  0x0038U
#define DW3000_HAL_AES_CFG_KEY_LOAD_MASK  ((uint16_t)DW3000_AES_CFG_KEY_LOAD)
#define DW3000_HAL_AES_CFG_KEY_SRC_SHIFT  7U
#define DW3000_HAL_AES_CFG_KEY_SRC_MASK   0x0080U
#define DW3000_HAL_AES_CFG_TAG_SIZE_SHIFT 8U
#define DW3000_HAL_AES_CFG_TAG_SIZE_MASK  0x0700U
#define DW3000_HAL_AES_CFG_CORE_SHIFT     11U
#define DW3000_HAL_AES_CFG_CORE_MASK      0x0800U
#define DW3000_HAL_AES_CFG_KEY_OTP_MASK   ((uint16_t)DW3000_AES_CFG_KEY_OTP)
#define DW3000_HAL_AES_CFG_FLAGS_MASK \
    (DW3000_HAL_AES_CFG_KEY_LOAD_MASK | DW3000_HAL_AES_CFG_KEY_OTP_MASK)

#define DW3000_HAL_AES_DMA_PORT_MASK       0x7UL
#define DW3000_HAL_AES_DMA_ADDR_MASK       0x03FFUL
#define DW3000_HAL_AES_DMA_SRC_ADDR_SHIFT  3U
#define DW3000_HAL_AES_DMA_DST_PORT_SHIFT  13U
#define DW3000_HAL_AES_DMA_DST_ADDR_SHIFT  16U
#define DW3000_HAL_AES_DMA_END_SHIFT       26U
#define DW3000_HAL_AES_DMA_HDR_SIZE_MASK   0x7FU
#define DW3000_HAL_AES_DMA_PYLD_SIZE_MASK  0x03FFU
#define DW3000_HAL_AES_DMA_PYLD_SIZE_SHIFT 7U
#define DW3000_HAL_AES_KEY_WORD_SIZE       4U

#define DW3000_HAL_AES_STATUS_MASK        \
    ((uint32_t)DW3000_AES_STS_AES_DONE |  \
     (uint32_t)DW3000_AES_STS_AUTH_ERR |  \
     (uint32_t)DW3000_AES_STS_TRANS_ERR | \
     (uint32_t)DW3000_AES_STS_MEM_CONF |  \
     (uint32_t)DW3000_AES_STS_RAM_EMPTY | \
     (uint32_t)DW3000_AES_STS_RAM_FULL)

#define DW3000_HAL_REG_AES_IV \
    DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_1, 0x0034U, 16U)

static bool dw3000_hal_aes_can_access(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static void dw3000_hal_aes_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static uint32_t dw3000_hal_aes_min_u32(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static bool dw3000_hal_aes_mode_is_valid(dw3000_aes_mode_t mode) {
    return (mode == DW3000_AES_MODE_ENCRYPT) ||
           (mode == DW3000_AES_MODE_DECRYPT);
}

static bool dw3000_hal_aes_key_size_is_valid(dw3000_aes_key_size_t key_size) {
    return (key_size == DW3000_AES_KEY_SIZE_128) ||
           (key_size == DW3000_AES_KEY_SIZE_192) ||
           (key_size == DW3000_AES_KEY_SIZE_256);
}

static bool dw3000_hal_aes_tag_size_is_valid(dw3000_aes_tag_size_t tag_size) {
    return tag_size <= DW3000_AES_TAG_SIZE_16;
}

static bool dw3000_hal_aes_core_is_valid(dw3000_aes_core_t core) {
    return (core == DW3000_AES_CORE_GCM) || (core == DW3000_AES_CORE_CCM);
}

static bool dw3000_hal_aes_key_src_is_valid(dw3000_aes_key_src_t key_src) {
    return (key_src == DW3000_AES_KEY_SRC_REGISTER) ||
           (key_src == DW3000_AES_KEY_SRC_RAM);
}

static bool dw3000_hal_aes_endianness_is_valid(
    dw3000_aes_endianness_t endianness
) {
    return (endianness == DW3000_AES_ENDIAN_BIG) ||
           (endianness == DW3000_AES_ENDIAN_LITTLE);
}

static bool dw3000_hal_aes_src_port_is_valid(dw3000_aes_port_t port) {
    return (port == DW3000_AES_PORT_SCRATCH) ||
           (port == DW3000_AES_PORT_RX_0) ||
           (port == DW3000_AES_PORT_RX_1) ||
           (port == DW3000_AES_PORT_TX);
}

static bool dw3000_hal_aes_dst_port_is_valid(dw3000_aes_port_t port) {
    return dw3000_hal_aes_src_port_is_valid(port) ||
           (port == DW3000_AES_PORT_STS_KEY);
}

static size_t dw3000_hal_aes_port_size(dw3000_aes_port_t port) {
    switch (port) {
        case DW3000_AES_PORT_SCRATCH:
            return DW3000_AES_SCRATCH_RAM_SIZE;

        case DW3000_AES_PORT_RX_0:
        case DW3000_AES_PORT_RX_1:
        case DW3000_AES_PORT_TX:
            return DW3000_AES_FRAME_BUFFER_SIZE;

        case DW3000_AES_PORT_STS_KEY:
            return DW3000_AES_STS_KEY_SIZE;

        default:
            return 0U;
    }
}

static uint16_t dw3000_hal_aes_encode_cfg(
    const dw3000_aes_config_t* config
) {
    return ((uint16_t)config->mode & DW3000_HAL_AES_CFG_MODE_MASK) |
           (((uint16_t)config->key_size << DW3000_HAL_AES_CFG_KEY_SIZE_SHIFT) &
            DW3000_HAL_AES_CFG_KEY_SIZE_MASK) |
           (((uint16_t)config->key_addr << DW3000_HAL_AES_CFG_KEY_ADDR_SHIFT) &
            DW3000_HAL_AES_CFG_KEY_ADDR_MASK) |
           ((uint16_t)config->flags & DW3000_HAL_AES_CFG_FLAGS_MASK) |
           (((uint16_t)config->key_src << DW3000_HAL_AES_CFG_KEY_SRC_SHIFT) &
            DW3000_HAL_AES_CFG_KEY_SRC_MASK) |
           (((uint16_t)config->tag_size << DW3000_HAL_AES_CFG_TAG_SIZE_SHIFT) &
            DW3000_HAL_AES_CFG_TAG_SIZE_MASK) |
           (((uint16_t)config->core << DW3000_HAL_AES_CFG_CORE_SHIFT) &
            DW3000_HAL_AES_CFG_CORE_MASK);
}

static void dw3000_hal_aes_decode_cfg(
    uint16_t             raw,
    dw3000_aes_config_t* config
) {
    config->mode     = (dw3000_aes_mode_t)(raw & DW3000_HAL_AES_CFG_MODE_MASK);
    config->key_size = (dw3000_aes_key_size_t)((raw & DW3000_HAL_AES_CFG_KEY_SIZE_MASK) >>
                                               DW3000_HAL_AES_CFG_KEY_SIZE_SHIFT);
    config->key_addr = (uint8_t)((raw & DW3000_HAL_AES_CFG_KEY_ADDR_MASK) >>
                                 DW3000_HAL_AES_CFG_KEY_ADDR_SHIFT);
    config->flags    = (dw3000_aes_cfg_flags_t)(raw & DW3000_HAL_AES_CFG_FLAGS_MASK);
    config->key_src  = (dw3000_aes_key_src_t)((raw & DW3000_HAL_AES_CFG_KEY_SRC_MASK) >>
                                             DW3000_HAL_AES_CFG_KEY_SRC_SHIFT);
    config->tag_size = (dw3000_aes_tag_size_t)((raw & DW3000_HAL_AES_CFG_TAG_SIZE_MASK) >>
                                               DW3000_HAL_AES_CFG_TAG_SIZE_SHIFT);
    config->core     = (dw3000_aes_core_t)((raw & DW3000_HAL_AES_CFG_CORE_MASK) >> DW3000_HAL_AES_CFG_CORE_SHIFT);
}

static void dw3000_hal_aes_pack_dma(
    const dw3000_aes_dma_cfg_t* dma,
    uint8_t                     raw[8]
) {
    uint32_t low = ((uint32_t)dma->src_port & DW3000_HAL_AES_DMA_PORT_MASK) |
                   (((uint32_t)dma->src_addr & DW3000_HAL_AES_DMA_ADDR_MASK) << DW3000_HAL_AES_DMA_SRC_ADDR_SHIFT) |
                   (((uint32_t)dma->dst_port & DW3000_HAL_AES_DMA_PORT_MASK) << DW3000_HAL_AES_DMA_DST_PORT_SHIFT) |
                   (((uint32_t)dma->dst_addr & DW3000_HAL_AES_DMA_ADDR_MASK) << DW3000_HAL_AES_DMA_DST_ADDR_SHIFT) |
                   (((uint32_t)dma->endianness & 0x1UL) << DW3000_HAL_AES_DMA_END_SHIFT);
    uint32_t high = ((uint32_t)dma->hdr_size & DW3000_HAL_AES_DMA_HDR_SIZE_MASK) |
                    (((uint32_t)dma->pyld_size & DW3000_HAL_AES_DMA_PYLD_SIZE_MASK) << DW3000_HAL_AES_DMA_PYLD_SIZE_SHIFT);

    raw[0] = (uint8_t)(low & 0xFFU);
    raw[1] = (uint8_t)((low >> 8U) & 0xFFU);
    raw[2] = (uint8_t)((low >> 16U) & 0xFFU);
    raw[3] = (uint8_t)((low >> 24U) & 0xFFU);
    raw[4] = (uint8_t)(high & 0xFFU);
    raw[5] = (uint8_t)((high >> 8U) & 0xFFU);
    raw[6] = (uint8_t)((high >> 16U) & 0xFFU);
    raw[7] = (uint8_t)((high >> 24U) & 0xFFU);
}

static void dw3000_hal_aes_unpack_dma(
    const uint8_t         raw[8],
    dw3000_aes_dma_cfg_t* dma
) {
    uint32_t low = (uint32_t)raw[0] |
                   ((uint32_t)raw[1] << 8U) |
                   ((uint32_t)raw[2] << 16U) |
                   ((uint32_t)raw[3] << 24U);
    uint32_t high = (uint32_t)raw[4] |
                    ((uint32_t)raw[5] << 8U) |
                    ((uint32_t)raw[6] << 16U) |
                    ((uint32_t)raw[7] << 24U);

    dma->src_port   = (dw3000_aes_port_t)(low & DW3000_HAL_AES_DMA_PORT_MASK);
    dma->src_addr   = (uint16_t)((low >> DW3000_HAL_AES_DMA_SRC_ADDR_SHIFT) &
                               DW3000_HAL_AES_DMA_ADDR_MASK);
    dma->dst_port   = (dw3000_aes_port_t)((low >> DW3000_HAL_AES_DMA_DST_PORT_SHIFT) &
                                        DW3000_HAL_AES_DMA_PORT_MASK);
    dma->dst_addr   = (uint16_t)((low >> DW3000_HAL_AES_DMA_DST_ADDR_SHIFT) &
                               DW3000_HAL_AES_DMA_ADDR_MASK);
    dma->endianness = (dw3000_aes_endianness_t)((low >> DW3000_HAL_AES_DMA_END_SHIFT) & 0x1UL);
    dma->hdr_size   = (uint8_t)(high & DW3000_HAL_AES_DMA_HDR_SIZE_MASK);
    dma->pyld_size  = (uint16_t)((high >> DW3000_HAL_AES_DMA_PYLD_SIZE_SHIFT) &
                                DW3000_HAL_AES_DMA_PYLD_SIZE_MASK);
}

static dw3000_error_t dw3000_hal_aes_validate_ram_range(
    uint16_t offset,
    size_t   data_len,
    size_t   size
) {
    if (data_len == 0U) {
        return DW3000_ERROR_OK;
    }

    if (((size_t)offset >= size) || (data_len > (size - (size_t)offset))) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    return DW3000_ERROR_OK;
}

static void dw3000_hal_aes_key_to_key_ram(
    const dw3000_aes_key_t* key,
    uint8_t                 raw[DW3000_AES_KEY_RAM_SLOT_SIZE]
) {
    for (uint8_t word = 0U; word < 4U; ++word) {
        memcpy(
            &raw[word * DW3000_HAL_AES_KEY_WORD_SIZE],
            &key->bytes[(3U - word) * DW3000_HAL_AES_KEY_WORD_SIZE],
            DW3000_HAL_AES_KEY_WORD_SIZE
        );
    }
}

static void dw3000_hal_aes_key_from_key_ram(
    const uint8_t     raw[DW3000_AES_KEY_RAM_SLOT_SIZE],
    dw3000_aes_key_t* key
) {
    for (uint8_t word = 0U; word < 4U; ++word) {
        memcpy(
            &key->bytes[(3U - word) * DW3000_HAL_AES_KEY_WORD_SIZE],
            &raw[word * DW3000_HAL_AES_KEY_WORD_SIZE],
            DW3000_HAL_AES_KEY_WORD_SIZE
        );
    }
}

size_t dw3000_hal_aes_tag_size_bytes(dw3000_aes_tag_size_t tag_size) {
    switch (tag_size) {
        case DW3000_AES_TAG_SIZE_NONE:
            return 0U;
        case DW3000_AES_TAG_SIZE_4:
            return 4U;
        case DW3000_AES_TAG_SIZE_6:
            return 6U;
        case DW3000_AES_TAG_SIZE_8:
            return 8U;
        case DW3000_AES_TAG_SIZE_10:
            return 10U;
        case DW3000_AES_TAG_SIZE_12:
            return 12U;
        case DW3000_AES_TAG_SIZE_14:
            return 14U;
        case DW3000_AES_TAG_SIZE_16:
            return 16U;
        default:
            return 0U;
    }
}

size_t dw3000_hal_aes_key_size_bytes(dw3000_aes_key_size_t key_size) {
    switch (key_size) {
        case DW3000_AES_KEY_SIZE_128:
            return 16U;
        case DW3000_AES_KEY_SIZE_192:
            return 24U;
        case DW3000_AES_KEY_SIZE_256:
            return 32U;
        default:
            return 0U;
    }
}

dw3000_error_t dw3000_hal_aes_validate_dma_cfg(
    const dw3000_aes_dma_cfg_t* dma
) {
    if (dma == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_src_port_is_valid(dma->src_port) ||
        !dw3000_hal_aes_dst_port_is_valid(dma->dst_port) ||
        !dw3000_hal_aes_endianness_is_valid(dma->endianness)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (((uint16_t)dma->src_addr & ~DW3000_HAL_AES_DMA_ADDR_MASK) != 0U ||
        ((uint16_t)dma->dst_addr & ~DW3000_HAL_AES_DMA_ADDR_MASK) != 0U ||
        ((uint8_t)dma->hdr_size & ~DW3000_HAL_AES_DMA_HDR_SIZE_MASK) != 0U ||
        ((uint16_t)dma->pyld_size & ~DW3000_HAL_AES_DMA_PYLD_SIZE_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_validate_transfer(
    const dw3000_aes_config_t* config
) {
    size_t tag_size;
    size_t total_size;
    size_t src_size;
    size_t dst_size;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    tag_size   = dw3000_hal_aes_tag_size_bytes(config->tag_size);
    total_size = (size_t)config->dma.hdr_size +
                 (size_t)config->dma.pyld_size +
                 tag_size;
    src_size = dw3000_hal_aes_port_size(config->dma.src_port);
    dst_size = dw3000_hal_aes_port_size(config->dma.dst_port);

    if ((total_size != 0U) &&
        (((size_t)config->dma.src_addr >= src_size) ||
         ((size_t)config->dma.dst_addr >= dst_size))) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    if (((size_t)config->dma.src_addr + total_size) > src_size ||
        ((size_t)config->dma.dst_addr + total_size) > dst_size) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_validate_cfg(
    const dw3000_aes_config_t* config
) {
    dw3000_error_t err;
    size_t         key_size;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_mode_is_valid(config->mode) ||
        !dw3000_hal_aes_key_size_is_valid(config->key_size) ||
        !dw3000_hal_aes_tag_size_is_valid(config->tag_size) ||
        !dw3000_hal_aes_core_is_valid(config->core) ||
        !dw3000_hal_aes_key_src_is_valid(config->key_src) ||
        ((uint16_t)config->flags & ~DW3000_HAL_AES_CFG_FLAGS_MASK) != 0U ||
        config->key_addr >= DW3000_AES_KEY_RAM_SLOTS) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((config->core == DW3000_AES_CORE_CCM) &&
        (config->key_size != DW3000_AES_KEY_SIZE_128)) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    if ((config->key_src == DW3000_AES_KEY_SRC_REGISTER) &&
        (((uint16_t)config->flags & (uint16_t)DW3000_AES_CFG_KEY_OTP) == 0U) &&
        (config->key_size != DW3000_AES_KEY_SIZE_128)) {
        return DW3000_ERROR_NOT_SUPPORTED;
    }

    key_size = dw3000_hal_aes_key_size_bytes(config->key_size);
    if ((config->key_src == DW3000_AES_KEY_SRC_RAM) &&
        (((size_t)config->key_addr * DW3000_AES_KEY_RAM_SLOT_SIZE + key_size) >
         (DW3000_AES_KEY_RAM_SLOTS * DW3000_AES_KEY_RAM_SLOT_SIZE))) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    err = dw3000_hal_aes_validate_dma_cfg(&config->dma);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_aes_validate_transfer(config);
}

dw3000_error_t dw3000_hal_aes_read_cfg(
    dw3000_device_t*     device,
    dw3000_aes_config_t* config
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (config == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    *config = device->config.aes;

    err = dw3000_reg_read_u16(device, DW3000_REG_AES_CFG, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aes_decode_cfg(raw, config);

    err = dw3000_hal_aes_read_dma_cfg(device, &config->dma);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aes_read_iv(device, &config->iv);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if ((config->key_src == DW3000_AES_KEY_SRC_REGISTER) &&
        (((uint16_t)config->flags & (uint16_t)DW3000_AES_CFG_KEY_OTP) == 0U)) {
        err = dw3000_hal_aes_read_key(device, &config->key);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_write_cfg(
    dw3000_device_t*           device,
    const dw3000_aes_config_t* config
) {
    if ((device == NULL) || (config == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u16(
        device,
        DW3000_REG_AES_CFG,
        dw3000_hal_aes_encode_cfg(config)
    );
}

dw3000_error_t dw3000_hal_aes_read_dma_cfg(
    dw3000_device_t*      device,
    dw3000_aes_dma_cfg_t* dma
) {
    dw3000_error_t err;
    uint8_t        raw[8];

    if ((device == NULL) || (dma == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_DMA_CFG, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aes_unpack_dma(raw, dma);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_write_dma_cfg(
    dw3000_device_t*            device,
    const dw3000_aes_dma_cfg_t* dma
) {
    dw3000_error_t err;
    uint8_t        raw[8];

    if ((device == NULL) || (dma == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_aes_validate_dma_cfg(dma);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aes_pack_dma(dma, raw);
    return dw3000_reg_write(device, DW3000_REG_DMA_CFG, raw, sizeof(raw));
}

dw3000_error_t dw3000_hal_aes_read_iv(
    dw3000_device_t* device,
    dw3000_aes_iv_t* iv
) {
    if ((device == NULL) || (iv == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read(device, DW3000_HAL_REG_AES_IV, iv->bytes, sizeof(iv->bytes));
}

dw3000_error_t dw3000_hal_aes_write_iv(
    dw3000_device_t*       device,
    const dw3000_aes_iv_t* iv
) {
    if ((device == NULL) || (iv == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write(device, DW3000_HAL_REG_AES_IV, iv->bytes, sizeof(iv->bytes));
}

dw3000_error_t dw3000_hal_aes_build_ccm_iv(
    const uint8_t*   nonce13,
    uint16_t         payload_size,
    dw3000_aes_iv_t* iv
) {
    if ((nonce13 == NULL) || (iv == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    memset(iv->bytes, 0, sizeof(iv->bytes));
    iv->bytes[0]  = nonce13[10];
    iv->bytes[1]  = nonce13[9];
    iv->bytes[2]  = nonce13[8];
    iv->bytes[3]  = nonce13[7];
    iv->bytes[4]  = nonce13[6];
    iv->bytes[5]  = nonce13[5];
    iv->bytes[6]  = nonce13[4];
    iv->bytes[7]  = nonce13[3];
    iv->bytes[8]  = nonce13[2];
    iv->bytes[9]  = nonce13[1];
    iv->bytes[10] = nonce13[0];
    iv->bytes[12] = (uint8_t)(payload_size & 0xFFU);
    iv->bytes[13] = (uint8_t)((payload_size >> 8U) & 0xFFU);
    iv->bytes[14] = nonce13[12];
    iv->bytes[15] = nonce13[11];

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_write_ccm_nonce(
    dw3000_device_t* device,
    const uint8_t*   nonce13,
    uint16_t         payload_size
) {
    dw3000_error_t  err;
    dw3000_aes_iv_t iv;

    err = dw3000_hal_aes_build_ccm_iv(nonce13, payload_size, &iv);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_aes_write_iv(device, &iv);
}

dw3000_error_t dw3000_hal_aes_write_gcm_iv_96(
    dw3000_device_t* device,
    const uint8_t*   iv96
) {
    dw3000_aes_iv_t iv = {0};

    if (iv96 == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    memcpy(iv.bytes, iv96, 12U);
    return dw3000_hal_aes_write_iv(device, &iv);
}

dw3000_error_t dw3000_hal_aes_read_key(
    dw3000_device_t*  device,
    dw3000_aes_key_t* key
) {
    if ((device == NULL) || (key == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_reg_read(device, DW3000_REG_AES_KEY, key->bytes, sizeof(key->bytes));
}

dw3000_error_t dw3000_hal_aes_write_key(
    dw3000_device_t*        device,
    const dw3000_aes_key_t* key
) {
    if ((device == NULL) || (key == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write(device, DW3000_REG_AES_KEY, key->bytes, sizeof(key->bytes));
}

dw3000_error_t dw3000_hal_aes_read_key_ram(
    dw3000_device_t* device,
    uint16_t         offset,
    void*            data,
    size_t           data_len
) {
    dw3000_error_t    err;
    dw3000_reg_desc_t reg = DW3000_REG_AES_KEY_RAM;

    if ((device == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_aes_validate_ram_range(
        offset,
        data_len,
        DW3000_AES_KEY_RAM_SLOTS * DW3000_AES_KEY_RAM_SLOT_SIZE
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reg.offset = offset;
    reg.length = (DW3000_AES_KEY_RAM_SLOTS * DW3000_AES_KEY_RAM_SLOT_SIZE) -
                 (size_t)offset;
    return dw3000_reg_read(device, reg, data, data_len);
}

dw3000_error_t dw3000_hal_aes_write_key_ram(
    dw3000_device_t* device,
    uint16_t         offset,
    const void*      data,
    size_t           data_len
) {
    dw3000_error_t    err;
    dw3000_reg_desc_t reg = DW3000_REG_AES_KEY_RAM;

    if ((device == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_aes_validate_ram_range(
        offset,
        data_len,
        DW3000_AES_KEY_RAM_SLOTS * DW3000_AES_KEY_RAM_SLOT_SIZE
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reg.offset = offset;
    reg.length = (DW3000_AES_KEY_RAM_SLOTS * DW3000_AES_KEY_RAM_SLOT_SIZE) -
                 (size_t)offset;
    return dw3000_reg_write(device, reg, data, data_len);
}

dw3000_error_t dw3000_hal_aes_read_key_ram_slot(
    dw3000_device_t*  device,
    uint8_t           slot,
    dw3000_aes_key_t* key
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_AES_KEY_RAM_SLOT_SIZE];

    if ((slot >= DW3000_AES_KEY_RAM_SLOTS) || (key == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_aes_read_key_ram(
        device,
        (uint16_t)(slot * DW3000_AES_KEY_RAM_SLOT_SIZE),
        raw,
        sizeof(raw)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_aes_key_from_key_ram(raw, key);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_write_key_ram_slot(
    dw3000_device_t*        device,
    uint8_t                 slot,
    const dw3000_aes_key_t* key
) {
    uint8_t raw[DW3000_AES_KEY_RAM_SLOT_SIZE];

    if ((slot >= DW3000_AES_KEY_RAM_SLOTS) || (key == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    dw3000_hal_aes_key_to_key_ram(key, raw);
    return dw3000_hal_aes_write_key_ram(
        device,
        (uint16_t)(slot * DW3000_AES_KEY_RAM_SLOT_SIZE),
        raw,
        sizeof(raw)
    );
}

dw3000_error_t dw3000_hal_aes_read_scratch(
    dw3000_device_t* device,
    uint16_t         offset,
    void*            data,
    size_t           data_len
) {
    dw3000_error_t    err;
    dw3000_reg_desc_t reg = DW3000_REG_SCRATCH_RAM;

    if ((device == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_aes_validate_ram_range(offset, data_len, DW3000_AES_SCRATCH_RAM_SIZE);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reg.offset = offset;
    reg.length = DW3000_AES_SCRATCH_RAM_SIZE - (size_t)offset;
    return dw3000_reg_read(device, reg, data, data_len);
}

dw3000_error_t dw3000_hal_aes_write_scratch(
    dw3000_device_t* device,
    uint16_t         offset,
    const void*      data,
    size_t           data_len
) {
    dw3000_error_t    err;
    dw3000_reg_desc_t reg = DW3000_REG_SCRATCH_RAM;

    if ((device == NULL) || ((data_len != 0U) && (data == NULL))) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_hal_aes_validate_ram_range(offset, data_len, DW3000_AES_SCRATCH_RAM_SIZE);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    reg.offset = offset;
    reg.length = DW3000_AES_SCRATCH_RAM_SIZE - (size_t)offset;
    return dw3000_reg_write(device, reg, data, data_len);
}

dw3000_error_t dw3000_hal_aes_read_status(
    dw3000_device_t*     device,
    dw3000_aes_status_t* status
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_AES_STS, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *status = (dw3000_aes_status_t)(raw & DW3000_HAL_AES_STATUS_MASK);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_clear_status(
    dw3000_device_t*    device,
    dw3000_aes_status_t status
) {
    uint32_t bits = (uint32_t)status & DW3000_HAL_AES_STATUS_MASK;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (bits == 0U) {
        return DW3000_ERROR_OK;
    }

    return dw3000_reg_write_u32(device, DW3000_REG_AES_STS, bits);
}

dw3000_error_t dw3000_hal_aes_clear_events(dw3000_device_t* device) {
    dw3000_error_t err;

    err = dw3000_hal_aes_clear_status(
        device,
        (dw3000_aes_status_t)DW3000_HAL_AES_STATUS_MASK
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_status_clear(
        device,
        DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR
    );
}

dw3000_error_t dw3000_hal_aes_enable_events(dw3000_device_t* device) {
    return dw3000_hal_status_enable(
        device,
        DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR
    );
}

dw3000_error_t dw3000_hal_aes_disable_events(dw3000_device_t* device) {
    return dw3000_hal_status_disable(
        device,
        DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR
    );
}

bool dw3000_hal_aes_status_has_error(dw3000_aes_status_t status) {
    return (((uint32_t)status & DW3000_AES_STS_ERROR_MASK) != 0U);
}

dw3000_error_t dw3000_hal_aes_start(dw3000_device_t* device) {
    dw3000_error_t err;
    uint16_t       cfg;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_aes_can_access(device)) {
        return DW3000_ERROR_BUSY;
    }

    err = dw3000_reg_read_u16(device, DW3000_REG_AES_CFG, &cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    cfg = (uint16_t)(cfg | DW3000_HAL_AES_CFG_KEY_LOAD_MASK);
    err = dw3000_reg_write_u16(device, DW3000_REG_AES_CFG, cfg);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_reg_write_u8(device, DW3000_REG_AES_START, DW3000_AES_START_TRIGGER);
}

dw3000_error_t dw3000_hal_aes_wait_done(
    dw3000_device_t*     device,
    uint32_t             timeout_us,
    dw3000_aes_status_t* final_status
) {
    dw3000_error_t      err;
    dw3000_aes_status_t status;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((timeout_us != 0U) &&
        (device->port.get_time_us == NULL) &&
        (device->port.delay_us == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (device->port.get_time_us != NULL) {
        uint64_t start_us = device->port.get_time_us(device->port.ctx);

        do {
            err = dw3000_hal_aes_read_status(device, &status);
            if (err != DW3000_ERROR_OK) {
                return err;
            }

            if (final_status != NULL) {
                *final_status = status;
            }

            if (dw3000_hal_aes_status_has_error(status)) {
                return DW3000_ERROR_INVALID_STATE;
            }

            if (((uint32_t)status & (uint32_t)DW3000_AES_STS_AES_DONE) != 0U) {
                return DW3000_ERROR_OK;
            }

            uint64_t elapsed_us = device->port.get_time_us(device->port.ctx) - start_us;
            if (elapsed_us >= timeout_us) {
                return DW3000_ERROR_TIMEOUT;
            }

            if (device->port.delay_us != NULL) {
                uint32_t remaining_us = timeout_us - (uint32_t)elapsed_us;
                dw3000_hal_aes_delay_us(
                    device,
                    dw3000_hal_aes_min_u32(
                        remaining_us,
                        DW3000_HAL_AES_READY_POLL_DELAY_US
                    )
                );
            }
        } while (true);
    }

    do {
        err = dw3000_hal_aes_read_status(device, &status);
        if (err != DW3000_ERROR_OK) {
            return err;
        }

        if (final_status != NULL) {
            *final_status = status;
        }

        if (dw3000_hal_aes_status_has_error(status)) {
            return DW3000_ERROR_INVALID_STATE;
        }

        if (((uint32_t)status & (uint32_t)DW3000_AES_STS_AES_DONE) != 0U) {
            return DW3000_ERROR_OK;
        }

        if (timeout_us == 0U) {
            return DW3000_ERROR_TIMEOUT;
        }

        uint32_t delay_us = dw3000_hal_aes_min_u32(
            timeout_us,
            DW3000_HAL_AES_READY_POLL_DELAY_US
        );
        dw3000_hal_aes_delay_us(device, delay_us);
        timeout_us -= delay_us;
    } while (true);
}

dw3000_error_t dw3000_hal_aes_configure(
    dw3000_device_t*           device,
    const dw3000_aes_config_t* config
) {
    dw3000_error_t      err;
    dw3000_aes_config_t actual;
    dw3000_aes_config_t cfg_to_write;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    actual = (config != NULL) ? *config : device->config.aes;

    err = dw3000_hal_aes_validate_cfg(&actual);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if ((actual.key_src == DW3000_AES_KEY_SRC_REGISTER) &&
        (((uint16_t)actual.flags & (uint16_t)DW3000_AES_CFG_KEY_OTP) == 0U)) {
        err = dw3000_hal_aes_write_key(device, &actual.key);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    err = dw3000_hal_aes_write_iv(device, &actual.iv);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aes_write_dma_cfg(device, &actual.dma);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    cfg_to_write       = actual;
    cfg_to_write.flags = (dw3000_aes_cfg_flags_t)(cfg_to_write.flags | DW3000_AES_CFG_KEY_LOAD);

    err = dw3000_hal_aes_write_cfg(device, &cfg_to_write);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.aes = actual;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_aes_configure_current(dw3000_device_t* device) {
    return dw3000_hal_aes_configure(device, NULL);
}

dw3000_error_t dw3000_hal_aes_run(
    dw3000_device_t*           device,
    const dw3000_aes_config_t* config,
    uint32_t                   timeout_us,
    dw3000_aes_status_t*       final_status
) {
    dw3000_error_t err;

    err = dw3000_hal_aes_clear_events(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aes_configure(device, config);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_aes_start(device);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_aes_wait_done(device, timeout_us, final_status);
}
