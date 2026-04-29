#ifndef DW3000_HAL_AES_H
#define DW3000_HAL_AES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/aes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_AES_DONE_TIMEOUT_US 10000U

size_t dw3000_hal_aes_tag_size_bytes(dw3000_aes_tag_size_t tag_size);

size_t dw3000_hal_aes_key_size_bytes(dw3000_aes_key_size_t key_size);

dw3000_error_t dw3000_hal_aes_validate_cfg(
    const dw3000_aes_config_t* config
);

dw3000_error_t dw3000_hal_aes_validate_dma_cfg(
    const dw3000_aes_dma_cfg_t* dma
);

dw3000_error_t dw3000_hal_aes_validate_transfer(
    const dw3000_aes_config_t* config
);

dw3000_error_t dw3000_hal_aes_read_cfg(
    dw3000_device_t*     device,
    dw3000_aes_config_t* config
);

dw3000_error_t dw3000_hal_aes_write_cfg(
    dw3000_device_t*           device,
    const dw3000_aes_config_t* config
);

dw3000_error_t dw3000_hal_aes_read_dma_cfg(
    dw3000_device_t*      device,
    dw3000_aes_dma_cfg_t* dma
);

dw3000_error_t dw3000_hal_aes_write_dma_cfg(
    dw3000_device_t*            device,
    const dw3000_aes_dma_cfg_t* dma
);

dw3000_error_t dw3000_hal_aes_read_iv(
    dw3000_device_t* device,
    dw3000_aes_iv_t* iv
);

dw3000_error_t dw3000_hal_aes_write_iv(
    dw3000_device_t*       device,
    const dw3000_aes_iv_t* iv
);

dw3000_error_t dw3000_hal_aes_build_ccm_iv(
    const uint8_t*   nonce13,
    uint16_t         payload_size,
    dw3000_aes_iv_t* iv
);

dw3000_error_t dw3000_hal_aes_write_ccm_nonce(
    dw3000_device_t* device,
    const uint8_t*   nonce13,
    uint16_t         payload_size
);

dw3000_error_t dw3000_hal_aes_write_gcm_iv_96(
    dw3000_device_t* device,
    const uint8_t*   iv96
);

dw3000_error_t dw3000_hal_aes_read_key(
    dw3000_device_t*  device,
    dw3000_aes_key_t* key
);

dw3000_error_t dw3000_hal_aes_write_key(
    dw3000_device_t*        device,
    const dw3000_aes_key_t* key
);

dw3000_error_t dw3000_hal_aes_read_key_ram(
    dw3000_device_t* device,
    uint16_t         offset,
    void*            data,
    size_t           data_len
);

dw3000_error_t dw3000_hal_aes_write_key_ram(
    dw3000_device_t* device,
    uint16_t         offset,
    const void*      data,
    size_t           data_len
);

/* Slot helpers accept/return the same byte layout as AES_KEY. The HAL
   translates the reversed 32-bit word order used by AES_KEY_RAM. */
dw3000_error_t dw3000_hal_aes_read_key_ram_slot(
    dw3000_device_t*  device,
    uint8_t           slot,
    dw3000_aes_key_t* key
);

dw3000_error_t dw3000_hal_aes_write_key_ram_slot(
    dw3000_device_t*        device,
    uint8_t                 slot,
    const dw3000_aes_key_t* key
);

dw3000_error_t dw3000_hal_aes_read_scratch(
    dw3000_device_t* device,
    uint16_t         offset,
    void*            data,
    size_t           data_len
);

dw3000_error_t dw3000_hal_aes_write_scratch(
    dw3000_device_t* device,
    uint16_t         offset,
    const void*      data,
    size_t           data_len
);

dw3000_error_t dw3000_hal_aes_read_status(
    dw3000_device_t*     device,
    dw3000_aes_status_t* status
);

dw3000_error_t dw3000_hal_aes_clear_status(
    dw3000_device_t*    device,
    dw3000_aes_status_t status
);

dw3000_error_t dw3000_hal_aes_clear_events(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aes_enable_events(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aes_disable_events(dw3000_device_t* device);

bool dw3000_hal_aes_status_has_error(dw3000_aes_status_t status);

dw3000_error_t dw3000_hal_aes_start(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aes_wait_done(
    dw3000_device_t*     device,
    uint32_t             timeout_us,
    dw3000_aes_status_t* final_status
);

dw3000_error_t dw3000_hal_aes_configure(
    dw3000_device_t*           device,
    const dw3000_aes_config_t* config
);

dw3000_error_t dw3000_hal_aes_configure_current(dw3000_device_t* device);

dw3000_error_t dw3000_hal_aes_run(
    dw3000_device_t*           device,
    const dw3000_aes_config_t* config,
    uint32_t                   timeout_us,
    dw3000_aes_status_t*       final_status
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_AES_H */
