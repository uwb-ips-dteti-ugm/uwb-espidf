#ifndef DW3000_ESPIDF_H
#define DW3000_ESPIDF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/spi_master.h"
#include "dw3000_error.h"
#include "dw3000_port.h"
#include "soc/gpio_num.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    spi_device_handle_t spi;
    gpio_num_t          reset_gpio; /* active low RSTn; GPIO_NUM_NC disables */
    gpio_num_t          irq_gpio;   /* active high IRQ; GPIO_NUM_NC disables */
    bool                configure_reset_gpio;
    bool                configure_irq_gpio;
} dw3000_espidf_port_config_t;

void dw3000_espidf_port_default_config(
    dw3000_espidf_port_config_t* config
);

/* Allocate a dw3000_port_t with the full ESP-IDF callback set installed.
   The returned port owns its internal ctx and mutex; callers should not
   overwrite port->ctx. Use dw3000_espidf_port_configure() to attach the
   ESP-IDF SPI device handle and optional reset/IRQ pins. */
dw3000_port_t* dw3000_espidf_port_new(void);

void dw3000_espidf_port_delete(dw3000_port_t* port);

dw3000_error_t dw3000_espidf_port_configure(
    dw3000_port_t*                     port,
    const dw3000_espidf_port_config_t* config
);

/* Callback implementations for dw3000_port_t. ctx must be the internal
   context created by dw3000_espidf_port_new(). */
dw3000_error_t dw3000_espidf_spi_write(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    const void*    data,
    size_t         data_len
);

dw3000_error_t dw3000_espidf_spi_read(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    void*          data,
    size_t         data_len
);

void dw3000_espidf_reset(void* ctx, bool asserted);

int dw3000_espidf_irq_read(void* ctx);

void dw3000_espidf_delay_us(void* ctx, uint32_t delay_us);

uint64_t dw3000_espidf_get_time_us(void* ctx);

void dw3000_espidf_lock(void* ctx);

void dw3000_espidf_unlock(void* ctx);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_ESPIDF_H */
