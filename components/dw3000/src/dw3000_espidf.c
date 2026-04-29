#include "dw3000_espidf.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"  // IWYU pragma: keep
#include "freertos/semphr.h"

#define DW3000_ESPIDF_CTX_MAGIC          0x44334944UL
#define DW3000_ESPIDF_SPI_MAX_HEADER_LEN 2U

typedef struct {
    uint32_t            magic;
    spi_device_handle_t spi;
    gpio_num_t          reset_gpio;
    gpio_num_t          irq_gpio;
    SemaphoreHandle_t   lock;
} dw3000_espidf_ctx_t;

static dw3000_error_t dw3000_espidf_from_esp_err(esp_err_t err) {
    switch (err) {
        case ESP_OK:
            return DW3000_ERROR_OK;

        case ESP_ERR_INVALID_ARG:
            return DW3000_ERROR_INVALID_ARG;

        case ESP_ERR_INVALID_SIZE:
            return DW3000_ERROR_INVALID_SIZE;

        case ESP_ERR_INVALID_STATE:
            return DW3000_ERROR_INVALID_STATE;

        case ESP_ERR_NOT_SUPPORTED:
            return DW3000_ERROR_NOT_SUPPORTED;

        case ESP_ERR_TIMEOUT:
            return DW3000_ERROR_TIMEOUT;

        case ESP_ERR_NO_MEM:
            return DW3000_ERROR_NO_MEMORY;

        default:
            return DW3000_ERROR_IO;
    }
}

static dw3000_espidf_ctx_t* dw3000_espidf_ctx(void* ctx) {
    dw3000_espidf_ctx_t* espidf_ctx = (dw3000_espidf_ctx_t*)ctx;

    if ((espidf_ctx == NULL) ||
        (espidf_ctx->magic != DW3000_ESPIDF_CTX_MAGIC)) {
        return NULL;
    }

    return espidf_ctx;
}

static bool dw3000_espidf_gpio_is_valid(gpio_num_t gpio) {
    return (gpio == GPIO_NUM_NC) ||
           ((gpio >= 0) && (gpio < GPIO_NUM_MAX) && (gpio < 64));
}

static uint64_t dw3000_espidf_gpio_mask(gpio_num_t gpio) {
    return 1ULL << (uint32_t)gpio;
}

