#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/rx.h"
#include "dw3000_hal/status.h"
#include "dw3000_hal/tx.h"
#include "dw3000_register.h"
#include "esp_log.h"
#include "hal_ss_twr_common.h"

static const char* TAG = "dw3000_hal_ss_twr_r";

static const dw3000_ss_twr_board_config_t BOARD_CONFIG = {
    .spi_host              = DW3000_HAL_SS_TWR_RESPONDER_SPI_HOST,
    .spi_clock_hz          = DW3000_HAL_SS_TWR_RESPONDER_SPI_CLOCK_HZ,
    .pin_sck               = DW3000_HAL_SS_TWR_RESPONDER_PIN_SCK,
    .pin_miso              = DW3000_HAL_SS_TWR_RESPONDER_PIN_MISO,
    .pin_mosi              = DW3000_HAL_SS_TWR_RESPONDER_PIN_MOSI,
    .pin_cs                = DW3000_HAL_SS_TWR_RESPONDER_PIN_CS,
    .pin_rst               = DW3000_HAL_SS_TWR_RESPONDER_PIN_RST,
    .pin_irq               = DW3000_HAL_SS_TWR_RESPONDER_PIN_IRQ,
    .configure_rst         = DW3000_HAL_SS_TWR_RESPONDER_CONFIGURE_RST,
    .configure_irq         = DW3000_HAL_SS_TWR_RESPONDER_CONFIGURE_IRQ,
    .spi_max_transfer_size = DW3000_HAL_SS_TWR_RESPONDER_SPI_MAX_TRANSFER_SIZE,
};

static const dw3000_ss_twr_device_config_t DEVICE_CONFIG = {
    .load_otp_calibration = DW3000_HAL_SS_TWR_RESPONDER_LOAD_OTP_CALIBRATION,
    .enter_idle_pll       = DW3000_HAL_SS_TWR_RESPONDER_ENTER_IDLE_PLL,
    .enable_extpa         = DW3000_HAL_SS_TWR_RESPONDER_ENABLE_EXTPA,
    .enable_exttxe        = DW3000_HAL_SS_TWR_RESPONDER_ENABLE_EXTTXE,
    .enable_extrxe        = DW3000_HAL_SS_TWR_RESPONDER_ENABLE_EXTRXE,
    .run_rx_calibration   = DW3000_HAL_SS_TWR_RESPONDER_RUN_RX_CALIBRATION,
    .rx_cal_timeout_us    = DW3000_HAL_SS_TWR_RESPONDER_RX_CAL_TIMEOUT_US,
    .reset_assert_us      = DW3000_HAL_SS_TWR_RESPONDER_RESET_ASSERT_US,
    .reset_settle_us      = DW3000_HAL_SS_TWR_RESPONDER_RESET_SETTLE_US,
    .ready_timeout_us     = DW3000_HAL_SS_TWR_RESPONDER_READY_TIMEOUT_US,
    .idle_pll_timeout_us  = DW3000_HAL_SS_TWR_RESPONDER_IDLE_PLL_TIMEOUT_US,
    .init_retries         = DW3000_HAL_SS_TWR_RESPONDER_INIT_RETRIES,
    .dtune3               = DW3000_HAL_SS_TWR_RESPONDER_DTUNE3,
    .tx_power_byte        = DW3000_HAL_SS_TWR_RESPONDER_TX_POWER_BYTE,
    .tx_antenna_delay     = DW3000_HAL_SS_TWR_RESPONDER_TX_ANTENNA_DELAY,
    .rx_antenna_delay     = DW3000_HAL_SS_TWR_RESPONDER_RX_ANTENNA_DELAY,
    .pan_id               = DW3000_HAL_SS_TWR_RESPONDER_PAN_ID,
    .short_addr           = DW3000_HAL_SS_TWR_RESPONDER_SHORT_ADDR,
};

