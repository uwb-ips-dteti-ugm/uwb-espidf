#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "driver/spi_master.h"
#include "dw3000_error.h"
#include "dw3000_espidf.h"
#include "dw3000_hal/aes.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/status.h"
#include "dw3000_register.h"
#include "esp_err.h"
#include "esp_log.h"

#define DW3000_HAL_GPIO_IRQ_KEY_SLOT         3U
#define DW3000_HAL_GPIO_IRQ_INPUT_OFFSET     0U
#define DW3000_HAL_GPIO_IRQ_OUTPUT_OFFSET    48U
#define DW3000_HAL_GPIO_IRQ_PAYLOAD_LEN      32U
#define DW3000_HAL_GPIO_IRQ_WAIT_TIMEOUT_US  5000U
#define DW3000_HAL_GPIO_IRQ_POLL_DELAY_US    100U

static const char* TAG = "dw3000_hal_gpio_irq";

static const uint8_t DW3000_HAL_GPIO_IRQ_PAYLOAD[DW3000_HAL_GPIO_IRQ_PAYLOAD_LEN] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
    0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F,
};

static const uint8_t DW3000_HAL_GPIO_IRQ_IV96[12] = {
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5,
    0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB,
};

static const dw3000_aes_key_t DW3000_HAL_GPIO_IRQ_KEY = {
    .bytes = {
        0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE,
        0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
    },
};

typedef struct {
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
} dw3000_hal_gpio_irq_app_t;

static bool dw3000_hal_gpio_irq_check_esp(
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

static bool dw3000_hal_gpio_irq_check_dw3000(
    dw3000_error_t err,
    const char*    expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

static bool dw3000_hal_gpio_irq_check_bytes_differ(
    const char*    name,
    const uint8_t* actual,
    const uint8_t* other,
    size_t         len
) {
    for (size_t i = 0U; i < len; ++i) {
        if (actual[i] != other[i]) {
            return true;
        }
    }

    ESP_LOGE(TAG, "%s did not change", name);
    return false;
}

#define DW3000_HAL_GPIO_IRQ_CHECK_ESP(expr_) \
    dw3000_hal_gpio_irq_check_esp((expr_), #expr_)

#define DW3000_HAL_GPIO_IRQ_CHECK_DW3000(expr_) \
    dw3000_hal_gpio_irq_check_dw3000((expr_), #expr_)

static bool dw3000_hal_gpio_irq_setup_spi(
    dw3000_hal_gpio_irq_app_t* app
) {
    spi_bus_config_t bus_config = {
        .mosi_io_num     = DW3000_HAL_GPIO_IRQ_PIN_MOSI,
        .miso_io_num     = DW3000_HAL_GPIO_IRQ_PIN_MISO,
        .sclk_io_num     = DW3000_HAL_GPIO_IRQ_PIN_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = DW3000_HAL_GPIO_IRQ_SPI_MAX_TRANSFER_SIZE,
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = DW3000_HAL_GPIO_IRQ_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = DW3000_HAL_GPIO_IRQ_PIN_CS,
        .queue_size     = 1,
    };
    esp_err_t err;

    err = spi_bus_initialize(
        DW3000_HAL_GPIO_IRQ_SPI_HOST,
        &bus_config,
        SPI_DMA_CH_AUTO
    );
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return DW3000_HAL_GPIO_IRQ_CHECK_ESP(err);
    }

    return DW3000_HAL_GPIO_IRQ_CHECK_ESP(spi_bus_add_device(
        DW3000_HAL_GPIO_IRQ_SPI_HOST,
        &device_config,
        &app->spi
    ));
}

static bool dw3000_hal_gpio_irq_setup_port(
    dw3000_hal_gpio_irq_app_t* app
) {
    dw3000_espidf_port_config_t port_config;

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(TAG, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = DW3000_HAL_GPIO_IRQ_PIN_RST;
    port_config.irq_gpio             = DW3000_HAL_GPIO_IRQ_PIN_IRQ;
    port_config.configure_reset_gpio = DW3000_HAL_GPIO_IRQ_CONFIGURE_RST;
    port_config.configure_irq_gpio   = DW3000_HAL_GPIO_IRQ_CONFIGURE_IRQ;

    return DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_espidf_port_configure(
        app->port,
        &port_config
    ));
}

static bool dw3000_hal_gpio_irq_setup(dw3000_hal_gpio_irq_app_t* app) {
    memset(app, 0, sizeof(*app));

    if (!dw3000_hal_gpio_irq_setup_spi(app)) {
        return false;
    }

    if (!dw3000_hal_gpio_irq_setup_port(app)) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        DW3000_HAL_GPIO_IRQ_SPI_HOST,
        DW3000_HAL_GPIO_IRQ_SPI_CLOCK_HZ,
        DW3000_HAL_GPIO_IRQ_PIN_SCK,
        DW3000_HAL_GPIO_IRQ_PIN_MISO,
        DW3000_HAL_GPIO_IRQ_PIN_MOSI,
        DW3000_HAL_GPIO_IRQ_PIN_CS,
        DW3000_HAL_GPIO_IRQ_PIN_RST,
        DW3000_HAL_GPIO_IRQ_PIN_IRQ
    );

    return true;
}

static bool dw3000_hal_gpio_irq_teardown(dw3000_hal_gpio_irq_app_t* app) {
    bool ok = true;

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok       = DW3000_HAL_GPIO_IRQ_CHECK_ESP(spi_bus_remove_device(app->spi)) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok = DW3000_HAL_GPIO_IRQ_CHECK_ESP(spi_bus_free(
            DW3000_HAL_GPIO_IRQ_SPI_HOST
        )) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

static bool dw3000_hal_gpio_irq_initialize(
    dw3000_hal_gpio_irq_app_t* app
) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);
    config.load_otp_calibration = DW3000_HAL_GPIO_IRQ_LOAD_OTP_CALIBRATION;
    config.auto_init_pll        = DW3000_HAL_GPIO_IRQ_ENTER_IDLE_PLL;

    return DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_initialize(
        &app->device,
        NULL,
        NULL,
        app->port,
        &config
    ));
}

