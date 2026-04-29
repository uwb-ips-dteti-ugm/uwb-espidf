#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "driver/spi_master.h"
#include "dw3000_error.h"
#include "dw3000_espidf.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/mac.h"
#include "dw3000_hal/status.h"
#include "dw3000_register.h"
#include "esp_err.h"
#include "esp_log.h"

static const char* TAG = "dw3000_hal_basic_info";

typedef struct {
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
} dw3000_hal_basic_info_app_t;

static bool dw3000_hal_basic_info_check_esp(
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

static bool dw3000_hal_basic_info_check_dw3000(
    dw3000_error_t err,
    const char*    expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

#define DW3000_HAL_BASIC_INFO_CHECK_ESP(expr_) \
    dw3000_hal_basic_info_check_esp((expr_), #expr_)

#define DW3000_HAL_BASIC_INFO_CHECK_DW3000(expr_) \
    dw3000_hal_basic_info_check_dw3000((expr_), #expr_)

static bool dw3000_hal_basic_info_setup_spi(dw3000_hal_basic_info_app_t* app) {
    spi_bus_config_t bus_config = {
        .mosi_io_num     = DW3000_HAL_BASIC_INFO_PIN_MOSI,
        .miso_io_num     = DW3000_HAL_BASIC_INFO_PIN_MISO,
        .sclk_io_num     = DW3000_HAL_BASIC_INFO_PIN_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = DW3000_HAL_BASIC_INFO_SPI_MAX_TRANSFER_SIZE,
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = DW3000_HAL_BASIC_INFO_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = DW3000_HAL_BASIC_INFO_PIN_CS,
        .queue_size     = 1,
    };
    esp_err_t err;

    err = spi_bus_initialize(DW3000_HAL_BASIC_INFO_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return DW3000_HAL_BASIC_INFO_CHECK_ESP(err);
    }

    return DW3000_HAL_BASIC_INFO_CHECK_ESP(spi_bus_add_device(
        DW3000_HAL_BASIC_INFO_SPI_HOST,
        &device_config,
        &app->spi
    ));
}

static bool dw3000_hal_basic_info_setup_port(dw3000_hal_basic_info_app_t* app) {
    dw3000_espidf_port_config_t port_config;

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(TAG, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = DW3000_HAL_BASIC_INFO_PIN_RST;
    port_config.irq_gpio             = DW3000_HAL_BASIC_INFO_PIN_IRQ;
    port_config.configure_reset_gpio = DW3000_HAL_BASIC_INFO_CONFIGURE_RST;
    port_config.configure_irq_gpio   = DW3000_HAL_BASIC_INFO_CONFIGURE_IRQ;

    return DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_espidf_port_configure(
        app->port,
        &port_config
    ));
}

static bool dw3000_hal_basic_info_setup(dw3000_hal_basic_info_app_t* app) {
    memset(app, 0, sizeof(*app));

    if (!dw3000_hal_basic_info_setup_spi(app)) {
        return false;
    }

    if (!dw3000_hal_basic_info_setup_port(app)) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        DW3000_HAL_BASIC_INFO_SPI_HOST,
        DW3000_HAL_BASIC_INFO_SPI_CLOCK_HZ,
        DW3000_HAL_BASIC_INFO_PIN_SCK,
        DW3000_HAL_BASIC_INFO_PIN_MISO,
        DW3000_HAL_BASIC_INFO_PIN_MOSI,
        DW3000_HAL_BASIC_INFO_PIN_CS,
        DW3000_HAL_BASIC_INFO_PIN_RST,
        DW3000_HAL_BASIC_INFO_PIN_IRQ
    );

    return DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_hal_bringup(
        &app->device,
        NULL,
        app->port,
        NULL
    ));
}

static bool dw3000_hal_basic_info_teardown(dw3000_hal_basic_info_app_t* app) {
    bool ok = true;

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok       = DW3000_HAL_BASIC_INFO_CHECK_ESP(spi_bus_remove_device(app->spi)) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok                   = DW3000_HAL_BASIC_INFO_CHECK_ESP(spi_bus_free(DW3000_HAL_BASIC_INFO_SPI_HOST)) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

static bool dw3000_hal_basic_info_read_registers(dw3000_device_t* device) {
    dw3000_device_id_t  id;
    dw3000_txrx_event_t status;
    dw3000_txrx_event_t enabled_events;
    dw3000_mac_eui_t    eui;
    dw3000_mac_panadr_t panadr;
    uint32_t            sys_cfg;
    uint32_t            sys_time;
    uint32_t            sys_state;
    uint8_t             fint_stat;

    if (!DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_hal_read_device_id(device, &id))) {
        return false;
    }

    if (!dw3000_hal_is_supported_device_id(&id)) {
        ESP_LOGE(
            TAG,
            "Unsupported DEV_ID ridtag=0x%04" PRIX16 " model=0x%02" PRIX8
            " ver=0x%01" PRIX8 " rev=0x%01" PRIX8,
            id.ridtag,
            id.model,
            id.ver,
            id.rev
        );
        return false;
    }

    if (!DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_hal_status_read(device, &status)) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_hal_status_read_enabled(
            device,
            &enabled_events
        )) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_hal_mac_read_eui(device, &eui)) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_hal_mac_read_panadr(device, &panadr)) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_SYS_CFG,
            &sys_cfg
        )) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_SYS_TIME,
            &sys_time
        )) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_reg_read_u32(
            device,
            DW3000_REG_SYS_STATE,
            &sys_state
        )) ||
        !DW3000_HAL_BASIC_INFO_CHECK_DW3000(dw3000_reg_read_u8(
            device,
            DW3000_REG_FINT_STAT,
            &fint_stat
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "DEV_ID ridtag=0x%04" PRIX16 " model=0x%02" PRIX8
        " ver=0x%01" PRIX8 " rev=0x%01" PRIX8,
        id.ridtag,
        id.model,
        id.ver,
        id.rev
    );
    ESP_LOGI(TAG, "SYS_STATUS=0x%012" PRIX64, (uint64_t)status);
    ESP_LOGI(TAG, "SYS_ENABLE=0x%012" PRIX64, (uint64_t)enabled_events);
    ESP_LOGI(TAG, "SYS_CFG=0x%08" PRIX32, sys_cfg);
    ESP_LOGI(TAG, "SYS_TIME=0x%08" PRIX32, sys_time);
    ESP_LOGI(TAG, "SYS_STATE=0x%08" PRIX32, sys_state);
    ESP_LOGI(TAG, "FINT_STAT=0x%02" PRIX8, fint_stat);
    ESP_LOGI(TAG, "EUI=0x%016" PRIX64, eui);
    ESP_LOGI(
        TAG,
        "PANADR pan_id=0x%04" PRIX16 " short_addr=0x%04" PRIX16,
        panadr.pan_id,
        panadr.short_addr
    );

    return true;
}

void app_main(void) {
    dw3000_hal_basic_info_app_t app;
    bool                  ok;

    ok = dw3000_hal_basic_info_setup(&app) &&
         dw3000_hal_basic_info_read_registers(&app.device);

    ok = dw3000_hal_basic_info_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_BASIC_INFO_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_BASIC_INFO_FAIL");
    }
}
