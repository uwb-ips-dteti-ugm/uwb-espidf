#include "hal_ss_twr_common.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "driver/spi_master.h"
#include "dw3000_error.h"
#include "dw3000_hal/calib.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/gpio.h"
#include "dw3000_hal/phy.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_hal/status.h"
#include "dw3000_register.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DW3000_SS_TWR_TX_FCTRL_PHY_MASK 0x0000F400UL

static const uint8_t DW3000_SS_TWR_MAGIC[] = {
    'D', 'W', '3', 'T', 'W', 'R',
};

static bool check_esp(
    const char* tag,
    esp_err_t   err,
    const char* expression
) {
    if (err == ESP_OK) {
        return true;
    }

    ESP_LOGE(tag, "%s failed: %s", expression, esp_err_to_name(err));
    return false;
}

bool dw3000_ss_twr_check_dw3000(
    const char*     tag,
    dw3000_error_t  err,
    const char*     expression
) {
    if (err == DW3000_ERROR_OK) {
        return true;
    }

    ESP_LOGE(tag, "%s failed: %s", expression, dw3000_error_to_string(err));
    return false;
}

static void write_u16_le(uint8_t* dst, uint16_t value) {
    dst[0] = (uint8_t)(value & 0xFFU);
    dst[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static uint16_t read_u16_le(const uint8_t* src) {
    return (uint16_t)src[0] | ((uint16_t)src[1] << 8U);
}

static void write_ts40_le(uint8_t* dst, dw3000_txrx_timestamp_t value) {
    value &= DW3000_SS_TWR_TIMESTAMP_MASK;

    for (size_t i = 0U; i < DW3000_SS_TWR_TIMESTAMP_LEN; ++i) {
        dst[i] = (uint8_t)((value >> (8U * i)) & 0xFFU);
    }
}

static dw3000_txrx_timestamp_t read_ts40_le(const uint8_t* src) {
    dw3000_txrx_timestamp_t value = 0U;

    for (size_t i = 0U; i < DW3000_SS_TWR_TIMESTAMP_LEN; ++i) {
        value |= (dw3000_txrx_timestamp_t)src[i] << (8U * i);
    }

    return value & DW3000_SS_TWR_TIMESTAMP_MASK;
}

static size_t build_header(
    uint8_t  seq,
    uint16_t pan_id,
    uint16_t src_addr,
    uint16_t dst_addr,
    uint8_t* frame
) {
    size_t offset = 0U;

    frame[offset++] = 0x41U;
    frame[offset++] = 0x88U;
    frame[offset++] = seq;
    write_u16_le(&frame[offset], pan_id);
    offset += 2U;
    write_u16_le(&frame[offset], dst_addr);
    offset += 2U;
    write_u16_le(&frame[offset], src_addr);
    offset += 2U;

    return offset;
}

static size_t build_common_payload(
    uint8_t                 seq,
    dw3000_ss_twr_msg_type_t type,
    uint16_t                pan_id,
    uint16_t                src_addr,
    uint16_t                dst_addr,
    uint8_t*                frame
) {
    size_t offset = build_header(seq, pan_id, src_addr, dst_addr, frame);

    memcpy(&frame[offset], DW3000_SS_TWR_MAGIC, sizeof(DW3000_SS_TWR_MAGIC));
    offset += sizeof(DW3000_SS_TWR_MAGIC);
    frame[offset++] = (uint8_t)type;
    frame[offset++] = seq;

    return offset;
}

static uint32_t expected_tx_fctrl_phy_bits(
    const dw3000_device_config_t* config
) {
    uint32_t bits = (uint32_t)config->phy.preamble_length << 12U;

    if (config->phy.data_rate == DW3000_PHY_DATA_RATE_6M81) {
        bits |= 1UL << 10U;
    }

    return bits;
}

static void apply_radio_profile(
    dw3000_device_config_t*                 config,
    const dw3000_ss_twr_device_config_t*    test_config
) {
    config->phy.preamble_length = DW3000_PHY_PREAMBLE_LEN_128;
    config->phy.sfd_type        = DW3000_PHY_SFD_TYPE_DECAWAVE_8;
    config->phy.pac_size        = DW3000_PHY_PAC_SIZE_8;
    config->rx_tune.sfd_toc     = dw3000_hal_phy_sfd_timeout(&config->phy);
    config->rx_tune.dtune3      = test_config->dtune3;
    config->sts.packet_cfg      = DW3000_STS_PACKET_CFG_SP0;
    config->sts.pdoa_mode       = DW3000_STS_PDOA_MODE_DISABLED;
    config->sts.sys_cfg_flags   = 0U;
    config->cia.tx_antd         = test_config->tx_antenna_delay;
    config->cia.rx_antd         = test_config->rx_antenna_delay;
    config->calib.tx_power.data = test_config->tx_power_byte;
    config->calib.tx_power.phr  = test_config->tx_power_byte;
    config->calib.tx_power.shr  = test_config->tx_power_byte;
    config->calib.tx_power.sts  = test_config->tx_power_byte;

    if (test_config->enable_extpa) {
        config->pmsc.txfseq = DW3000_PMSC_TXFSEQ_FINE_DISABLED;
    }
}

static bool log_radio_config(
    dw3000_ss_twr_app_t*        app,
    const dw3000_device_config_t* config
) {
    uint32_t tx_fctrl;
    uint32_t dtune3;
    uint16_t chan_ctrl;
    uint16_t rx_sfd_toc;
    uint32_t expected_tx_fctrl;

    if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_reg_read_u32(
            &app->device,
            DW3000_REG_TX_FCTRL,
            &tx_fctrl
        )) ||
        !DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_reg_read_u16(
            &app->device,
            DW3000_REG_CHAN_CTRL,
            &chan_ctrl
        )) ||
        !DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_reg_read_u32(
            &app->device,
            DW3000_REG_DTUNE3,
            &dtune3
        )) ||
        !DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_reg_read_u16(
            &app->device,
            DW3000_REG_RX_SFD_TOC,
            &rx_sfd_toc
        ))) {
        return false;
    }

    ESP_LOGI(
        app->tag,
        "radio TX_FCTRL=0x%08" PRIX32 " CHAN_CTRL=0x%04" PRIX16
        " DTUNE3=0x%08" PRIX32 " RX_SFD_TOC=%" PRIu16,
        tx_fctrl,
        chan_ctrl,
        dtune3,
        rx_sfd_toc
    );

    expected_tx_fctrl = expected_tx_fctrl_phy_bits(config);
    if (((tx_fctrl & DW3000_SS_TWR_TX_FCTRL_PHY_MASK) != expected_tx_fctrl) ||
        (dtune3 != config->rx_tune.dtune3) ||
        (rx_sfd_toc != config->rx_tune.sfd_toc)) {
        ESP_LOGE(
            app->tag,
            "radio readback mismatch expected TX_FCTRL[phy]=0x%04" PRIX32
            " DTUNE3=0x%08" PRIX32 " RX_SFD_TOC=%" PRIu16,
            expected_tx_fctrl,
            config->rx_tune.dtune3,
            config->rx_tune.sfd_toc
        );
        return false;
    }

    return true;
}

