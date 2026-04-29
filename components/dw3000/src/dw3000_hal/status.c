#include "dw3000_hal/status.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_register.h"

#define DW3000_HAL_STATUS_LEN 6U

#define DW3000_HAL_STATUS_CLEARABLE_MASK \
    (DW3000_TXRX_EVENT_CPLOCK |          \
     DW3000_TXRX_EVENT_SPICRCE |         \
     DW3000_TXRX_EVENT_AAT |             \
     DW3000_TXRX_EVENT_TXFRB |           \
     DW3000_TXRX_EVENT_TXPRS |           \
     DW3000_TXRX_EVENT_TXPHS |           \
     DW3000_TXRX_EVENT_TXFRS |           \
     DW3000_TXRX_EVENT_RXPRD |           \
     DW3000_TXRX_EVENT_RXSFDD |          \
     DW3000_TXRX_EVENT_CIADONE |         \
     DW3000_TXRX_EVENT_RXPHD |           \
     DW3000_TXRX_EVENT_RXPHE |           \
     DW3000_TXRX_EVENT_RXFR |            \
     DW3000_TXRX_EVENT_RXFCG |           \
     DW3000_TXRX_EVENT_RXFCE |           \
     DW3000_TXRX_EVENT_RXFSL |           \
     DW3000_TXRX_EVENT_RXFTO |           \
     DW3000_TXRX_EVENT_CIAERR |          \
     DW3000_TXRX_EVENT_VWARN |           \
     DW3000_TXRX_EVENT_RXOVRR |          \
     DW3000_TXRX_EVENT_RXPTO |           \
     DW3000_TXRX_EVENT_SPIRDY |          \
     DW3000_TXRX_EVENT_RCINIT |          \
     DW3000_TXRX_EVENT_PLL_HILO |        \
     DW3000_TXRX_EVENT_RXSTO |           \
     DW3000_TXRX_EVENT_HPDWARN |         \
     DW3000_TXRX_EVENT_CPERR |           \
     DW3000_TXRX_EVENT_ARFE |            \
     DW3000_TXRX_EVENT_RXPREJ |          \
     DW3000_TXRX_EVENT_VT_DET |          \
     DW3000_TXRX_EVENT_GPIOIRQ |         \
     DW3000_TXRX_EVENT_AES_DONE |        \
     DW3000_TXRX_EVENT_AES_ERR |         \
     DW3000_TXRX_EVENT_CMD_ERR |         \
     DW3000_TXRX_EVENT_SPI_OVF |         \
     DW3000_TXRX_EVENT_SPI_UNF |         \
     DW3000_TXRX_EVENT_SPIERR |          \
     DW3000_TXRX_EVENT_CCA_FAIL)

#define DW3000_HAL_STATUS_ENABLE_MASK DW3000_HAL_STATUS_CLEARABLE_MASK

static dw3000_txrx_event_t dw3000_hal_status_unpack(const uint8_t raw[DW3000_HAL_STATUS_LEN]) {
    dw3000_txrx_event_t value = 0U;

    for (uint8_t i = 0U; i < DW3000_HAL_STATUS_LEN; ++i) {
        value |= (dw3000_txrx_event_t)raw[i] << (8U * i);
    }

    return value;
}

static void dw3000_hal_status_pack(
    dw3000_txrx_event_t value,
    uint8_t             raw[DW3000_HAL_STATUS_LEN]
) {
    for (uint8_t i = 0U; i < DW3000_HAL_STATUS_LEN; ++i) {
        raw[i] = (uint8_t)((value >> (8U * i)) & 0xFFU);
    }
}

static dw3000_error_t dw3000_hal_status_read_enabled_uncached(
    dw3000_device_t*     device,
    dw3000_txrx_event_t* events
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_HAL_STATUS_LEN];

    err = dw3000_reg_read(device, DW3000_REG_SYS_ENABLE, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *events                      = dw3000_hal_status_unpack(raw) & DW3000_HAL_STATUS_ENABLE_MASK;
    device->enabled_events       = *events;
    device->enabled_events_valid = true;

    return DW3000_ERROR_OK;
}

dw3000_txrx_event_t dw3000_hal_status_clearable_events(void) {
    return DW3000_HAL_STATUS_CLEARABLE_MASK;
}

dw3000_txrx_event_t dw3000_hal_status_enable_events(void) {
    return DW3000_HAL_STATUS_ENABLE_MASK;
}

bool dw3000_hal_status_has_event(
    dw3000_txrx_event_t status,
    dw3000_txrx_event_t event
) {
    return (status & event) == event;
}

dw3000_error_t dw3000_hal_status_read(
    dw3000_device_t*     device,
    dw3000_txrx_event_t* status
) {
    dw3000_error_t err;
    uint8_t        raw[DW3000_HAL_STATUS_LEN];

    if ((device == NULL) || (status == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read(device, DW3000_REG_SYS_STATUS, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *status = dw3000_hal_status_unpack(raw);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_status_clear(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
) {
    uint8_t raw[DW3000_HAL_STATUS_LEN];

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    events &= DW3000_HAL_STATUS_CLEARABLE_MASK;
    if (events == 0U) {
        return DW3000_ERROR_OK;
    }

    dw3000_hal_status_pack(events, raw);
    return dw3000_reg_write(device, DW3000_REG_SYS_STATUS, raw, sizeof(raw));
}

dw3000_error_t dw3000_hal_status_clear_all(dw3000_device_t* device) {
    return dw3000_hal_status_clear(device, DW3000_HAL_STATUS_CLEARABLE_MASK);
}

dw3000_error_t dw3000_hal_status_read_enabled(
    dw3000_device_t*     device,
    dw3000_txrx_event_t* events
) {
    if ((device == NULL) || (events == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_status_read_enabled_uncached(device, events);
}

dw3000_error_t dw3000_hal_status_set_enabled(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
) {
    uint8_t raw[DW3000_HAL_STATUS_LEN];

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    events &= DW3000_HAL_STATUS_ENABLE_MASK;
    dw3000_hal_status_pack(events, raw);

    dw3000_error_t err = dw3000_reg_write(device, DW3000_REG_SYS_ENABLE, raw, sizeof(raw));
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->enabled_events       = events;
    device->enabled_events_valid = true;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_status_enable(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
) {
    dw3000_error_t      err;
    dw3000_txrx_event_t current;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->enabled_events_valid) {
        current = device->enabled_events;
    } else {
        err = dw3000_hal_status_read_enabled_uncached(device, &current);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return dw3000_hal_status_set_enabled(device, current | events);
}

dw3000_error_t dw3000_hal_status_disable(
    dw3000_device_t*    device,
    dw3000_txrx_event_t events
) {
    dw3000_error_t      err;
    dw3000_txrx_event_t current;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (device->enabled_events_valid) {
        current = device->enabled_events;
    } else {
        err = dw3000_hal_status_read_enabled_uncached(device, &current);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    return dw3000_hal_status_set_enabled(device, current & ~events);
}
