#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "driver/spi_master.h"
#include "dw3000_error.h"
#include "dw3000_espidf.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/mac.h"
#include "dw3000_hal/status.h"
#include "dw3000_hal/tx.h"
#include "esp_err.h"
#include "esp_log.h"

#define DW3000_HAL_TX_BASIC_FRAME_HEADER_LEN 9U
#define DW3000_HAL_TX_BASIC_FRAME_MAX_LEN    64U
#define DW3000_HAL_TX_BASIC_POLL_DELAY_US    100U

#define DW3000_HAL_TX_BASIC_ERROR_EVENTS \
    (DW3000_TXRX_EVENT_HPDWARN |         \
     DW3000_TXRX_EVENT_SPICRCE |         \
     DW3000_TXRX_EVENT_CMD_ERR |         \
     DW3000_TXRX_EVENT_SPI_OVF |         \
     DW3000_TXRX_EVENT_SPI_UNF |         \
     DW3000_TXRX_EVENT_SPIERR)

static const char* TAG = "dw3000_hal_tx_basic";

static const uint8_t DW3000_HAL_TX_BASIC_PAYLOAD_MAGIC[] = "DW3000-HAL-TX";

typedef struct {
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
} dw3000_hal_tx_basic_app_t;

static bool dw3000_hal_tx_basic_check_esp(
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

static bool dw3000_hal_tx_basic_check_dw3000(
    dw3000_error_t err,
    const char*    expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

#define DW3000_HAL_TX_BASIC_CHECK_ESP(expr_) \
    dw3000_hal_tx_basic_check_esp((expr_), #expr_)

#define DW3000_HAL_TX_BASIC_CHECK_DW3000(expr_) \
    dw3000_hal_tx_basic_check_dw3000((expr_), #expr_)

static void dw3000_hal_tx_basic_write_u16_le(uint8_t* dst, uint16_t value) {
    dst[0] = (uint8_t)(value & 0xFFU);
    dst[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static size_t dw3000_hal_tx_basic_build_frame(uint8_t seq, uint8_t* frame) {
    size_t offset = 0U;

    frame[offset++] = 0x41U;
    frame[offset++] = 0x88U;
    frame[offset++] = seq;
    dw3000_hal_tx_basic_write_u16_le(&frame[offset], DW3000_HAL_TX_BASIC_PAN_ID);
    offset += 2U;
    dw3000_hal_tx_basic_write_u16_le(&frame[offset], DW3000_HAL_TX_BASIC_DST_ADDR);
    offset += 2U;
    dw3000_hal_tx_basic_write_u16_le(&frame[offset], DW3000_HAL_TX_BASIC_SRC_ADDR);
    offset += 2U;

    memcpy(
        &frame[offset],
        DW3000_HAL_TX_BASIC_PAYLOAD_MAGIC,
        sizeof(DW3000_HAL_TX_BASIC_PAYLOAD_MAGIC) - 1U
    );
    offset += sizeof(DW3000_HAL_TX_BASIC_PAYLOAD_MAGIC) - 1U;
    frame[offset++] = seq;

    return offset;
}

static bool dw3000_hal_tx_basic_setup_spi(dw3000_hal_tx_basic_app_t* app) {
    spi_bus_config_t bus_config = {
        .mosi_io_num     = DW3000_HAL_TX_BASIC_PIN_MOSI,
        .miso_io_num     = DW3000_HAL_TX_BASIC_PIN_MISO,
        .sclk_io_num     = DW3000_HAL_TX_BASIC_PIN_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = DW3000_HAL_TX_BASIC_SPI_MAX_TRANSFER_SIZE,
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = DW3000_HAL_TX_BASIC_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = DW3000_HAL_TX_BASIC_PIN_CS,
        .queue_size     = 1,
    };
    esp_err_t err;

    err = spi_bus_initialize(
        DW3000_HAL_TX_BASIC_SPI_HOST,
        &bus_config,
        SPI_DMA_CH_AUTO
    );
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return DW3000_HAL_TX_BASIC_CHECK_ESP(err);
    }

    return DW3000_HAL_TX_BASIC_CHECK_ESP(spi_bus_add_device(
        DW3000_HAL_TX_BASIC_SPI_HOST,
        &device_config,
        &app->spi
    ));
}

static bool dw3000_hal_tx_basic_setup_port(dw3000_hal_tx_basic_app_t* app) {
    dw3000_espidf_port_config_t port_config;

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(TAG, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = DW3000_HAL_TX_BASIC_PIN_RST;
    port_config.irq_gpio             = DW3000_HAL_TX_BASIC_PIN_IRQ;
    port_config.configure_reset_gpio = DW3000_HAL_TX_BASIC_CONFIGURE_RST;
    port_config.configure_irq_gpio   = DW3000_HAL_TX_BASIC_CONFIGURE_IRQ;

    return DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_espidf_port_configure(
        app->port,
        &port_config
    ));
}

static bool dw3000_hal_tx_basic_setup(dw3000_hal_tx_basic_app_t* app) {
    memset(app, 0, sizeof(*app));

    if (!dw3000_hal_tx_basic_setup_spi(app)) {
        return false;
    }

    if (!dw3000_hal_tx_basic_setup_port(app)) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        DW3000_HAL_TX_BASIC_SPI_HOST,
        DW3000_HAL_TX_BASIC_SPI_CLOCK_HZ,
        DW3000_HAL_TX_BASIC_PIN_SCK,
        DW3000_HAL_TX_BASIC_PIN_MISO,
        DW3000_HAL_TX_BASIC_PIN_MOSI,
        DW3000_HAL_TX_BASIC_PIN_CS,
        DW3000_HAL_TX_BASIC_PIN_RST,
        DW3000_HAL_TX_BASIC_PIN_IRQ
    );

    return true;
}

static bool dw3000_hal_tx_basic_teardown(dw3000_hal_tx_basic_app_t* app) {
    bool ok = true;

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok       = DW3000_HAL_TX_BASIC_CHECK_ESP(spi_bus_remove_device(app->spi)) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok = DW3000_HAL_TX_BASIC_CHECK_ESP(spi_bus_free(
            DW3000_HAL_TX_BASIC_SPI_HOST
        )) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

static bool dw3000_hal_tx_basic_initialize(dw3000_hal_tx_basic_app_t* app) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);
    config.load_otp_calibration = DW3000_HAL_TX_BASIC_LOAD_OTP_CALIBRATION;
    config.auto_init_pll        = DW3000_HAL_TX_BASIC_ENTER_IDLE_PLL;
    config.mac.panadr.pan_id    = DW3000_HAL_TX_BASIC_PAN_ID;
    config.mac.panadr.short_addr = DW3000_HAL_TX_BASIC_SRC_ADDR;

    return DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_initialize(
        &app->device,
        NULL,
        NULL,
        app->port,
        &config
    ));
}

static bool dw3000_hal_tx_basic_wait_done(
    dw3000_device_t*      device,
    uint32_t              timeout_us,
    dw3000_txrx_event_t*  events_out
) {
    uint64_t start_us;
    dw3000_txrx_event_t events = 0U;

    if ((device == NULL) || (device->port.get_time_us == NULL)) {
        ESP_LOGE(TAG, "TX wait requires get_time_us port callback");
        return false;
    }

    start_us = device->port.get_time_us(device->port.ctx);
    do {
        if (!DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_status_read(
                device,
                &events
            ))) {
            return false;
        }

        if ((events & DW3000_TXRX_EVENT_TXFRS) != 0U) {
            if (events_out != NULL) {
                *events_out = events;
            }
            return true;
        }

        if ((events & DW3000_HAL_TX_BASIC_ERROR_EVENTS) != 0U) {
            ESP_LOGE(TAG, "TX error SYS_STATUS=0x%012" PRIX64, (uint64_t)events);
            return false;
        }

        if (device->port.delay_us != NULL) {
            device->port.delay_us(device->port.ctx, DW3000_HAL_TX_BASIC_POLL_DELAY_US);
        }
    } while ((device->port.get_time_us(device->port.ctx) - start_us) < timeout_us);

    ESP_LOGE(TAG, "TX timeout SYS_STATUS=0x%012" PRIX64, (uint64_t)events);
    return false;
}

static bool dw3000_hal_tx_basic_send_one(
    dw3000_device_t* device,
    uint8_t          seq
) {
    uint8_t                 frame_data[DW3000_HAL_TX_BASIC_FRAME_MAX_LEN];
    size_t                  frame_len;
    dw3000_txrx_tx_frame_t  frame;
    dw3000_txrx_event_t     events;
    dw3000_txrx_timestamp_t tx_timestamp;

    frame_len = dw3000_hal_tx_basic_build_frame(seq, frame_data);
    if ((frame_len + DW3000_HAL_TX_AUTO_FCS_LEN) > DW3000_HAL_TX_MAX_FRAME_STANDARD) {
        ESP_LOGE(TAG, "frame too long: %u", (unsigned)frame_len);
        return false;
    }

    frame.tx_flen     = (uint16_t)(frame_len + DW3000_HAL_TX_AUTO_FCS_LEN);
    frame.ranging     = false;
    frame.tx_b_offset = 0U;
    frame.fine_plen   = 0U;

    if (!DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_fcmd_txrxoff(device)) ||
        !DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_status_clear_all(device)) ||
        !DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_tx_prepare_frame(
            device,
            frame_data,
            frame_len,
            &frame
        )) ||
        !DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_tx_start_immediate(device))) {
        return false;
    }

    if (!dw3000_hal_tx_basic_wait_done(
            device,
            DW3000_HAL_TX_BASIC_TX_TIMEOUT_US,
            &events
        )) {
        (void)dw3000_hal_fcmd_txrxoff(device);
        return false;
    }

    if (!DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_tx_read_timestamp(
            device,
            &tx_timestamp
        ))) {
        (void)dw3000_hal_fcmd_txrxoff(device);
        return false;
    }

    ESP_LOGI(
        TAG,
        "tx seq=%u len=%u tx_ts=0x%010" PRIX64 " SYS_STATUS=0x%012" PRIX64,
        (unsigned)seq,
        (unsigned)frame_len,
        (uint64_t)tx_timestamp,
        (uint64_t)events
    );

    return DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_fcmd_txrxoff(device)) &&
           DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_status_clear_all(device));
}

