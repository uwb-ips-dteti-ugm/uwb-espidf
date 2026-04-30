#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/rx.h"
#include "dw3000_hal/status.h"
#include "dw3000_hal/tx.h"
#include "esp_log.h"
#include "hal_ss_twr_common.h"

static const char* TAG = "dw3000_hal_ss_twr_i";

static const dw3000_ss_twr_board_config_t BOARD_CONFIG = {
    .spi_host              = DW3000_HAL_SS_TWR_INITIATOR_SPI_HOST,
    .spi_clock_hz          = DW3000_HAL_SS_TWR_INITIATOR_SPI_CLOCK_HZ,
    .pin_sck               = DW3000_HAL_SS_TWR_INITIATOR_PIN_SCK,
    .pin_miso              = DW3000_HAL_SS_TWR_INITIATOR_PIN_MISO,
    .pin_mosi              = DW3000_HAL_SS_TWR_INITIATOR_PIN_MOSI,
    .pin_cs                = DW3000_HAL_SS_TWR_INITIATOR_PIN_CS,
    .pin_rst               = DW3000_HAL_SS_TWR_INITIATOR_PIN_RST,
    .pin_irq               = DW3000_HAL_SS_TWR_INITIATOR_PIN_IRQ,
    .configure_rst         = DW3000_HAL_SS_TWR_INITIATOR_CONFIGURE_RST,
    .configure_irq         = DW3000_HAL_SS_TWR_INITIATOR_CONFIGURE_IRQ,
    .spi_max_transfer_size = DW3000_HAL_SS_TWR_INITIATOR_SPI_MAX_TRANSFER_SIZE,
};

static const dw3000_ss_twr_device_config_t DEVICE_CONFIG = {
    .load_otp_calibration = DW3000_HAL_SS_TWR_INITIATOR_LOAD_OTP_CALIBRATION,
    .enter_idle_pll       = DW3000_HAL_SS_TWR_INITIATOR_ENTER_IDLE_PLL,
    .enable_extpa         = DW3000_HAL_SS_TWR_INITIATOR_ENABLE_EXTPA,
    .enable_exttxe        = DW3000_HAL_SS_TWR_INITIATOR_ENABLE_EXTTXE,
    .enable_extrxe        = DW3000_HAL_SS_TWR_INITIATOR_ENABLE_EXTRXE,
    .run_rx_calibration   = DW3000_HAL_SS_TWR_INITIATOR_RUN_RX_CALIBRATION,
    .rx_cal_timeout_us    = DW3000_HAL_SS_TWR_INITIATOR_RX_CAL_TIMEOUT_US,
    .reset_assert_us      = DW3000_HAL_SS_TWR_INITIATOR_RESET_ASSERT_US,
    .reset_settle_us      = DW3000_HAL_SS_TWR_INITIATOR_RESET_SETTLE_US,
    .ready_timeout_us     = DW3000_HAL_SS_TWR_INITIATOR_READY_TIMEOUT_US,
    .idle_pll_timeout_us  = DW3000_HAL_SS_TWR_INITIATOR_IDLE_PLL_TIMEOUT_US,
    .init_retries         = DW3000_HAL_SS_TWR_INITIATOR_INIT_RETRIES,
    .dtune3               = DW3000_HAL_SS_TWR_INITIATOR_DTUNE3,
    .tx_power_byte        = DW3000_HAL_SS_TWR_INITIATOR_TX_POWER_BYTE,
    .tx_antenna_delay     = DW3000_HAL_SS_TWR_INITIATOR_TX_ANTENNA_DELAY,
    .rx_antenna_delay     = DW3000_HAL_SS_TWR_INITIATOR_RX_ANTENNA_DELAY,
    .pan_id               = DW3000_HAL_SS_TWR_INITIATOR_PAN_ID,
    .short_addr           = DW3000_HAL_SS_TWR_INITIATOR_SHORT_ADDR,
};