static dw3000_error_t dw3000_espidf_configure_reset_gpio(
    gpio_num_t gpio
) {
    gpio_config_t config = {
        .pin_bit_mask = dw3000_espidf_gpio_mask(gpio),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_set_level(gpio, 1);
    if (err != ESP_OK) {
        return dw3000_espidf_from_esp_err(err);
    }

    return dw3000_espidf_from_esp_err(gpio_config(&config));
}

static dw3000_error_t dw3000_espidf_configure_irq_gpio(
    gpio_num_t gpio
) {
    gpio_config_t config = {
        .pin_bit_mask = dw3000_espidf_gpio_mask(gpio),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };

    return dw3000_espidf_from_esp_err(gpio_config(&config));
}

static dw3000_error_t dw3000_espidf_validate_spi_access(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    const void*    data,
    size_t         data_len
) {
    dw3000_espidf_ctx_t* espidf_ctx = dw3000_espidf_ctx(ctx);

    if ((espidf_ctx == NULL) || (espidf_ctx->spi == NULL)) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (header == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((header_len == 0U) ||
        (header_len > DW3000_ESPIDF_SPI_MAX_HEADER_LEN)) {
        return DW3000_ERROR_INVALID_SIZE;
    }

    if ((data == NULL) && (data_len != 0U)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

static uint16_t dw3000_espidf_spi_header_cmd(
    const uint8_t* header,
    size_t         header_len
) {
    uint16_t cmd = 0U;

    for (size_t i = 0U; i < header_len; ++i) {
        cmd = (uint16_t)((cmd << 8U) | header[i]);
    }

    return cmd;
}

void dw3000_espidf_delay_us(void* ctx, uint32_t delay_us) {
    (void)ctx;

    if (delay_us != 0U) {
        esp_rom_delay_us(delay_us);
    }
}

uint64_t dw3000_espidf_get_time_us(void* ctx) {
    (void)ctx;

    return (uint64_t)esp_timer_get_time();
}

void dw3000_espidf_port_default_config(
    dw3000_espidf_port_config_t* config
) {
    if (config == NULL) {
        return;
    }

    memset(config, 0, sizeof(*config));
    config->reset_gpio = GPIO_NUM_NC;
    config->irq_gpio   = GPIO_NUM_NC;
}

dw3000_port_t* dw3000_espidf_port_new(void) {
    dw3000_port_t*       port;
    dw3000_espidf_ctx_t* ctx;

    port = (dw3000_port_t*)calloc(1U, sizeof(*port));

    if (port == NULL) {
        return NULL;
    }

    ctx = (dw3000_espidf_ctx_t*)calloc(1U, sizeof(*ctx));
    if (ctx == NULL) {
        free(port);
        return NULL;
    }

    ctx->magic      = DW3000_ESPIDF_CTX_MAGIC;
    ctx->reset_gpio = GPIO_NUM_NC;
    ctx->irq_gpio   = GPIO_NUM_NC;
    ctx->lock       = xSemaphoreCreateRecursiveMutex();
    if (ctx->lock == NULL) {
        free(ctx);
        free(port);
        return NULL;
    }

    port->ctx         = ctx;
    port->spi_write   = dw3000_espidf_spi_write;
    port->spi_read    = dw3000_espidf_spi_read;
    port->reset       = dw3000_espidf_reset;
    port->irq_read    = dw3000_espidf_irq_read;
    port->delay_us    = dw3000_espidf_delay_us;
    port->get_time_us = dw3000_espidf_get_time_us;
    port->lock        = dw3000_espidf_lock;
    port->unlock      = dw3000_espidf_unlock;

    return port;
}

void dw3000_espidf_port_delete(dw3000_port_t* port) {
    dw3000_espidf_ctx_t* ctx;

    if (port == NULL) {
        return;
    }

    ctx = dw3000_espidf_ctx(port->ctx);
    if (ctx != NULL) {
        if (ctx->lock != NULL) {
            vSemaphoreDelete(ctx->lock);
        }

        ctx->magic = 0U;
        free(ctx);
    }

    free(port);
}

dw3000_error_t dw3000_espidf_port_configure(
    dw3000_port_t*                     port,
    const dw3000_espidf_port_config_t* config
) {
    dw3000_espidf_ctx_t*               ctx;
    dw3000_espidf_port_config_t        default_config;
    const dw3000_espidf_port_config_t* actual_config = config;
    dw3000_error_t                     err;

    if (port == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    ctx = dw3000_espidf_ctx(port->ctx);
    if (ctx == NULL) {
        return DW3000_ERROR_INVALID_STATE;
    }

    if (actual_config == NULL) {
        dw3000_espidf_port_default_config(&default_config);
        actual_config = &default_config;
    }

    if (!dw3000_espidf_gpio_is_valid(actual_config->reset_gpio) ||
        !dw3000_espidf_gpio_is_valid(actual_config->irq_gpio)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if ((actual_config->reset_gpio != GPIO_NUM_NC) &&
        actual_config->configure_reset_gpio) {
        err = dw3000_espidf_configure_reset_gpio(actual_config->reset_gpio);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    if ((actual_config->irq_gpio != GPIO_NUM_NC) &&
        actual_config->configure_irq_gpio) {
        err = dw3000_espidf_configure_irq_gpio(actual_config->irq_gpio);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    ctx->spi        = actual_config->spi;
    ctx->reset_gpio = actual_config->reset_gpio;
    ctx->irq_gpio   = actual_config->irq_gpio;

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_espidf_spi_write(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    const void*    data,
    size_t         data_len
) {
    dw3000_error_t        err;
    dw3000_espidf_ctx_t*  espidf_ctx;
    spi_transaction_ext_t transaction;

    err = dw3000_espidf_validate_spi_access(
        ctx,
        header,
        header_len,
        data,
        data_len
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    espidf_ctx = dw3000_espidf_ctx(ctx);

    memset(&transaction, 0, sizeof(transaction));
    transaction.base.flags     = SPI_TRANS_VARIABLE_CMD;
    transaction.base.cmd       = dw3000_espidf_spi_header_cmd(header, header_len);
    transaction.base.length    = data_len * 8U;
    transaction.base.tx_buffer = data;
    transaction.command_bits   = header_len * 8U;

    return dw3000_espidf_from_esp_err(
        spi_device_polling_transmit(espidf_ctx->spi, &transaction.base)
    );
}

dw3000_error_t dw3000_espidf_spi_read(
    void*          ctx,
    const uint8_t* header,
    size_t         header_len,
    void*          data,
    size_t         data_len
) {
    dw3000_error_t        err;
    dw3000_espidf_ctx_t*  espidf_ctx;
    spi_transaction_ext_t transaction;

    err = dw3000_espidf_validate_spi_access(
        ctx,
        header,
        header_len,
        data,
        data_len
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    espidf_ctx = dw3000_espidf_ctx(ctx);

    memset(&transaction, 0, sizeof(transaction));
    transaction.base.flags     = SPI_TRANS_VARIABLE_CMD;
    transaction.base.cmd       = dw3000_espidf_spi_header_cmd(header, header_len);
    transaction.base.length    = data_len * 8U;
    transaction.base.rxlength  = data_len * 8U;
    transaction.base.rx_buffer = data;
    transaction.command_bits   = header_len * 8U;

    return dw3000_espidf_from_esp_err(
        spi_device_polling_transmit(espidf_ctx->spi, &transaction.base)
    );
}

void dw3000_espidf_reset(void* ctx, bool asserted) {
    dw3000_espidf_ctx_t* espidf_ctx = dw3000_espidf_ctx(ctx);

    if ((espidf_ctx == NULL) || (espidf_ctx->reset_gpio == GPIO_NUM_NC)) {
        return;
    }

    (void)gpio_set_level(espidf_ctx->reset_gpio, asserted ? 0 : 1);
}

int dw3000_espidf_irq_read(void* ctx) {
    dw3000_espidf_ctx_t* espidf_ctx = dw3000_espidf_ctx(ctx);

    if ((espidf_ctx == NULL) || (espidf_ctx->irq_gpio == GPIO_NUM_NC)) {
        return 0;
    }

    return gpio_get_level(espidf_ctx->irq_gpio);
}

void dw3000_espidf_lock(void* ctx) {
    dw3000_espidf_ctx_t* espidf_ctx = dw3000_espidf_ctx(ctx);

    if ((espidf_ctx == NULL) || (espidf_ctx->lock == NULL)) {
        return;
    }

    (void)xSemaphoreTakeRecursive(espidf_ctx->lock, portMAX_DELAY);
}

void dw3000_espidf_unlock(void* ctx) {
    dw3000_espidf_ctx_t* espidf_ctx = dw3000_espidf_ctx(ctx);

    if ((espidf_ctx == NULL) || (espidf_ctx->lock == NULL)) {
        return;
    }

    (void)xSemaphoreGiveRecursive(espidf_ctx->lock);
}