static bool run_rx_calibration(
    dw3000_ss_twr_app_t*                  app,
    const dw3000_ss_twr_device_config_t*  test_config
) {
    dw3000_error_t err;
    dw3000_calib_rx_cal_result_t result = {
        .i = DW3000_CALIB_RX_CAL_RESULT_FAIL,
        .q = DW3000_CALIB_RX_CAL_RESULT_FAIL,
    };

    if (!test_config->run_rx_calibration) {
        return true;
    }

    err = dw3000_hal_calib_run_rx_calibration(
        &app->device,
        test_config->rx_cal_timeout_us,
        &result
    );
    if (err != DW3000_ERROR_OK) {
        ESP_LOGE(
            app->tag,
            "RX_CAL failed: %s resi=0x%08" PRIX32 " resq=0x%08" PRIX32,
            dw3000_error_to_string(err),
            result.i,
            result.q
        );
        return false;
    }

    ESP_LOGI(
        app->tag,
        "RX_CAL resi=0x%08" PRIX32 " resq=0x%08" PRIX32,
        result.i,
        result.q
    );
    return true;
}

static bool configure_radio_profile(
    dw3000_ss_twr_app_t*                  app,
    const dw3000_device_config_t*         config,
    const dw3000_ss_twr_device_config_t*  test_config
) {
    dw3000_error_t err;

    ESP_LOGI(
        app->tag,
        "desired radio plen=%u pac=%u sfd_toc=%" PRIu16
        " sts_packet=%u sts_flags=0x%08" PRIX32
        " tx_antd=%" PRIu16 " rx_antd=%" PRIu16,
        (unsigned)dw3000_hal_phy_preamble_symbols(config->phy.preamble_length),
        (unsigned)dw3000_hal_phy_pac_symbols(config->phy.pac_size),
        config->rx_tune.sfd_toc,
        (unsigned)config->sts.packet_cfg,
        (uint32_t)config->sts.sys_cfg_flags,
        config->cia.tx_antd,
        config->cia.rx_antd
    );

    if ((app->device.state_flags & DW3000_DEVICE_STATE_IDLE_PLL) != 0U) {
        err = dw3000_hal_pmsc_force_idle_rc(&app->device);
        if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, err)) {
            return false;
        }
    }

    err = dw3000_hal_phy_configure(
        &app->device,
        &config->phy,
        &config->rx_tune
    );
    if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, err)) {
        return false;
    }

    if (config->auto_init_pll) {
        err = dw3000_hal_pmsc_enter_idle_pll(
            &app->device,
            test_config->idle_pll_timeout_us
        );
        if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, err)) {
            return false;
        }
    }

    return run_rx_calibration(app, test_config) &&
           log_radio_config(app, config);
}