static bool send_poll(
    dw3000_ss_twr_app_t*      app,
    uint8_t                   seq,
    dw3000_txrx_timestamp_t*  poll_tx_ts
) {
    uint8_t frame_data[DW3000_SS_TWR_FRAME_MAX_LEN];
    size_t  frame_len = dw3000_ss_twr_build_poll_frame(
        seq,
        DW3000_HAL_SS_TWR_INITIATOR_PAN_ID,
        DW3000_HAL_SS_TWR_INITIATOR_SHORT_ADDR,
        DW3000_HAL_SS_TWR_INITIATOR_RESPONDER_ADDR,
        frame_data
    );
    dw3000_txrx_tx_frame_t frame = {
        .tx_flen     = (uint16_t)(frame_len + DW3000_SS_TWR_AUTO_FCS_LEN),
        .ranging     = true,
        .tx_b_offset = 0U,
        .fine_plen   = 0U,
    };
    dw3000_txrx_event_t events = 0U;
    dw3000_txrx_delayed_time_t poll_tx_rawst = 0U;

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_fcmd_txrxoff(&app->device))) {
        return false;
    }

    dw3000_ss_twr_delay_us(&app->device, DW3000_HAL_SS_TWR_INITIATOR_RADIO_SETTLE_US);

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_clear_all(&app->device)) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_tx_prepare_frame(
            &app->device,
            frame_data,
            frame_len,
            &frame
        )) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_tx_start_immediate(&app->device))) {
        return false;
    }

    if (!dw3000_ss_twr_wait_events(
            app,
            DW3000_TXRX_EVENT_TXFRS,
            DW3000_TXRX_EVENT_CMD_ERR | DW3000_TXRX_EVENT_HPDWARN,
            DW3000_HAL_SS_TWR_INITIATOR_TX_TIMEOUT_US,
            DW3000_HAL_SS_TWR_INITIATOR_POLL_DELAY_US,
            &events
        )) {
        ESP_LOGE(TAG, "poll TX failed SYS_STATUS=0x%012" PRIX64, (uint64_t)events);
        return false;
    }

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_tx_read_raw_timestamp(
            &app->device,
            &poll_tx_rawst
        ))) {
        return false;
    }
    *poll_tx_ts = dw3000_ss_twr_tx_timestamp_from_delayed_time(
        poll_tx_rawst,
        DW3000_HAL_SS_TWR_INITIATOR_TX_ANTENNA_DELAY
    );

    ESP_LOGI(
        TAG,
        "poll tx seq=%u len=%u tx_rawst=0x%08" PRIX32 " tx_ts=0x%010" PRIX64
        " SYS_STATUS=0x%012" PRIX64,
        (unsigned)seq,
        (unsigned)frame_len,
        poll_tx_rawst,
        (uint64_t)*poll_tx_ts,
        (uint64_t)events
    );

    return true;
}

static bool receive_response(
    dw3000_ss_twr_app_t*     app,
    uint8_t                  seq,
    dw3000_txrx_timestamp_t  poll_tx_ts
) {
    uint8_t frame[DW3000_SS_TWR_FRAME_MAX_LEN];
    size_t frame_len = 0U;
    dw3000_txrx_event_t events = 0U;
    dw3000_txrx_rx_finfo_t finfo;
    dw3000_txrx_timestamp_t resp_rx_ts = 0U;
    dw3000_ss_twr_frame_info_t info;
    int64_t tof_dtu;
    int32_t distance_cm;

    if (!dw3000_ss_twr_arm_rx(
            app,
            DW3000_HAL_SS_TWR_INITIATOR_RADIO_SETTLE_US
        )) {
        return false;
    }

    if (!dw3000_ss_twr_wait_events(
            app,
            DW3000_TXRX_EVENT_RXFCG,
            dw3000_hal_rx_error_events() | DW3000_TXRX_EVENT_CMD_ERR,
            DW3000_HAL_SS_TWR_INITIATOR_RX_TIMEOUT_US,
            DW3000_HAL_SS_TWR_INITIATOR_POLL_DELAY_US,
            &events
        )) {
        ESP_LOGE(TAG, "response RX failed SYS_STATUS=0x%012" PRIX64, (uint64_t)events);
        return false;
    }

    if (!dw3000_ss_twr_read_rx_frame(
            app,
            frame,
            sizeof(frame),
            &frame_len,
            &resp_rx_ts,
            &finfo,
            events
        )) {
        return false;
    }

    if (!dw3000_ss_twr_parse_frame(frame, frame_len, &info) ||
        (info.type != DW3000_SS_TWR_MSG_RESP) ||
        (info.seq != seq) ||
        (info.pan_id != DW3000_HAL_SS_TWR_INITIATOR_PAN_ID) ||
        (info.dst_addr != DW3000_HAL_SS_TWR_INITIATOR_SHORT_ADDR) ||
        (info.src_addr != DW3000_HAL_SS_TWR_INITIATOR_RESPONDER_ADDR)) {
        ESP_LOGW(TAG, "unexpected response frame seq=%u len=%u", (unsigned)seq, (unsigned)frame_len);
        return false;
    }

    tof_dtu = dw3000_ss_twr_tof_dtu(
        poll_tx_ts,
        info.poll_rx_ts,
        info.resp_tx_ts,
        resp_rx_ts
    );
    distance_cm = dw3000_ss_twr_tof_to_centimeters(tof_dtu);

    ESP_LOGI(
        TAG,
        "twr seq=%u poll_tx=0x%010" PRIX64 " poll_rx=0x%010" PRIX64
        " resp_tx=0x%010" PRIX64 " resp_rx=0x%010" PRIX64
        " tof=%" PRId64 "dtu distance=%" PRId32 "cm",
        (unsigned)seq,
        (uint64_t)poll_tx_ts,
        (uint64_t)info.poll_rx_ts,
        (uint64_t)info.resp_tx_ts,
        (uint64_t)resp_rx_ts,
        tof_dtu,
        distance_cm
    );

    if ((distance_cm < -DW3000_HAL_SS_TWR_INITIATOR_MAX_ABS_DISTANCE_CM) ||
        (distance_cm > DW3000_HAL_SS_TWR_INITIATOR_MAX_ABS_DISTANCE_CM)) {
        ESP_LOGW(TAG, "distance estimate outside smoke-test bound");
        ESP_LOG_BUFFER_HEX(TAG, frame, frame_len);
    }

    return true;
}