static bool read_poll(
    dw3000_ss_twr_app_t*        app,
    dw3000_ss_twr_frame_info_t* info,
    dw3000_txrx_timestamp_t*    poll_rx_ts
) {
    uint8_t                frame[DW3000_SS_TWR_FRAME_MAX_LEN];
    size_t                 frame_len = 0U;
    dw3000_txrx_event_t    events    = 0U;
    dw3000_txrx_rx_finfo_t finfo;

    if (!dw3000_ss_twr_arm_rx(
            app,
            DW3000_HAL_SS_TWR_RESPONDER_RADIO_SETTLE_US
        )) {
        return false;
    }

    if (!dw3000_ss_twr_wait_events(
            app,
            DW3000_TXRX_EVENT_RXFCG,
            dw3000_hal_rx_error_events() | DW3000_TXRX_EVENT_CMD_ERR,
            DW3000_HAL_SS_TWR_RESPONDER_WAIT_TIMEOUT_US,
            DW3000_HAL_SS_TWR_RESPONDER_POLL_DELAY_US,
            &events
        )) {
        ESP_LOGE(TAG, "poll RX failed SYS_STATUS=0x%012" PRIX64, (uint64_t)events);
        return false;
    }

    if (!dw3000_ss_twr_read_rx_frame(
            app,
            frame,
            sizeof(frame),
            &frame_len,
            poll_rx_ts,
            &finfo,
            events
        )) {
        return false;
    }

    if (!dw3000_ss_twr_parse_frame(frame, frame_len, info) ||
        (info->type != DW3000_SS_TWR_MSG_POLL) ||
        (info->pan_id != DW3000_HAL_SS_TWR_RESPONDER_PAN_ID) ||
        (info->dst_addr != DW3000_HAL_SS_TWR_RESPONDER_SHORT_ADDR) ||
        (info->src_addr != DW3000_HAL_SS_TWR_RESPONDER_INITIATOR_ADDR)) {
        ESP_LOGW(TAG, "unexpected poll frame len=%u", (unsigned)frame_len);
        return false;
    }

    return true;
}

static bool send_response(
    dw3000_ss_twr_app_t*              app,
    const dw3000_ss_twr_frame_info_t* poll_info,
    dw3000_txrx_timestamp_t           poll_rx_ts
) {
    uint8_t                   frame_data[DW3000_SS_TWR_FRAME_MAX_LEN];
    uint32_t                  sys_time_hi32;
    uint32_t                  reply_delay_dtu_hi32;
    dw3000_txrx_delayed_time_t delayed_time;
    dw3000_txrx_timestamp_t   scheduled_resp_tx_ts;
    size_t                    frame_len;
    dw3000_txrx_tx_frame_t    frame;
    dw3000_txrx_event_t       events       = 0U;
    dw3000_txrx_timestamp_t   actual_tx_ts = 0U;

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_fcmd_txrxoff(&app->device))) {
        return false;
    }

    dw3000_ss_twr_delay_us(&app->device, DW3000_HAL_SS_TWR_RESPONDER_RADIO_SETTLE_US);

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_clear_all(&app->device)) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_reg_read_u32(
            &app->device,
            DW3000_REG_SYS_TIME,
            &sys_time_hi32
        ))) {
        return false;
    }

    reply_delay_dtu_hi32 = (uint32_t)(
        ((uint64_t)DW3000_HAL_SS_TWR_RESPONDER_REPLY_DELAY_US *
         DW3000_SS_TWR_DTU_PER_US) >> 8U
    );
    delayed_time = (dw3000_txrx_delayed_time_t)(
        (sys_time_hi32 + reply_delay_dtu_hi32) &
        DW3000_SS_TWR_DELAYED_TIME_MASK
    );
    scheduled_resp_tx_ts = dw3000_ss_twr_tx_timestamp_from_delayed_time(
        delayed_time,
        DW3000_HAL_SS_TWR_RESPONDER_TX_ANTENNA_DELAY
    );
    frame_len = dw3000_ss_twr_build_response_frame(
        poll_info->seq,
        DW3000_HAL_SS_TWR_RESPONDER_PAN_ID,
        DW3000_HAL_SS_TWR_RESPONDER_SHORT_ADDR,
        DW3000_HAL_SS_TWR_RESPONDER_INITIATOR_ADDR,
        poll_rx_ts,
        scheduled_resp_tx_ts,
        frame_data
    );
    frame = (dw3000_txrx_tx_frame_t){
        .tx_flen     = (uint16_t)(frame_len + DW3000_SS_TWR_AUTO_FCS_LEN),
        .ranging     = true,
        .tx_b_offset = 0U,
        .fine_plen   = 0U,
    };

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_tx_set_delayed_time(&app->device, delayed_time)) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_tx_prepare_frame(&app->device, frame_data, frame_len, &frame)) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_tx_start_delayed(&app->device))) {
        return false;
    }

    if (!dw3000_ss_twr_wait_events(
            app,
            DW3000_TXRX_EVENT_TXFRS,
            DW3000_TXRX_EVENT_CMD_ERR | DW3000_TXRX_EVENT_HPDWARN,
            DW3000_HAL_SS_TWR_RESPONDER_TX_TIMEOUT_US,
            DW3000_HAL_SS_TWR_RESPONDER_POLL_DELAY_US,
            &events
        )) {
        ESP_LOGE(
            TAG,
            "response TX failed SYS_STATUS=0x%012" PRIX64
            " sys_time=0x%08" PRIX32 " delayed=0x%08" PRIX32
            " scheduled_tx=0x%010" PRIX64,
            (uint64_t)events,
            sys_time_hi32,
            delayed_time,
            (uint64_t)scheduled_resp_tx_ts
        );
        return false;
    }

    (void)dw3000_hal_tx_read_timestamp(&app->device, &actual_tx_ts);

    ESP_LOGI(
        TAG,
        "resp tx seq=%u poll_rx=0x%010" PRIX64
        " sys_time=0x%08" PRIX32 " delayed=0x%08" PRIX32
        " scheduled_tx=0x%010" PRIX64
        " actual_tx=0x%010" PRIX64 " SYS_STATUS=0x%012" PRIX64,
        (unsigned)poll_info->seq,
        (uint64_t)poll_rx_ts,
        sys_time_hi32,
        delayed_time,
        (uint64_t)scheduled_resp_tx_ts,
        (uint64_t)actual_tx_ts,
        (uint64_t)events
    );

    return true;
}

