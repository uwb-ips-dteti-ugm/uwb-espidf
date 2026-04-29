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
#include "dw3000_hal/rx.h"
#include "dw3000_hal/status.h"
#include "dw3000_register.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DW3000_HAL_RX_BASIC_FRAME_HEADER_LEN 9U
#define DW3000_HAL_RX_BASIC_FRAME_MAX_LEN    127U
#define DW3000_HAL_RX_BASIC_POLL_DELAY_US    1000U
#define DW3000_HAL_RX_BASIC_REARM_DELAY_US   1000U

static const char* TAG = "dw3000_hal_rx_basic";

static const uint8_t DW3000_HAL_RX_BASIC_PAYLOAD_MAGIC[] = "DW3000-HAL-TX";

typedef struct {
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
} dw3000_hal_rx_basic_app_t;

static bool dw3000_hal_rx_basic_check_esp(
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

static bool dw3000_hal_rx_basic_check_dw3000(
    dw3000_error_t err,
    const char*    expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(TAG, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

#define DW3000_HAL_RX_BASIC_CHECK_ESP(expr_) \
    dw3000_hal_rx_basic_check_esp((expr_), #expr_)

#define DW3000_HAL_RX_BASIC_CHECK_DW3000(expr_) \
    dw3000_hal_rx_basic_check_dw3000((expr_), #expr_)

static uint16_t dw3000_hal_rx_basic_read_u16_le(const uint8_t* src) {
    return (uint16_t)src[0] | ((uint16_t)src[1] << 8U);
}

static void dw3000_hal_rx_basic_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if (delay_us >= 1000U) {
        TickType_t ticks = pdMS_TO_TICKS((delay_us + 999U) / 1000U);

        if (ticks == 0U) {
            ticks = 1U;
        }

        vTaskDelay(ticks);
    } else if ((delay_us != 0U) && (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

static bool dw3000_hal_rx_basic_frame_matches(
    const uint8_t* frame,
    size_t         frame_len,
    uint8_t*       seq_out
) {
    size_t payload_offset = DW3000_HAL_RX_BASIC_FRAME_HEADER_LEN;
    size_t magic_len      = sizeof(DW3000_HAL_RX_BASIC_PAYLOAD_MAGIC) - 1U;
    uint16_t pan_id;
    uint16_t dst_addr;
    uint16_t src_addr;

    if (frame_len < (payload_offset + magic_len + 1U)) {
        ESP_LOGW(TAG, "short frame len=%u", (unsigned)frame_len);
        return false;
    }

    if ((frame[0] != 0x41U) || (frame[1] != 0x88U)) {
        ESP_LOGW(TAG, "unexpected FCF: %02" PRIX8 " %02" PRIX8, frame[0], frame[1]);
        return false;
    }

    pan_id   = dw3000_hal_rx_basic_read_u16_le(&frame[3]);
    dst_addr = dw3000_hal_rx_basic_read_u16_le(&frame[5]);
    src_addr = dw3000_hal_rx_basic_read_u16_le(&frame[7]);

    if ((pan_id != DW3000_HAL_RX_BASIC_PAN_ID) ||
        (dst_addr != DW3000_HAL_RX_BASIC_SHORT_ADDR) ||
        (src_addr != DW3000_HAL_RX_BASIC_EXPECTED_SRC_ADDR)) {
        ESP_LOGW(
            TAG,
            "unexpected address pan=0x%04" PRIX16 " dst=0x%04" PRIX16
            " src=0x%04" PRIX16,
            pan_id,
            dst_addr,
            src_addr
        );
        return false;
    }

    if (memcmp(
            &frame[payload_offset],
            DW3000_HAL_RX_BASIC_PAYLOAD_MAGIC,
            magic_len
        ) != 0) {
        ESP_LOGW(TAG, "payload magic mismatch");
        return false;
    }

    if (frame[payload_offset + magic_len] != frame[2]) {
        ESP_LOGW(
            TAG,
            "payload seq mismatch header=%u payload=%u",
            (unsigned)frame[2],
            (unsigned)frame[payload_offset + magic_len]
        );
        return false;
    }

    if (seq_out != NULL) {
        *seq_out = frame[2];
    }

    return true;
}

static bool dw3000_hal_rx_basic_setup_spi(dw3000_hal_rx_basic_app_t* app) {
    spi_bus_config_t bus_config = {
        .mosi_io_num     = DW3000_HAL_RX_BASIC_PIN_MOSI,
        .miso_io_num     = DW3000_HAL_RX_BASIC_PIN_MISO,
        .sclk_io_num     = DW3000_HAL_RX_BASIC_PIN_SCK,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = DW3000_HAL_RX_BASIC_SPI_MAX_TRANSFER_SIZE,
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = DW3000_HAL_RX_BASIC_SPI_CLOCK_HZ,
        .mode           = 0,
        .spics_io_num   = DW3000_HAL_RX_BASIC_PIN_CS,
        .queue_size     = 1,
    };
    esp_err_t err;

    err = spi_bus_initialize(
        DW3000_HAL_RX_BASIC_SPI_HOST,
        &bus_config,
        SPI_DMA_CH_AUTO
    );
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return DW3000_HAL_RX_BASIC_CHECK_ESP(err);
    }

    return DW3000_HAL_RX_BASIC_CHECK_ESP(spi_bus_add_device(
        DW3000_HAL_RX_BASIC_SPI_HOST,
        &device_config,
        &app->spi
    ));
}

static bool dw3000_hal_rx_basic_setup_port(dw3000_hal_rx_basic_app_t* app) {
    dw3000_espidf_port_config_t port_config;

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(TAG, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = DW3000_HAL_RX_BASIC_PIN_RST;
    port_config.irq_gpio             = DW3000_HAL_RX_BASIC_PIN_IRQ;
    port_config.configure_reset_gpio = DW3000_HAL_RX_BASIC_CONFIGURE_RST;
    port_config.configure_irq_gpio   = DW3000_HAL_RX_BASIC_CONFIGURE_IRQ;

    return DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_espidf_port_configure(
        app->port,
        &port_config
    ));
}

static bool dw3000_hal_rx_basic_setup(dw3000_hal_rx_basic_app_t* app) {
    memset(app, 0, sizeof(*app));

    if (!dw3000_hal_rx_basic_setup_spi(app)) {
        return false;
    }

    if (!dw3000_hal_rx_basic_setup_port(app)) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        DW3000_HAL_RX_BASIC_SPI_HOST,
        DW3000_HAL_RX_BASIC_SPI_CLOCK_HZ,
        DW3000_HAL_RX_BASIC_PIN_SCK,
        DW3000_HAL_RX_BASIC_PIN_MISO,
        DW3000_HAL_RX_BASIC_PIN_MOSI,
        DW3000_HAL_RX_BASIC_PIN_CS,
        DW3000_HAL_RX_BASIC_PIN_RST,
        DW3000_HAL_RX_BASIC_PIN_IRQ
    );

    return true;
}

static void dw3000_hal_rx_basic_log_init_snapshot(
    dw3000_device_t* device,
    const char*      stage
) {
    dw3000_txrx_event_t status;
    dw3000_device_id_t  id;
    uint32_t            sys_state;
    dw3000_error_t      err;

    err = dw3000_hal_status_read(device, &status);
    if (err == DW3000_ERROR_OK) {
        ESP_LOGE(
            TAG,
            "%s snapshot SYS_STATUS=0x%012" PRIX64,
            stage,
            (uint64_t)status
        );
    } else {
        ESP_LOGE(
            TAG,
            "%s snapshot SYS_STATUS read failed: %s",
            stage,
            dw3000_error_to_string(err)
        );
    }

    err = dw3000_hal_read_device_id(device, &id);
    if (err == DW3000_ERROR_OK) {
        ESP_LOGE(
            TAG,
            "%s snapshot DEV_ID ridtag=0x%04" PRIX16 " model=0x%02" PRIX8
            " ver=0x%01" PRIX8 " rev=0x%01" PRIX8,
            stage,
            id.ridtag,
            id.model,
            id.ver,
            id.rev
        );
    } else {
        ESP_LOGE(
            TAG,
            "%s snapshot DEV_ID read failed: %s",
            stage,
            dw3000_error_to_string(err)
        );
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_SYS_STATE, &sys_state);
    if (err == DW3000_ERROR_OK) {
        ESP_LOGE(
            TAG,
            "%s snapshot SYS_STATE=0x%08" PRIX32
            " state_flags=0x%08" PRIX32,
            stage,
            sys_state,
            (uint32_t)device->state_flags
        );
    } else {
        ESP_LOGE(
            TAG,
            "%s snapshot SYS_STATE read failed: %s state_flags=0x%08" PRIX32,
            stage,
            dw3000_error_to_string(err),
            (uint32_t)device->state_flags
        );
    }
}

static bool dw3000_hal_rx_basic_teardown(dw3000_hal_rx_basic_app_t* app) {
    bool ok = true;

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok       = DW3000_HAL_RX_BASIC_CHECK_ESP(spi_bus_remove_device(app->spi)) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok = DW3000_HAL_RX_BASIC_CHECK_ESP(spi_bus_free(
            DW3000_HAL_RX_BASIC_SPI_HOST
        )) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

static dw3000_error_t dw3000_hal_rx_basic_initialize_once(
    dw3000_hal_rx_basic_app_t*       app,
    const dw3000_device_config_t*    config,
    const dw3000_hal_bringup_t*      bringup,
    const dw3000_hal_init_options_t* options
) {
    dw3000_device_config_t bringup_config = *config;
    dw3000_error_t         err;

    bringup_config.auto_init_pll = false;

    err = dw3000_hal_bringup(&app->device, bringup, app->port, &bringup_config);
    if (err != DW3000_ERROR_OK) {
        ESP_LOGE(
            TAG,
            "bringup failed: %s",
            dw3000_error_to_string(err)
        );
        dw3000_hal_rx_basic_log_init_snapshot(&app->device, "bringup");
        return err;
    }

    ESP_LOGI(
        TAG,
        "bringup ok DEV_ID ridtag=0x%04" PRIX16 " model=0x%02" PRIX8
        " ver=0x%01" PRIX8 " rev=0x%01" PRIX8
        " state_flags=0x%08" PRIX32,
        app->device.id.ridtag,
        app->device.id.model,
        app->device.id.ver,
        app->device.id.rev,
        (uint32_t)app->device.state_flags
    );

    app->device.config = *config;
    app->device.state_flags = (dw3000_device_state_flags_t)(
        app->device.state_flags & ~DW3000_DEVICE_STATE_INITIALIZED
    );

    err = dw3000_hal_configure_device(&app->device, options);
    if (err != DW3000_ERROR_OK) {
        ESP_LOGE(
            TAG,
            "configure failed: %s",
            dw3000_error_to_string(err)
        );
        dw3000_hal_rx_basic_log_init_snapshot(&app->device, "configure");
        return err;
    }

    ESP_LOGI(
        TAG,
        "configure ok state_flags=0x%08" PRIX32,
        (uint32_t)app->device.state_flags
    );

    return DW3000_ERROR_OK;
}

static bool dw3000_hal_rx_basic_initialize(dw3000_hal_rx_basic_app_t* app) {
    dw3000_device_config_t     config;
    dw3000_hal_bringup_t       bringup;
    dw3000_hal_init_options_t  options;
    dw3000_error_t             err = DW3000_ERROR_TIMEOUT;
    uint32_t                   attempt;

    dw3000_hal_default_config(&config);
    config.load_otp_calibration  = DW3000_HAL_RX_BASIC_LOAD_OTP_CALIBRATION;
    config.auto_init_pll         = DW3000_HAL_RX_BASIC_ENTER_IDLE_PLL;
    config.use_double_buffer     = false;
    config.mac.panadr.pan_id     = DW3000_HAL_RX_BASIC_PAN_ID;
    config.mac.panadr.short_addr = DW3000_HAL_RX_BASIC_SHORT_ADDR;

    dw3000_hal_default_bringup(&bringup);
    bringup.reset_assert_us  = DW3000_HAL_RX_BASIC_RESET_ASSERT_US;
    bringup.reset_settle_us  = DW3000_HAL_RX_BASIC_RESET_SETTLE_US;
    bringup.ready_timeout_us = DW3000_HAL_RX_BASIC_READY_TIMEOUT_US;

    dw3000_hal_default_init_options(&options);
    options.idle_pll_timeout_us = DW3000_HAL_RX_BASIC_IDLE_PLL_TIMEOUT_US;

    for (attempt = 1U;
         attempt <= DW3000_HAL_RX_BASIC_INIT_RETRIES;
         ++attempt) {
        ESP_LOGI(
            TAG,
            "init attempt %" PRIu32 "/%" PRIu32
            " reset_assert=%" PRIu32 "us reset_settle=%" PRIu32
            "us ready_timeout=%" PRIu32 "us idle_pll_timeout=%" PRIu32 "us",
            attempt,
            (uint32_t)DW3000_HAL_RX_BASIC_INIT_RETRIES,
            bringup.reset_assert_us,
            bringup.reset_settle_us,
            bringup.ready_timeout_us,
            options.idle_pll_timeout_us
        );

        err = dw3000_hal_rx_basic_initialize_once(
            app,
            &config,
            &bringup,
            &options
        );
        if (err == DW3000_ERROR_OK) {
            return true;
        }

        if (attempt < DW3000_HAL_RX_BASIC_INIT_RETRIES) {
            vTaskDelay(pdMS_TO_TICKS(20U));
        }
    }

    ESP_LOGE(
        TAG,
        "initialize failed after %" PRIu32 " attempts: %s",
        (uint32_t)DW3000_HAL_RX_BASIC_INIT_RETRIES,
        dw3000_error_to_string(err)
    );

    return false;
}

static bool dw3000_hal_rx_basic_arm(dw3000_device_t* device) {
    return DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_fcmd_txrxoff(device)) &&
           DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_status_clear_all(device)) &&
           DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_rx_start_immediate(device));
}

static bool dw3000_hal_rx_basic_rearm(dw3000_device_t* device) {
    if (!dw3000_hal_rx_basic_arm(device)) {
        return false;
    }

    dw3000_hal_rx_basic_delay_us(device, DW3000_HAL_RX_BASIC_REARM_DELAY_US);
    return true;
}

static bool dw3000_hal_rx_basic_read_received_frame(
    dw3000_device_t*        device,
    dw3000_txrx_event_t     events,
    bool*                   matched
) {
    uint8_t                    frame[DW3000_HAL_RX_BASIC_FRAME_MAX_LEN];
    uint8_t                    seq = 0U;
    size_t                     read_len;
    dw3000_txrx_rx_finfo_t     finfo;
    dw3000_txrx_timestamp_t    rx_timestamp;

    *matched = false;

    if (!DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_rx_read_finfo(
            device,
            &finfo
        ))) {
        return false;
    }

    read_len = finfo.rx_flen;
    if (read_len > sizeof(frame)) {
        read_len = sizeof(frame);
    }

    if (!DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_rx_read_active_buffer(
            device,
            0U,
            frame,
            read_len
        )) ||
        !DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_rx_read_timestamp(
            device,
            &rx_timestamp
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "rx len=%u pacc=%u data_rate=%u prf=%u rx_ts=0x%010" PRIX64
        " SYS_STATUS=0x%012" PRIX64,
        (unsigned)finfo.rx_flen,
        (unsigned)finfo.rx_pacc,
        (unsigned)finfo.data_rate,
        (unsigned)finfo.prf,
        (uint64_t)rx_timestamp,
        (uint64_t)events
    );

    if (dw3000_hal_rx_basic_frame_matches(frame, read_len, &seq)) {
        ESP_LOGI(TAG, "matched frame seq=%u", (unsigned)seq);
        *matched = true;
    }

    return true;
}

static bool dw3000_hal_rx_basic_run_test(dw3000_device_t* device) {
    dw3000_txrx_event_t saved_enabled;
    dw3000_txrx_event_t test_enabled = dw3000_hal_rx_all_events();
    dw3000_txrx_event_t rx_success_events = dw3000_hal_rx_success_events();
    dw3000_txrx_event_t rx_error_events = dw3000_hal_rx_error_events();
    dw3000_txrx_event_t events = 0U;
    uint64_t            start_us;
    uint32_t            rx_error_count = 0U;
    bool                ok = true;
    bool                matched = false;

    if ((device == NULL) || (device->port.get_time_us == NULL)) {
        ESP_LOGE(TAG, "RX test requires get_time_us port callback");
        return false;
    }

    if (!DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_status_read_enabled(
            device,
            &saved_enabled
        )) ||
        !DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_status_set_enabled(
            device,
            test_enabled
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "saved SYS_ENABLE=0x%012" PRIX64, (uint64_t)saved_enabled);
    ESP_LOGI(TAG, "test SYS_ENABLE=0x%012" PRIX64, (uint64_t)test_enabled);

    if (!dw3000_hal_rx_basic_arm(device)) {
        (void)dw3000_hal_status_set_enabled(device, saved_enabled);
        return false;
    }

    start_us = device->port.get_time_us(device->port.ctx);
    while ((device->port.get_time_us(device->port.ctx) - start_us) <
           DW3000_HAL_RX_BASIC_WAIT_TIMEOUT_US) {
        if (!DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_status_read(
                device,
                &events
            ))) {
            ok = false;
            break;
        }

        if ((events & DW3000_TXRX_EVENT_CMD_ERR) != 0U) {
            ESP_LOGE(TAG, "RX command error SYS_STATUS=0x%012" PRIX64, (uint64_t)events);
            ok = false;
            break;
        } else if ((events & rx_error_events) != 0U) {
            ++rx_error_count;
            if ((rx_error_count <= 5U) || ((rx_error_count % 32U) == 0U)) {
                ESP_LOGW(
                    TAG,
                    "RX error #%u SYS_STATUS=0x%012" PRIX64
                    " err=0x%012" PRIX64 "; rearming",
                    (unsigned)rx_error_count,
                    (uint64_t)events,
                    (uint64_t)(events & rx_error_events)
                );
            }
            if (!dw3000_hal_rx_basic_rearm(device)) {
                ok = false;
                break;
            }
        } else if ((events & DW3000_TXRX_EVENT_RXFCG) != 0U) {
            if (!dw3000_hal_rx_basic_read_received_frame(device, events, &matched)) {
                ok = false;
                break;
            }

            if (matched) {
                break;
            }

            ESP_LOGW(TAG, "valid RX frame did not match test pattern; rearming");
            if (!dw3000_hal_rx_basic_rearm(device)) {
                ok = false;
                break;
            }
        } else if ((events & DW3000_TXRX_EVENT_RXFR) != 0U) {
            ++rx_error_count;
            if ((rx_error_count <= 5U) || ((rx_error_count % 32U) == 0U)) {
                ESP_LOGW(
                    TAG,
                    "partial RX event #%u SYS_STATUS=0x%012" PRIX64
                    " success=0x%012" PRIX64 "; rearming",
                    (unsigned)rx_error_count,
                    (uint64_t)events,
                    (uint64_t)(events & rx_success_events)
                );
            }
            if (!dw3000_hal_rx_basic_rearm(device)) {
                ok = false;
                break;
            }
        } else {
            dw3000_hal_rx_basic_delay_us(device, DW3000_HAL_RX_BASIC_POLL_DELAY_US);
        }
    }

    if (ok && !matched) {
        ESP_LOGE(
            TAG,
            "RX timeout after %" PRIu32 " us; last SYS_STATUS=0x%012" PRIX64,
            (uint32_t)DW3000_HAL_RX_BASIC_WAIT_TIMEOUT_US,
            (uint64_t)events
        );
        ok = false;
    }

    (void)dw3000_hal_fcmd_txrxoff(device);
    (void)dw3000_hal_status_clear_all(device);

    return DW3000_HAL_RX_BASIC_CHECK_DW3000(dw3000_hal_status_set_enabled(
        device,
        saved_enabled
    )) && ok;
}

void app_main(void) {
    dw3000_hal_rx_basic_app_t app;
    bool                      ok;

    ok = dw3000_hal_rx_basic_setup(&app) &&
         dw3000_hal_rx_basic_initialize(&app) &&
         dw3000_hal_rx_basic_run_test(&app.device);

    ok = dw3000_hal_rx_basic_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_RX_BASIC_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_RX_BASIC_FAIL");
    }
}