static int dw3000_hal_gpio_irq_read_irq(dw3000_device_t* device) {
    if ((device == NULL) || (device->port.irq_read == NULL)) {
        return -1;
    }

    return device->port.irq_read(device->port.ctx);
}

static bool dw3000_hal_gpio_irq_wait_level(
    dw3000_device_t* device,
    int              expected_level,
    uint32_t         timeout_us
) {
    uint64_t start_us;
    int      last_level = -1;

    if ((device == NULL) || (device->port.get_time_us == NULL)) {
        ESP_LOGE(TAG, "IRQ wait requires irq_read and get_time_us port callbacks");
        return false;
    }

    start_us = device->port.get_time_us(device->port.ctx);
    do {
        last_level = dw3000_hal_gpio_irq_read_irq(device);
        if (last_level < 0) {
            ESP_LOGE(TAG, "IRQ read callback is not configured");
            return false;
        }

        if ((last_level != 0) == (expected_level != 0)) {
            return true;
        }

        if (device->port.delay_us != NULL) {
            device->port.delay_us(device->port.ctx, DW3000_HAL_GPIO_IRQ_POLL_DELAY_US);
        }
    } while ((device->port.get_time_us(device->port.ctx) - start_us) < timeout_us);

    ESP_LOGE(
        TAG,
        "IRQ did not become %d within %" PRIu32 " us; last=%d",
        expected_level,
        timeout_us,
        last_level
    );
    return false;
}