static bool run_test(dw3000_ss_twr_app_t* app) {
    dw3000_txrx_event_t saved_enabled;
    dw3000_txrx_event_t test_enabled =
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_HPDWARN |
        DW3000_TXRX_EVENT_CMD_ERR |
        dw3000_hal_rx_all_events();
    uint32_t responses = 0U;

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_read_enabled(&app->device, &saved_enabled)) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_set_enabled(&app->device, test_enabled))) {
        return false;
    }

    ESP_LOGI(TAG, "saved SYS_ENABLE=0x%012" PRIX64, (uint64_t)saved_enabled);
    ESP_LOGI(TAG, "test SYS_ENABLE=0x%012" PRIX64, (uint64_t)test_enabled);

    while (responses < DW3000_HAL_SS_TWR_RESPONDER_RESPONSE_COUNT) {
        dw3000_ss_twr_frame_info_t poll_info;
        dw3000_txrx_timestamp_t    poll_rx_ts = 0U;

        if (read_poll(app, &poll_info, &poll_rx_ts) &&
            send_response(app, &poll_info, poll_rx_ts)) {
            responses++;
            ESP_LOGI(
                TAG,
                "responses=%" PRIu32 "/%" PRIu32,
                responses,
                (uint32_t)DW3000_HAL_SS_TWR_RESPONDER_RESPONSE_COUNT
            );
        }

        (void)dw3000_hal_fcmd_txrxoff(&app->device);
        (void)dw3000_hal_status_clear_all(&app->device);
    }

    (void)dw3000_hal_fcmd_txrxoff(&app->device);
    (void)dw3000_hal_status_clear_all(&app->device);

    return DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_set_enabled(&app->device, saved_enabled));
}

void app_main(void) {
    dw3000_ss_twr_app_t app;
    bool                ok;

    ok = dw3000_ss_twr_setup(&app, TAG, &BOARD_CONFIG) &&
         dw3000_ss_twr_initialize(&app, &DEVICE_CONFIG) &&
         run_test(&app);

    ok = dw3000_ss_twr_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_SS_TWR_RESPONDER_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_SS_TWR_RESPONDER_FAIL");
    }
}
