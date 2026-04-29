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

#define DW3000_HAL_AES_ENGINE_KEY_SLOT         2U
#define DW3000_HAL_AES_ENGINE_PLAINTEXT_OFFSET 0U
#define DW3000_HAL_AES_ENGINE_CIPHERTEXT_OFFSET 48U
#define DW3000_HAL_AES_ENGINE_RECOVERED_OFFSET 88U
#define DW3000_HAL_AES_ENGINE_PAYLOAD_LEN      32U

static const char* TAG = "dw3000_hal_aes_engine";

static const uint8_t DW3000_HAL_AES_ENGINE_PAYLOAD[DW3000_HAL_AES_ENGINE_PAYLOAD_LEN] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
    0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F,
};

static const uint8_t DW3000_HAL_AES_ENGINE_IV96[12] = {
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5,
    0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
};

static const dw3000_aes_key_t DW3000_HAL_AES_ENGINE_KEY = {
    .bytes = {
        0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
        0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C,
    },
};

typedef struct {
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
} dw3000_hal_aes_engine_app_t;

static bool dw3000_hal_aes_engine_check_esp(
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

static bool dw3000_hal_aes_engine_check_dw3000(
    dw3000_error_t err,
    const char*    expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

static bool dw3000_hal_aes_engine_check_bytes_equal(
    const char*    name,
    const uint8_t* actual,
    const uint8_t* expected,
    size_t         len
) {
    for (size_t i = 0U; i < len; ++i) {
        if (actual[i] != expected[i]) {
            ESP_LOGE(
                TAG,
                "%s mismatch at %u: actual=0x%02" PRIX8 " expected=0x%02" PRIX8,
                name,
                (unsigned)i,
                actual[i],
                expected[i]
            );
            return false;
        }
    }

    return true;
}

static bool dw3000_hal_aes_engine_check_bytes_differ(
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

static void dw3000_hal_aes_engine_log_bytes(
    const char*    name,
    const uint8_t* data,
    size_t         len
) {
    char hex[DW3000_HAL_AES_ENGINE_PAYLOAD_LEN * 2U + 1U];

    if (len > DW3000_HAL_AES_ENGINE_PAYLOAD_LEN) {
        len = DW3000_HAL_AES_ENGINE_PAYLOAD_LEN;
    }

    for (size_t i = 0U; i < len; ++i) {
        (void)snprintf(&hex[i * 2U], 3U, "%02X", data[i]);
    }
    hex[len * 2U] = '\0';

    ESP_LOGI(TAG, "%s=%s", name, hex);
}

#define DW3000_HAL_AES_ENGINE_CHECK_ESP(expr_) \
    dw3000_hal_aes_engine_check_esp((expr_), #expr_)

#define DW3000_HAL_AES_ENGINE_CHECK_DW3000(expr_) \
    dw3000_hal_aes_engine_check_dw3000((expr_), #expr_)

static bool dw3000_hal_aes_engine_setup_spi(
    dw3000_hal_aes_engine_app_t* app
) {
    spi_bus_config_t bus_config = {
        .mosi_io_num     = DW3000_HAL_AES_ENGINE_PIN_MOSI,
        .miso_io_num     = DW3000_HAL_AES_ENGINE_PIN_MISO,
        .sclk_io_num     = DW3000_HAL_AES_ENGINE_PIN_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = DW3000_HAL_AES_ENGINE_SPI_MAX_TRANSFER_SIZE,
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = DW3000_HAL_AES_ENGINE_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = DW3000_HAL_AES_ENGINE_PIN_CS,
        .queue_size     = 1,
    };
    esp_err_t err;

    err = spi_bus_initialize(
        DW3000_HAL_AES_ENGINE_SPI_HOST,
        &bus_config,
        SPI_DMA_CH_AUTO
    );
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return DW3000_HAL_AES_ENGINE_CHECK_ESP(err);
    }

    return DW3000_HAL_AES_ENGINE_CHECK_ESP(spi_bus_add_device(
        DW3000_HAL_AES_ENGINE_SPI_HOST,
        &device_config,
        &app->spi
    ));
}

static bool dw3000_hal_aes_engine_setup_port(
    dw3000_hal_aes_engine_app_t* app
) {
    dw3000_espidf_port_config_t port_config;

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(TAG, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = DW3000_HAL_AES_ENGINE_PIN_RST;
    port_config.irq_gpio             = DW3000_HAL_AES_ENGINE_PIN_IRQ;
    port_config.configure_reset_gpio = DW3000_HAL_AES_ENGINE_CONFIGURE_RST;
    port_config.configure_irq_gpio   = DW3000_HAL_AES_ENGINE_CONFIGURE_IRQ;

    return DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_espidf_port_configure(
        app->port,
        &port_config
    ));
}

static bool dw3000_hal_aes_engine_setup(dw3000_hal_aes_engine_app_t* app) {
    memset(app, 0, sizeof(*app));

    if (!dw3000_hal_aes_engine_setup_spi(app)) {
        return false;
    }

    if (!dw3000_hal_aes_engine_setup_port(app)) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        DW3000_HAL_AES_ENGINE_SPI_HOST,
        DW3000_HAL_AES_ENGINE_SPI_CLOCK_HZ,
        DW3000_HAL_AES_ENGINE_PIN_SCK,
        DW3000_HAL_AES_ENGINE_PIN_MISO,
        DW3000_HAL_AES_ENGINE_PIN_MOSI,
        DW3000_HAL_AES_ENGINE_PIN_CS,
        DW3000_HAL_AES_ENGINE_PIN_RST,
        DW3000_HAL_AES_ENGINE_PIN_IRQ
    );

    return true;
}

static bool dw3000_hal_aes_engine_teardown(dw3000_hal_aes_engine_app_t* app) {
    bool ok = true;

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok       = DW3000_HAL_AES_ENGINE_CHECK_ESP(spi_bus_remove_device(app->spi)) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok = DW3000_HAL_AES_ENGINE_CHECK_ESP(spi_bus_free(
            DW3000_HAL_AES_ENGINE_SPI_HOST
        )) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

static bool dw3000_hal_aes_engine_initialize(
    dw3000_hal_aes_engine_app_t* app
) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);
    config.load_otp_calibration = DW3000_HAL_AES_ENGINE_LOAD_OTP_CALIBRATION;
    config.auto_init_pll        = DW3000_HAL_AES_ENGINE_ENTER_IDLE_PLL;

    return DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_initialize(
        &app->device,
        NULL,
        NULL,
        app->port,
        &config
    ));
}

static void dw3000_hal_aes_engine_make_config(
    dw3000_aes_mode_t     mode,
    uint16_t              src_addr,
    uint16_t              dst_addr,
    dw3000_aes_config_t*  config
) {
    memset(config, 0, sizeof(*config));

    config->mode     = mode;
    config->key_size = DW3000_AES_KEY_SIZE_128;
    config->tag_size = DW3000_AES_TAG_SIZE_NONE;
    config->core     = DW3000_AES_CORE_GCM;
    config->key_src  = DW3000_AES_KEY_SRC_RAM;
    config->flags    = 0U;
    config->key_addr = DW3000_HAL_AES_ENGINE_KEY_SLOT;
    memcpy(config->iv.bytes, DW3000_HAL_AES_ENGINE_IV96, sizeof(DW3000_HAL_AES_ENGINE_IV96));

    config->dma.src_port   = DW3000_AES_PORT_SCRATCH;
    config->dma.src_addr   = src_addr;
    config->dma.dst_port   = DW3000_AES_PORT_SCRATCH;
    config->dma.dst_addr   = dst_addr;
    config->dma.endianness = DW3000_AES_ENDIAN_BIG;
    config->dma.hdr_size   = 0U;
    config->dma.pyld_size  = DW3000_HAL_AES_ENGINE_PAYLOAD_LEN;
}

static bool dw3000_hal_aes_engine_check_status(
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

static bool dw3000_hal_aes_engine_run_transform(
    dw3000_device_t*    device,
    const char*         name,
    dw3000_aes_mode_t   mode,
    uint16_t            src_addr,
    uint16_t            dst_addr
) {
    dw3000_aes_config_t config;
    dw3000_aes_status_t status;

    dw3000_hal_aes_engine_make_config(mode, src_addr, dst_addr, &config);

    if (!DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_run(
            device,
            &config,
            DW3000_HAL_AES_DONE_TIMEOUT_US,
            &status
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "%s status=0x%02" PRIX32, name, (uint32_t)status);
    return dw3000_hal_aes_engine_check_status(status);
}

static bool dw3000_hal_aes_engine_run_test(dw3000_device_t* device) {
    uint8_t key_readback[sizeof(DW3000_HAL_AES_ENGINE_KEY.bytes)];
    uint8_t ciphertext[DW3000_HAL_AES_ENGINE_PAYLOAD_LEN];
    uint8_t recovered[DW3000_HAL_AES_ENGINE_PAYLOAD_LEN];
    uint8_t fill[DW3000_HAL_AES_ENGINE_PAYLOAD_LEN];
    dw3000_aes_key_t key_read;
    dw3000_txrx_event_t events;
    bool ok = true;

    memset(fill, 0xA5, sizeof(fill));

    if (!DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_clear_events(device)) ||
        !DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_write_key_ram_slot(
            device,
            DW3000_HAL_AES_ENGINE_KEY_SLOT,
            &DW3000_HAL_AES_ENGINE_KEY
        )) ||
        !DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_read_key_ram_slot(
            device,
            DW3000_HAL_AES_ENGINE_KEY_SLOT,
            &key_read
        ))) {
        return false;
    }

    memcpy(key_readback, key_read.bytes, sizeof(key_readback));
    ok = dw3000_hal_aes_engine_check_bytes_equal(
        "AES key RAM slot",
        key_readback,
        DW3000_HAL_AES_ENGINE_KEY.bytes,
        sizeof(key_readback)
    ) && ok;

    if (!DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_write_scratch(
            device,
            DW3000_HAL_AES_ENGINE_PLAINTEXT_OFFSET,
            DW3000_HAL_AES_ENGINE_PAYLOAD,
            sizeof(DW3000_HAL_AES_ENGINE_PAYLOAD)
        )) ||
        !DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_write_scratch(
            device,
            DW3000_HAL_AES_ENGINE_CIPHERTEXT_OFFSET,
            fill,
            sizeof(fill)
        )) ||
        !DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_write_scratch(
            device,
            DW3000_HAL_AES_ENGINE_RECOVERED_OFFSET,
            fill,
            sizeof(fill)
        ))) {
        return false;
    }

    if (!dw3000_hal_aes_engine_run_transform(
            device,
            "encrypt",
            DW3000_AES_MODE_ENCRYPT,
            DW3000_HAL_AES_ENGINE_PLAINTEXT_OFFSET,
            DW3000_HAL_AES_ENGINE_CIPHERTEXT_OFFSET
        )) {
        return false;
    }

    if (!DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_read_scratch(
            device,
            DW3000_HAL_AES_ENGINE_CIPHERTEXT_OFFSET,
            ciphertext,
            sizeof(ciphertext)
        ))) {
        return false;
    }

    dw3000_hal_aes_engine_log_bytes(
        "plaintext",
        DW3000_HAL_AES_ENGINE_PAYLOAD,
        sizeof(DW3000_HAL_AES_ENGINE_PAYLOAD)
    );
    dw3000_hal_aes_engine_log_bytes("ciphertext", ciphertext, sizeof(ciphertext));
    ok = dw3000_hal_aes_engine_check_bytes_differ(
        "ciphertext",
        ciphertext,
        DW3000_HAL_AES_ENGINE_PAYLOAD,
        sizeof(ciphertext)
    ) && ok;

    if (!dw3000_hal_aes_engine_run_transform(
            device,
            "decrypt",
            DW3000_AES_MODE_DECRYPT,
            DW3000_HAL_AES_ENGINE_CIPHERTEXT_OFFSET,
            DW3000_HAL_AES_ENGINE_RECOVERED_OFFSET
        )) {
        return false;
    }

    if (!DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_read_scratch(
            device,
            DW3000_HAL_AES_ENGINE_RECOVERED_OFFSET,
            recovered,
            sizeof(recovered)
        ))) {
        return false;
    }

    dw3000_hal_aes_engine_log_bytes("recovered", recovered, sizeof(recovered));
    ok = dw3000_hal_aes_engine_check_bytes_equal(
        "recovered plaintext",
        recovered,
        DW3000_HAL_AES_ENGINE_PAYLOAD,
        sizeof(recovered)
    ) && ok;

    if (!DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_status_read(
            device,
            &events
        ))) {
        return false;
    }
    ESP_LOGI(TAG, "SYS_STATUS=0x%012" PRIX64, (uint64_t)events);

    return DW3000_HAL_AES_ENGINE_CHECK_DW3000(dw3000_hal_aes_clear_events(device)) && ok;
}

void app_main(void) {
    dw3000_hal_aes_engine_app_t app;
    bool                        ok;

    ok = dw3000_hal_aes_engine_setup(&app) &&
         dw3000_hal_aes_engine_initialize(&app) &&
         dw3000_hal_aes_engine_run_test(&app.device);

    ok = dw3000_hal_aes_engine_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_AES_ENGINE_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_AES_ENGINE_FAIL");
    }
}