static bool configure_external_rf(
    dw3000_ss_twr_app_t*                 app,
    const dw3000_ss_twr_device_config_t* test_config
) {
    ESP_LOGI(
        app->tag,
        "external RF extpa=%d exttxe=%d extrxe=%d",
        test_config->enable_extpa,
        test_config->enable_exttxe,
        test_config->enable_extrxe
    );

    return DW3000_SS_TWR_CHECK_DW3000(
        app->tag,
        dw3000_hal_gpio_configure_external_pa_lna(
            &app->device,
            test_config->enable_extpa,
            test_config->enable_exttxe,
            test_config->enable_extrxe
        )
    );
}

static dw3000_error_t initialize_once(
    dw3000_ss_twr_app_t*              app,
    const dw3000_device_config_t*     config,
    const dw3000_hal_bringup_t*       bringup,
    const dw3000_hal_init_options_t*  options
) {
    dw3000_device_config_t bringup_config = *config;
    dw3000_error_t         err;

    bringup_config.auto_init_pll = false;

    err = dw3000_hal_bringup(&app->device, bringup, app->port, &bringup_config);
    if (err != DW3000_ERROR_OK) {
        ESP_LOGE(app->tag, "bringup failed: %s", dw3000_error_to_string(err));
        return err;
    }

    ESP_LOGI(
        app->tag,
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
        ESP_LOGE(app->tag, "configure failed: %s", dw3000_error_to_string(err));
        return err;
    }

    ESP_LOGI(
        app->tag,
        "configure ok state_flags=0x%08" PRIX32,
        (uint32_t)app->device.state_flags
    );

    return DW3000_ERROR_OK;
}

bool dw3000_ss_twr_setup(
    dw3000_ss_twr_app_t*                app,
    const char*                         tag,
    const dw3000_ss_twr_board_config_t* board
) {
    spi_bus_config_t bus_config;
    spi_device_interface_config_t device_config;
    esp_err_t err;

    if ((app == NULL) || (tag == NULL) || (board == NULL)) {
        return false;
    }

    memset(app, 0, sizeof(*app));
    app->tag   = tag;
    app->board = *board;

    bus_config = (spi_bus_config_t){
        .mosi_io_num     = board->pin_mosi,
        .miso_io_num     = board->pin_miso,
        .sclk_io_num     = board->pin_sck,
        .quadwp_io_num   = GPIO_NUM_NC,
        .quadhd_io_num   = GPIO_NUM_NC,
        .max_transfer_sz = (int)board->spi_max_transfer_size,
    };
    device_config = (spi_device_interface_config_t){
        .clock_speed_hz = board->spi_clock_hz,
        .mode           = 0,
        .spics_io_num   = board->pin_cs,
        .queue_size     = 1,
    };

    err = spi_bus_initialize(board->spi_host, &bus_config, SPI_DMA_CH_AUTO);
    if (err == ESP_OK) {
        app->bus_initialized = true;
    } else if (err != ESP_ERR_INVALID_STATE) {
        return check_esp(tag, err, "spi_bus_initialize");
    }

    if (!check_esp(
            tag,
            spi_bus_add_device(board->spi_host, &device_config, &app->spi),
            "spi_bus_add_device"
        )) {
        return false;
    }

    app->port = dw3000_espidf_port_new();
    if (app->port == NULL) {
        ESP_LOGE(tag, "dw3000_espidf_port_new failed");
        return false;
    }

    dw3000_espidf_port_config_t port_config;
    dw3000_espidf_port_default_config(&port_config);
    port_config.spi                  = app->spi;
    port_config.reset_gpio           = board->pin_rst;
    port_config.irq_gpio             = board->pin_irq;
    port_config.configure_reset_gpio = board->configure_rst;
    port_config.configure_irq_gpio   = board->configure_irq;

    if (!DW3000_SS_TWR_CHECK_DW3000(
            tag,
            dw3000_espidf_port_configure(app->port, &port_config)
        )) {
        return false;
    }

    ESP_LOGI(
        tag,
        "SPI host=%d clock=%dHz sck=%d miso=%d mosi=%d cs=%d rst=%d irq=%d",
        board->spi_host,
        board->spi_clock_hz,
        board->pin_sck,
        board->pin_miso,
        board->pin_mosi,
        board->pin_cs,
        board->pin_rst,
        board->pin_irq
    );

    return true;
}