static void dw3000_hal_gpio_irq_make_aes_config(
    dw3000_aes_config_t* config
) {
    memset(config, 0, sizeof(*config));

    config->mode     = DW3000_AES_MODE_ENCRYPT;
    config->key_size = DW3000_AES_KEY_SIZE_128;
    config->tag_size = DW3000_AES_TAG_SIZE_NONE;
    config->core     = DW3000_AES_CORE_GCM;
    config->key_src  = DW3000_AES_KEY_SRC_RAM;
    config->flags    = 0U;
    config->key_addr = DW3000_HAL_GPIO_IRQ_KEY_SLOT;
    memcpy(config->iv.bytes, DW3000_HAL_GPIO_IRQ_IV96, sizeof(DW3000_HAL_GPIO_IRQ_IV96));

    config->dma.src_port   = DW3000_AES_PORT_SCRATCH;
    config->dma.src_addr   = DW3000_HAL_GPIO_IRQ_INPUT_OFFSET;
    config->dma.dst_port   = DW3000_AES_PORT_SCRATCH;
    config->dma.dst_addr   = DW3000_HAL_GPIO_IRQ_OUTPUT_OFFSET;
    config->dma.endianness = DW3000_AES_ENDIAN_BIG;
    config->dma.hdr_size   = 0U;
    config->dma.pyld_size  = DW3000_HAL_GPIO_IRQ_PAYLOAD_LEN;
}

static bool dw3000_hal_gpio_irq_check_aes_status(
    dw3000_aes_status_t status
) {
    if (((uint32_t)status & (uint32_t)DW3000_AES_STS_AES_DONE) == 0U) {
        ESP_LOGE(TAG, "AES_DONE was not set: status=0x%02" PRIX32, (uint32_t)status);
        return false;
    }

    if (dw3000_hal_aes_status_has_error(status)) {
        ESP_LOGE(TAG, "AES error status=0x%02" PRIX32, (uint32_t)status);
        return false;
    }

    return true;
}

static bool dw3000_hal_gpio_irq_run_aes_event(dw3000_device_t* device) {
    uint8_t             ciphertext[DW3000_HAL_GPIO_IRQ_PAYLOAD_LEN];
    uint8_t             fill[DW3000_HAL_GPIO_IRQ_PAYLOAD_LEN];
    dw3000_aes_config_t config;
    dw3000_aes_status_t status;

    memset(fill, 0xA5, sizeof(fill));
    dw3000_hal_gpio_irq_make_aes_config(&config);

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_write_key_ram_slot(
            device,
            DW3000_HAL_GPIO_IRQ_KEY_SLOT,
            &DW3000_HAL_GPIO_IRQ_KEY
        )) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_write_scratch(
            device,
            DW3000_HAL_GPIO_IRQ_INPUT_OFFSET,
            DW3000_HAL_GPIO_IRQ_PAYLOAD,
            sizeof(DW3000_HAL_GPIO_IRQ_PAYLOAD)
        )) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_write_scratch(
            device,
            DW3000_HAL_GPIO_IRQ_OUTPUT_OFFSET,
            fill,
            sizeof(fill)
        )) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_run(
            device,
            &config,
            DW3000_HAL_AES_DONE_TIMEOUT_US,
            &status
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "AES status=0x%02" PRIX32, (uint32_t)status);
    if (!dw3000_hal_gpio_irq_check_aes_status(status)) {
        return false;
    }

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_read_scratch(
            device,
            DW3000_HAL_GPIO_IRQ_OUTPUT_OFFSET,
            ciphertext,
            sizeof(ciphertext)
        ))) {
        return false;
    }

    return dw3000_hal_gpio_irq_check_bytes_differ(
        "ciphertext",
        ciphertext,
        DW3000_HAL_GPIO_IRQ_PAYLOAD,
        sizeof(ciphertext)
    );
}

static bool dw3000_hal_gpio_irq_log_irq_state(
    dw3000_device_t* device,
    const char*      prefix,
    dw3000_txrx_event_t* events_out
) {
    dw3000_txrx_event_t events;
    uint8_t             fint;
    int                 irq_level;

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_read(
            device,
            &events
        )) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_reg_read_u8(
            device,
            DW3000_REG_FINT_STAT,
            &fint
        ))) {
        return false;
    }

    irq_level = dw3000_hal_gpio_irq_read_irq(device);
    if (irq_level < 0) {
        ESP_LOGE(TAG, "IRQ read callback is not configured");
        return false;
    }

    ESP_LOGI(
        TAG,
        "%s SYS_STATUS=0x%012" PRIX64 " FINT_STAT=0x%02" PRIX8 " irq=%d",
        prefix,
        (uint64_t)events,
        fint,
        irq_level
    );

    if (events_out != NULL) {
        *events_out = events;
    }

    return true;
}

