#include "dw3000_hal/fcmd.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DW3000_FCMD_SPI_HEADER_PREFIX 0x81U
#define DW3000_FCMD_SPI_OPCODE_SHIFT  1U

static inline void dw3000_hal_fcmd_lock(const dw3000_device_t* device) {
    if (device->port.lock != NULL) {
        device->port.lock(device->port.ctx);
    }
}

static inline void dw3000_hal_fcmd_unlock(const dw3000_device_t* device) {
    if (device->port.unlock != NULL) {
        device->port.unlock(device->port.ctx);
    }
}

static bool dw3000_hal_fcmd_is_tx_command(dw3000_fcmd_t command) {
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

static bool dw3000_hal_fcmd_is_rx_command(dw3000_fcmd_t command) {
    return (command == DW3000_FCMD_RX) ||
           (command == DW3000_FCMD_DRX) ||
           (command == DW3000_FCMD_DRX_TS) ||
           (command == DW3000_FCMD_DRX_RS) ||
           (command == DW3000_FCMD_DRX_REF);
}

static void dw3000_hal_fcmd_update_state(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
) {
    if (command == DW3000_FCMD_TXRXOFF) {
        device->state_flags = (dw3000_device_state_flags_t)((device->state_flags | DW3000_DEVICE_STATE_IDLE_PLL) &
                                                            ~(DW3000_DEVICE_STATE_IDLE_RC |
                                                              DW3000_DEVICE_STATE_RX_ON |
                                                              DW3000_DEVICE_STATE_TX_PENDING |
                                                              DW3000_DEVICE_STATE_SLEEPING));
        return;
    }

    if (dw3000_hal_fcmd_is_tx_command(command)) {
        device->state_flags = (dw3000_device_state_flags_t)((device->state_flags | DW3000_DEVICE_STATE_TX_PENDING) &
                                                            ~(DW3000_DEVICE_STATE_IDLE_RC |
                                                              DW3000_DEVICE_STATE_IDLE_PLL |
                                                              DW3000_DEVICE_STATE_RX_ON |
                                                              DW3000_DEVICE_STATE_SLEEPING));
        return;
    }

    if (dw3000_hal_fcmd_is_rx_command(command)) {
        device->state_flags = (dw3000_device_state_flags_t)((device->state_flags | DW3000_DEVICE_STATE_RX_ON) &
                                                            ~(DW3000_DEVICE_STATE_IDLE_RC |
                                                              DW3000_DEVICE_STATE_IDLE_PLL |
                                                              DW3000_DEVICE_STATE_TX_PENDING |
                                                              DW3000_DEVICE_STATE_SLEEPING));
        return;
    }

    if ((command == DW3000_FCMD_DB_TOGGLE) && device->config.use_double_buffer) {
        device->active_rx_buffer ^= 1U;
    }
}

bool dw3000_hal_fcmd_is_valid(dw3000_fcmd_t command) {
    return (uint32_t)command <= (uint32_t)DW3000_FCMD_DB_TOGGLE;
}

uint8_t dw3000_hal_fcmd_header(dw3000_fcmd_t command) {
    return (uint8_t)(DW3000_FCMD_SPI_HEADER_PREFIX |
                     ((uint8_t)command << DW3000_FCMD_SPI_OPCODE_SHIFT));
}

dw3000_error_t dw3000_hal_fcmd_issue(
    dw3000_device_t* device,
    dw3000_fcmd_t    command
) {
    dw3000_error_t err;
    uint8_t        header;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_fcmd_is_valid(command)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->port.spi_write == NULL) {
        return DW3000_ERROR_INVALID_STATE;
    }

    header = dw3000_hal_fcmd_header(command);

    dw3000_hal_fcmd_lock(device);
    err = device->port.spi_write(device->port.ctx, &header, sizeof(header), NULL, 0U);
    dw3000_hal_fcmd_unlock(device);

    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_fcmd_update_state(device, command);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_fcmd_txrxoff(dw3000_device_t* device) {
    return dw3000_hal_fcmd_issue(device, DW3000_FCMD_TXRXOFF);
}

dw3000_error_t dw3000_hal_fcmd_clear_irqs(dw3000_device_t* device) {
    return dw3000_hal_fcmd_issue(device, DW3000_FCMD_CLR_IRQS);
}

dw3000_error_t dw3000_hal_fcmd_db_toggle(dw3000_device_t* device) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!device->config.use_double_buffer) {
        return DW3000_ERROR_INVALID_STATE;
    }

    return dw3000_hal_fcmd_issue(device, DW3000_FCMD_DB_TOGGLE);
}