static bool dw3000_hal_tx_basic_run_test(dw3000_device_t* device) {
    dw3000_txrx_event_t saved_enabled;
    dw3000_txrx_event_t test_enabled =
        DW3000_TXRX_EVENT_TXFRS | DW3000_HAL_TX_BASIC_ERROR_EVENTS;
    bool ok = true;

    if (!DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_status_read_enabled(
            device,
            &saved_enabled
        )) ||
        !DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_status_set_enabled(
            device,
            test_enabled
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "saved SYS_ENABLE=0x%012" PRIX64, (uint64_t)saved_enabled);
    ESP_LOGI(TAG, "test SYS_ENABLE=0x%012" PRIX64, (uint64_t)test_enabled);

    for (uint8_t seq = 0U; seq < DW3000_HAL_TX_BASIC_FRAME_COUNT; ++seq) {
        if (!dw3000_hal_tx_basic_send_one(device, seq)) {
            ok = false;
            break;
        }

        if ((device->port.delay_us != NULL) &&
            (seq + 1U < DW3000_HAL_TX_BASIC_FRAME_COUNT)) {
            device->port.delay_us(
                device->port.ctx,
                DW3000_HAL_TX_BASIC_FRAME_INTERVAL_US
            );
        }
    }

    (void)dw3000_hal_fcmd_txrxoff(device);
    (void)dw3000_hal_status_clear_all(device);

    return DW3000_HAL_TX_BASIC_CHECK_DW3000(dw3000_hal_status_set_enabled(
        device,
        saved_enabled
    )) && ok;
}

void app_main(void) {
    dw3000_hal_tx_basic_app_t app;
    bool                      ok;

    ok = dw3000_hal_tx_basic_setup(&app) &&
         dw3000_hal_tx_basic_initialize(&app) &&
         dw3000_hal_tx_basic_run_test(&app.device);

    ok = dw3000_hal_tx_basic_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_TX_BASIC_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_TX_BASIC_FAIL");
    }
}