static bool dw3000_hal_gpio_irq_prepare_events(
    dw3000_device_t*     device,
    dw3000_txrx_event_t  saved_enabled
) {
    dw3000_txrx_event_t enabled;

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_set_enabled(
            device,
            0U
        )) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_clear_all(device)) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_clear_events(device))) {
        return false;
    }

    if (!dw3000_hal_gpio_irq_wait_level(
            device,
            0,
            DW3000_HAL_GPIO_IRQ_WAIT_TIMEOUT_US
        )) {
        return false;
    }

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_set_enabled(
            device,
            DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR
        )) ||
        !DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_read_enabled(
            device,
            &enabled
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "saved SYS_ENABLE=0x%012" PRIX64, (uint64_t)saved_enabled);
    ESP_LOGI(TAG, "test SYS_ENABLE=0x%012" PRIX64, (uint64_t)enabled);

    if ((enabled & (DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR)) !=
        (DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR)) {
        ESP_LOGE(TAG, "AES interrupt events were not enabled");
        return false;
    }

    return dw3000_hal_gpio_irq_log_irq_state(device, "baseline", NULL);
}

static bool dw3000_hal_gpio_irq_restore_events(
    dw3000_device_t*    device,
    dw3000_txrx_event_t saved_enabled
) {
    return DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_set_enabled(
        device,
        saved_enabled
    ));
}

static bool dw3000_hal_gpio_irq_run_test(dw3000_device_t* device) {
    dw3000_txrx_event_t saved_enabled;
    dw3000_txrx_event_t events;
    bool                ok;

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_status_read_enabled(
            device,
            &saved_enabled
        ))) {
        return false;
    }

    if (!dw3000_hal_gpio_irq_prepare_events(device, saved_enabled)) {
        (void)dw3000_hal_gpio_irq_restore_events(device, saved_enabled);
        return false;
    }

    ok = dw3000_hal_gpio_irq_run_aes_event(device);

    if (ok) {
        ok = dw3000_hal_gpio_irq_wait_level(
            device,
            1,
            DW3000_HAL_GPIO_IRQ_WAIT_TIMEOUT_US
        ) && ok;
    }

    if (ok) {
        ok = dw3000_hal_gpio_irq_log_irq_state(device, "asserted", &events) && ok;
    }

    if (ok &&
        !dw3000_hal_status_has_event(events, DW3000_TXRX_EVENT_AES_DONE)) {
        ESP_LOGE(TAG, "SYS_STATUS.AES_DONE was not set");
        ok = false;
    }

    if (ok &&
        !dw3000_hal_status_has_event(events, DW3000_TXRX_EVENT_IRQS)) {
        ESP_LOGE(TAG, "SYS_STATUS.IRQS was not set");
        ok = false;
    }

    if (!DW3000_HAL_GPIO_IRQ_CHECK_DW3000(dw3000_hal_aes_clear_events(device))) {
        ok = false;
    }

    if (!dw3000_hal_gpio_irq_wait_level(
            device,
            0,
            DW3000_HAL_GPIO_IRQ_WAIT_TIMEOUT_US
        )) {
        ok = false;
    }

    if (!dw3000_hal_gpio_irq_log_irq_state(device, "cleared", &events)) {
        ok = false;
    }

    if ((events & (DW3000_TXRX_EVENT_AES_DONE | DW3000_TXRX_EVENT_AES_ERR |
                   DW3000_TXRX_EVENT_IRQS)) != 0U) {
        ESP_LOGE(TAG, "AES/IRQ events remained set after clear");
        ok = false;
    }

    return dw3000_hal_gpio_irq_restore_events(device, saved_enabled) && ok;
}

void app_main(void) {
    dw3000_hal_gpio_irq_app_t app;
    bool                      ok;

    ok = dw3000_hal_gpio_irq_setup(&app) &&
         dw3000_hal_gpio_irq_initialize(&app) &&
         dw3000_hal_gpio_irq_run_test(&app.device);

    ok = dw3000_hal_gpio_irq_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_GPIO_IRQ_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_GPIO_IRQ_FAIL");
    }
}