static bool run_test(dw3000_ss_twr_app_t* app) {
    dw3000_txrx_event_t saved_enabled;
    dw3000_txrx_event_t test_enabled =
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_HPDWARN |
        DW3000_TXRX_EVENT_CMD_ERR |
        dw3000_hal_rx_all_events();
    uint32_t successes = 0U;

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_read_enabled(
            &app->device,
            &saved_enabled
        )) ||
        !DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_set_enabled(
            &app->device,
            test_enabled
        ))) {
        return false;
    }

    ESP_LOGI(TAG, "saved SYS_ENABLE=0x%012" PRIX64, (uint64_t)saved_enabled);
    ESP_LOGI(TAG, "test SYS_ENABLE=0x%012" PRIX64, (uint64_t)test_enabled);

    for (uint8_t seq = 0U;
         seq < DW3000_HAL_SS_TWR_INITIATOR_EXCHANGE_COUNT;
         ++seq) {
        dw3000_txrx_timestamp_t poll_tx_ts = 0U;

        if (send_poll(app, seq, &poll_tx_ts) &&
            receive_response(app, seq, poll_tx_ts)) {
            successes++;
        }

        (void)dw3000_hal_fcmd_txrxoff(&app->device);
        (void)dw3000_hal_status_clear_all(&app->device);

        if ((seq + 1U) < DW3000_HAL_SS_TWR_INITIATOR_EXCHANGE_COUNT) {
            dw3000_ss_twr_delay_us(
                &app->device,
                DW3000_HAL_SS_TWR_INITIATOR_EXCHANGE_INTERVAL_US
            );
        }
    }

    (void)dw3000_hal_fcmd_txrxoff(&app->device);
    (void)dw3000_hal_status_clear_all(&app->device);

    if (!DW3000_SS_TWR_CHECK_DW3000(TAG, dw3000_hal_status_set_enabled(
            &app->device,
            saved_enabled
        ))) {
        return false;
    }

    ESP_LOGI(
        TAG,
        "successes=%" PRIu32 "/%" PRIu32,
        successes,
        (uint32_t)DW3000_HAL_SS_TWR_INITIATOR_EXCHANGE_COUNT
    );

    return successes >= DW3000_HAL_SS_TWR_INITIATOR_REQUIRED_SUCCESSES;
}

void app_main(void) {
    dw3000_ss_twr_app_t app;
    bool ok;

    ok = dw3000_ss_twr_setup(&app, TAG, &BOARD_CONFIG) &&
         dw3000_ss_twr_initialize(&app, &DEVICE_CONFIG) &&
         run_test(&app);

    ok = dw3000_ss_twr_teardown(&app) && ok;

    if (ok) {
        ESP_LOGI(TAG, "DW3000_HAL_SS_TWR_INITIATOR_PASS");
    } else {
        ESP_LOGE(TAG, "DW3000_HAL_SS_TWR_INITIATOR_FAIL");
    }
}