bool dw3000_ss_twr_teardown(dw3000_ss_twr_app_t* app) {
    bool ok = true;

    if (app == NULL) {
        return false;
    }

    if (app->port != NULL) {
        dw3000_espidf_port_delete(app->port);
        app->port = NULL;
    }

    if (app->spi != NULL) {
        ok = check_esp(
            app->tag,
            spi_bus_remove_device(app->spi),
            "spi_bus_remove_device"
        ) && ok;
        app->spi = NULL;
    }

    if (app->bus_initialized) {
        ok = check_esp(
            app->tag,
            spi_bus_free(app->board.spi_host),
            "spi_bus_free"
        ) && ok;
        app->bus_initialized = false;
    }

    return ok;
}

bool dw3000_ss_twr_initialize(
    dw3000_ss_twr_app_t*                 app,
    const dw3000_ss_twr_device_config_t* test_config
) {
    dw3000_device_config_t     config;
    dw3000_hal_bringup_t       bringup;
    dw3000_hal_init_options_t  options;
    dw3000_error_t             err = DW3000_ERROR_TIMEOUT;

    if ((app == NULL) || (test_config == NULL)) {
        return false;
    }

    dw3000_hal_default_config(&config);
    config.load_otp_calibration  = test_config->load_otp_calibration;
    config.auto_init_pll         = test_config->enter_idle_pll;
    config.use_double_buffer     = false;
    config.mac.panadr.pan_id     = test_config->pan_id;
    config.mac.panadr.short_addr = test_config->short_addr;
    apply_radio_profile(&config, test_config);

    dw3000_hal_default_bringup(&bringup);
    bringup.reset_assert_us  = test_config->reset_assert_us;
    bringup.reset_settle_us  = test_config->reset_settle_us;
    bringup.ready_timeout_us = test_config->ready_timeout_us;

    dw3000_hal_default_init_options(&options);
    options.idle_pll_timeout_us = test_config->idle_pll_timeout_us;

    for (uint32_t attempt = 1U; attempt <= test_config->init_retries; ++attempt) {
        ESP_LOGI(
            app->tag,
            "init attempt %" PRIu32 "/%" PRIu32
            " reset_assert=%" PRIu32 "us reset_settle=%" PRIu32
            "us ready_timeout=%" PRIu32 "us idle_pll_timeout=%" PRIu32 "us",
            attempt,
            test_config->init_retries,
            bringup.reset_assert_us,
            bringup.reset_settle_us,
            bringup.ready_timeout_us,
            options.idle_pll_timeout_us
        );

        err = initialize_once(app, &config, &bringup, &options);
        if (err == DW3000_ERROR_OK) {
            return configure_external_rf(app, test_config) &&
                   configure_radio_profile(app, &config, test_config);
        }

        if (attempt < test_config->init_retries) {
            vTaskDelay(pdMS_TO_TICKS(20U));
        }
    }

    ESP_LOGE(
        app->tag,
        "initialize failed after %" PRIu32 " attempts: %s",
        test_config->init_retries,
        dw3000_error_to_string(err)
    );
    return false;
}

void dw3000_ss_twr_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
) {
    if (delay_us >= 1000U) {
        TickType_t ticks = pdMS_TO_TICKS((delay_us + 999U) / 1000U);

        if (ticks == 0U) {
            ticks = 1U;
        }

        vTaskDelay(ticks);
    } else if ((delay_us != 0U) &&
               (device != NULL) &&
               (device->port.delay_us != NULL)) {
        device->port.delay_us(device->port.ctx, delay_us);
    }
}

