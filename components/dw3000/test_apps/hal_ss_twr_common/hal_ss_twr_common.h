#ifndef DW3000_HAL_SS_TWR_COMMON_H
#define DW3000_HAL_SS_TWR_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/spi_master.h"
#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_espidf.h"
#include "dw3000_hal/rx.h"
#include "dw3000_types/cia.h"
#include "dw3000_types/txrx.h"
#include "hal/spi_types.h"
#include "soc/gpio_num.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_SS_TWR_FRAME_MAX_LEN       64U
#define DW3000_SS_TWR_FRAME_HEADER_LEN    9U
#define DW3000_SS_TWR_AUTO_FCS_LEN        2U
#define DW3000_SS_TWR_TIMESTAMP_LEN       4U
#define DW3000_SS_TWR_TIMESTAMP_MASK      UINT64_C(0xFFFFFFFFFF)
#define DW3000_SS_TWR_EXCHANGE_TIME_MASK  UINT64_C(0xFFFFFFFF)
#define DW3000_SS_TWR_DELAYED_TIME_MASK   UINT32_C(0xFFFFFFFE)
#define DW3000_SS_TWR_DTU_PER_US          UINT64_C(63898)
#define DW3000_SS_TWR_DTU_PER_METER_X1000 UINT64_C(213139)

typedef enum {
    DW3000_SS_TWR_MSG_POLL = 0xE0U,
    DW3000_SS_TWR_MSG_RESP = 0xE1U,
} dw3000_ss_twr_msg_type_t;

typedef struct {
    spi_host_device_t spi_host;
    int               spi_clock_hz;
    gpio_num_t        pin_sck;
    gpio_num_t        pin_miso;
    gpio_num_t        pin_mosi;
    gpio_num_t        pin_cs;
    gpio_num_t        pin_rst;
    gpio_num_t        pin_irq;
    bool              configure_rst;
    bool              configure_irq;
    size_t            spi_max_transfer_size;
} dw3000_ss_twr_board_config_t;

typedef struct {
    bool     load_otp_calibration;
    bool     enter_idle_pll;
    bool     enable_extpa;
    bool     enable_exttxe;
    bool     enable_extrxe;
    bool     run_rx_calibration;
    uint32_t rx_cal_timeout_us;
    uint32_t reset_assert_us;
    uint32_t reset_settle_us;
    uint32_t ready_timeout_us;
    uint32_t idle_pll_timeout_us;
    uint32_t init_retries;
    uint32_t dtune3;
    uint8_t tx_power_byte;
    dw3000_cia_antenna_delay_t tx_antenna_delay;
    dw3000_cia_antenna_delay_t rx_antenna_delay;
    uint16_t pan_id;
    uint16_t short_addr;
} dw3000_ss_twr_device_config_t;

typedef struct {
    const char*         tag;
    spi_device_handle_t spi;
    dw3000_port_t*      port;
    dw3000_device_t     device;
    bool                bus_initialized;
    dw3000_ss_twr_board_config_t board;
} dw3000_ss_twr_app_t;

typedef struct {
    uint8_t                seq;
    dw3000_ss_twr_msg_type_t type;
    uint16_t               pan_id;
    uint16_t               dst_addr;
    uint16_t               src_addr;
    dw3000_txrx_timestamp_t poll_rx_ts;
    dw3000_txrx_timestamp_t resp_tx_ts;
} dw3000_ss_twr_frame_info_t;

bool dw3000_ss_twr_setup(
    dw3000_ss_twr_app_t*                app,
    const char*                         tag,
    const dw3000_ss_twr_board_config_t* board
);

bool dw3000_ss_twr_teardown(dw3000_ss_twr_app_t* app);

bool dw3000_ss_twr_initialize(
    dw3000_ss_twr_app_t*                   app,
    const dw3000_ss_twr_device_config_t*   test_config
);

void dw3000_ss_twr_delay_us(
    dw3000_device_t* device,
    uint32_t         delay_us
);

bool dw3000_ss_twr_wait_events(
    dw3000_ss_twr_app_t* app,
    dw3000_txrx_event_t  wanted_events,
    dw3000_txrx_event_t  stop_events,
    uint32_t             timeout_us,
    uint32_t             poll_delay_us,
    dw3000_txrx_event_t* events_out
);

bool dw3000_ss_twr_arm_rx(
    dw3000_ss_twr_app_t* app,
    uint32_t             settle_us
);

bool dw3000_ss_twr_read_rx_frame(
    dw3000_ss_twr_app_t*      app,
    uint8_t*                  frame,
    size_t                    frame_capacity,
    size_t*                   frame_len,
    dw3000_txrx_timestamp_t*  rx_timestamp,
    dw3000_txrx_rx_finfo_t*   finfo,
    dw3000_txrx_event_t       events
);

bool dw3000_ss_twr_prepare_tx_frame(
    dw3000_ss_twr_app_t*             app,
    const uint8_t*                   frame_data,
    size_t                           frame_len,
    const dw3000_txrx_tx_frame_t*    frame
);

size_t dw3000_ss_twr_build_poll_frame(
    uint8_t  seq,
    uint16_t pan_id,
    uint16_t src_addr,
    uint16_t dst_addr,
    uint8_t* frame
);

size_t dw3000_ss_twr_build_response_frame(
    uint8_t                 seq,
    uint16_t                pan_id,
    uint16_t                src_addr,
    uint16_t                dst_addr,
    dw3000_txrx_timestamp_t poll_rx_ts,
    dw3000_txrx_timestamp_t resp_tx_ts,
    uint8_t*                frame
);

bool dw3000_ss_twr_parse_frame(
    const uint8_t*             frame,
    size_t                     frame_len,
    dw3000_ss_twr_frame_info_t* info
);

dw3000_txrx_timestamp_t dw3000_ss_twr_timestamp_add_us(
    dw3000_txrx_timestamp_t timestamp,
    uint32_t                delay_us
);

dw3000_txrx_delayed_time_t dw3000_ss_twr_delayed_time_from_timestamp(
    dw3000_txrx_timestamp_t timestamp
);

dw3000_txrx_timestamp_t dw3000_ss_twr_tx_timestamp_from_delayed_time(
    dw3000_txrx_delayed_time_t delayed_time,
    dw3000_cia_antenna_delay_t tx_antenna_delay
);

uint64_t dw3000_ss_twr_timestamp_diff(
    dw3000_txrx_timestamp_t end,
    dw3000_txrx_timestamp_t start
);

int64_t dw3000_ss_twr_tof_dtu(
    dw3000_txrx_timestamp_t poll_tx_ts,
    dw3000_txrx_timestamp_t poll_rx_ts,
    dw3000_txrx_timestamp_t resp_tx_ts,
    dw3000_txrx_timestamp_t resp_rx_ts
);

int32_t dw3000_ss_twr_tof_to_centimeters(int64_t tof_dtu);

bool dw3000_ss_twr_check_dw3000(
    const char*     tag,
    dw3000_error_t  err,
    const char*     expression
);

#define DW3000_SS_TWR_CHECK_DW3000(tag_, expr_) \
    dw3000_ss_twr_check_dw3000((tag_), (expr_), #expr_)

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_SS_TWR_COMMON_H */