bool dw3000_ss_twr_wait_events(
    dw3000_ss_twr_app_t* app,
    dw3000_txrx_event_t  wanted_events,
    dw3000_txrx_event_t  stop_events,
    uint32_t             timeout_us,
    uint32_t             poll_delay_us,
    dw3000_txrx_event_t* events_out
) {
    uint64_t start_us;
    dw3000_txrx_event_t events = 0U;

    if ((app == NULL) || (app->device.port.get_time_us == NULL)) {
        return false;
    }

    start_us = app->device.port.get_time_us(app->device.port.ctx);
    while ((app->device.port.get_time_us(app->device.port.ctx) - start_us) <
           timeout_us) {
        if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_status_read(
                &app->device,
                &events
            ))) {
            return false;
        }

        if ((events & wanted_events) != 0U) {
            if (events_out != NULL) {
                *events_out = events;
            }
            return true;
        }

        if ((events & stop_events) != 0U) {
            if (events_out != NULL) {
                *events_out = events;
            }
            return false;
        }

        dw3000_ss_twr_delay_us(&app->device, poll_delay_us);
    }

    if (events_out != NULL) {
        *events_out = events;
    }
    return false;
}

bool dw3000_ss_twr_arm_rx(
    dw3000_ss_twr_app_t* app,
    uint32_t             settle_us
) {
    if (app == NULL) {
        return false;
    }

    if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_fcmd_txrxoff(&app->device))) {
        return false;
    }

    dw3000_ss_twr_delay_us(&app->device, settle_us);

    return DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_status_clear_all(&app->device)) &&
           DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_rx_start_immediate(&app->device));
}

bool dw3000_ss_twr_read_rx_frame(
    dw3000_ss_twr_app_t*      app,
    uint8_t*                  frame,
    size_t                    frame_capacity,
    size_t*                   frame_len,
    dw3000_txrx_timestamp_t*  rx_timestamp,
    dw3000_txrx_rx_finfo_t*   finfo,
    dw3000_txrx_event_t       events
) {
    size_t read_len;

    if ((app == NULL) ||
        (frame == NULL) ||
        (frame_len == NULL) ||
        (rx_timestamp == NULL) ||
        (finfo == NULL)) {
        return false;
    }

    if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_rx_read_finfo(
            &app->device,
            finfo
        ))) {
        return false;
    }

    read_len = finfo->rx_flen;
    if (read_len > frame_capacity) {
        read_len = frame_capacity;
    }

    if (!DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_rx_read_active_buffer(
            &app->device,
            0U,
            frame,
            read_len
        )) ||
        !DW3000_SS_TWR_CHECK_DW3000(app->tag, dw3000_hal_rx_read_timestamp(
            &app->device,
            rx_timestamp
        ))) {
        return false;
    }

    *frame_len = read_len;

    ESP_LOGI(
        app->tag,
        "rx len=%u pacc=%u data_rate=%u prf=%u rx_ts=0x%010" PRIX64
        " SYS_STATUS=0x%012" PRIX64,
        (unsigned)finfo->rx_flen,
        (unsigned)finfo->rx_pacc,
        (unsigned)finfo->data_rate,
        (unsigned)finfo->prf,
        (uint64_t)*rx_timestamp,
        (uint64_t)events
    );

    return true;
}

size_t dw3000_ss_twr_build_poll_frame(
    uint8_t  seq,
    uint16_t pan_id,
    uint16_t src_addr,
    uint16_t dst_addr,
    uint8_t* frame
) {
    return build_common_payload(
        seq,
        DW3000_SS_TWR_MSG_POLL,
        pan_id,
        src_addr,
        dst_addr,
        frame
    );
}

size_t dw3000_ss_twr_build_response_frame(
    uint8_t                 seq,
    uint16_t                pan_id,
    uint16_t                src_addr,
    uint16_t                dst_addr,
    dw3000_txrx_timestamp_t poll_rx_ts,
    dw3000_txrx_timestamp_t resp_tx_ts,
    uint8_t*                frame
) {
    size_t offset = build_common_payload(
        seq,
        DW3000_SS_TWR_MSG_RESP,
        pan_id,
        src_addr,
        dst_addr,
        frame
    );

    write_ts40_le(&frame[offset], poll_rx_ts);
    offset += DW3000_SS_TWR_TIMESTAMP_LEN;
    write_ts40_le(&frame[offset], resp_tx_ts);
    offset += DW3000_SS_TWR_TIMESTAMP_LEN;

    return offset;
}

bool dw3000_ss_twr_parse_frame(
    const uint8_t*              frame,
    size_t                      frame_len,
    dw3000_ss_twr_frame_info_t* info
) {
    size_t offset = DW3000_SS_TWR_FRAME_HEADER_LEN;

    if ((frame == NULL) || (info == NULL)) {
        return false;
    }

    memset(info, 0, sizeof(*info));

    if (frame_len < (offset + sizeof(DW3000_SS_TWR_MAGIC) + 2U)) {
        return false;
    }

    if ((frame[0] != 0x41U) || (frame[1] != 0x88U)) {
        return false;
    }

    if (memcmp(&frame[offset], DW3000_SS_TWR_MAGIC, sizeof(DW3000_SS_TWR_MAGIC)) != 0) {
        return false;
    }

    info->seq      = frame[2];
    info->pan_id   = read_u16_le(&frame[3]);
    info->dst_addr = read_u16_le(&frame[5]);
    info->src_addr = read_u16_le(&frame[7]);

    offset += sizeof(DW3000_SS_TWR_MAGIC);
    info->type = (dw3000_ss_twr_msg_type_t)frame[offset++];
    if (frame[offset++] != info->seq) {
        return false;
    }

    if (info->type == DW3000_SS_TWR_MSG_POLL) {
        return true;
    }

    if (info->type != DW3000_SS_TWR_MSG_RESP) {
        return false;
    }

    if (frame_len < (offset + (2U * DW3000_SS_TWR_TIMESTAMP_LEN))) {
        return false;
    }

    info->poll_rx_ts = read_ts40_le(&frame[offset]);
    offset += DW3000_SS_TWR_TIMESTAMP_LEN;
    info->resp_tx_ts = read_ts40_le(&frame[offset]);
    return true;
}

dw3000_txrx_timestamp_t dw3000_ss_twr_timestamp_add_us(
    dw3000_txrx_timestamp_t timestamp,
    uint32_t                delay_us
) {
    uint64_t delta = (uint64_t)delay_us * DW3000_SS_TWR_DTU_PER_US;

    return (timestamp + delta) & DW3000_SS_TWR_TIMESTAMP_MASK;
}

dw3000_txrx_delayed_time_t dw3000_ss_twr_delayed_time_from_timestamp(
    dw3000_txrx_timestamp_t timestamp
) {
    return (dw3000_txrx_delayed_time_t)(
        ((timestamp & DW3000_SS_TWR_TIMESTAMP_MASK) >> 8U) &
        DW3000_SS_TWR_DELAYED_TIME_MASK
    );
}

dw3000_txrx_timestamp_t dw3000_ss_twr_tx_timestamp_from_delayed_time(
    dw3000_txrx_delayed_time_t delayed_time,
    dw3000_cia_antenna_delay_t tx_antenna_delay
) {
    return ((((dw3000_txrx_timestamp_t)delayed_time) << 8U) +
            (dw3000_txrx_timestamp_t)tx_antenna_delay) &
           DW3000_SS_TWR_TIMESTAMP_MASK;
}

uint64_t dw3000_ss_twr_timestamp_diff(
    dw3000_txrx_timestamp_t end,
    dw3000_txrx_timestamp_t start
) {
    return (end - start) & DW3000_SS_TWR_TIMESTAMP_MASK;
}

int64_t dw3000_ss_twr_tof_dtu(
    dw3000_txrx_timestamp_t poll_tx_ts,
    dw3000_txrx_timestamp_t poll_rx_ts,
    dw3000_txrx_timestamp_t resp_tx_ts,
    dw3000_txrx_timestamp_t resp_rx_ts
) {
    uint64_t round_dtu = dw3000_ss_twr_timestamp_diff(resp_rx_ts, poll_tx_ts);
    uint64_t reply_dtu = dw3000_ss_twr_timestamp_diff(resp_tx_ts, poll_rx_ts);

    return ((int64_t)round_dtu - (int64_t)reply_dtu) / 2;
}

int32_t dw3000_ss_twr_tof_to_centimeters(int64_t tof_dtu) {
    int64_t sign = (tof_dtu < 0) ? -1 : 1;
    uint64_t abs_dtu = (tof_dtu < 0) ? (uint64_t)(-tof_dtu) : (uint64_t)tof_dtu;
    uint64_t cm = (abs_dtu * UINT64_C(100000)) / DW3000_SS_TWR_DTU_PER_METER_X1000;

    if (cm > INT32_MAX) {
        cm = INT32_MAX;
    }

    return (int32_t)((int64_t)cm * sign);
}
