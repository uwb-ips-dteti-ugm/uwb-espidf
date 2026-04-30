#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/tx.h"
#include "dw3000_types/device.h"
#include "dw3000_types/fcmd.h"
#include "dw3000_types/txrx.h"
#include "mock_dw3000_port.h"
#include "unity.h"

#define DW3000_TEST_TX_FCTRL_FRAME_MASK 0x03FF0BFFUL

static uint32_t read_le32(const uint8_t raw[4]) {
    return (uint32_t)raw[0] |
           ((uint32_t)raw[1] << 8U) |
           ((uint32_t)raw[2] << 16U) |
           ((uint32_t)raw[3] << 24U);
}

static uint16_t read_le16(const uint8_t raw[2]) {
    return (uint16_t)raw[0] | ((uint16_t)raw[1] << 8U);
}

static void define_tx_registers(dw3000_mock_port_t* mock) {
    const uint8_t tx_fctrl[4] = {0xEFU, 0xF7U, 0xFFU, 0xA5U};
    const uint8_t tx_fctrl_hi[2] = {0x5AU, 0x00U};
    const uint8_t tx_time[5] = {0x55U, 0x44U, 0x33U, 0x22U, 0x11U};

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_define_reg(
            mock,
            DW3000_REG_TX_BUFFER,
            NULL,
            32U,
            false
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_define_reg(
            mock,
            DW3000_REG_TX_FCTRL,
            tx_fctrl,
            sizeof(tx_fctrl),
            false
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_define_reg(
            mock,
            DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_0, 0x0028U, 2U),
            tx_fctrl_hi,
            sizeof(tx_fctrl_hi),
            false
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_define_reg(
            mock,
            DW3000_REG_TX_TIME,
            tx_time,
            sizeof(tx_time),
            false
        )
    );
}

static void test_tx_prepare_frame_packs_buffer_and_frame_control(void) {
    dw3000_mock_port_t mock;
    dw3000_device_t    device;
    const uint8_t      payload[] = {
        0x41U, 0x42U, 0x43U, 0x44U, 0x45U,
    };
    const dw3000_txrx_tx_frame_t frame = {
        .tx_flen     = 23U,
        .ranging     = true,
        .tx_b_offset = 0U,
        .fine_plen   = 0x12U,
    };
    uint8_t buffer[sizeof(payload)] = {0};
    uint8_t tx_fctrl_raw[4] = {0};
    uint8_t tx_fctrl_hi_raw[2] = {0};
    const uint32_t initial_tx_fctrl = 0xA5FFF7EFUL;
    const uint32_t expected_frame_bits = 23UL | (1UL << 11U);
    const uint32_t expected_tx_fctrl =
        (initial_tx_fctrl & ~DW3000_TEST_TX_FCTRL_FRAME_MASK) |
        expected_frame_bits;

    dw3000_mock_port_init(&mock, &device);
    dw3000_hal_default_config(&device.config);
    define_tx_registers(&mock);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_prepare_frame(&device, payload, sizeof(payload), &frame)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_get_reg_data(
            &mock,
            DW3000_REG_TX_BUFFER,
            buffer,
            sizeof(buffer)
        )
    );
    TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, buffer, sizeof(payload));

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_get_reg_data(
            &mock,
            DW3000_REG_TX_FCTRL,
            tx_fctrl_raw,
            sizeof(tx_fctrl_raw)
        )
    );
    TEST_ASSERT_EQUAL_HEX32(expected_tx_fctrl, read_le32(tx_fctrl_raw));

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_get_reg_data(
            &mock,
            DW3000_REG_DESC(DW3000_REG_FILE_GENERAL_CFG_0, 0x0028U, 2U),
            tx_fctrl_hi_raw,
            sizeof(tx_fctrl_hi_raw)
        )
    );
    TEST_ASSERT_EQUAL_HEX16(0x125AU, read_le16(tx_fctrl_hi_raw));
    TEST_ASSERT_EQUAL_size_t(2U, mock.read_count);
    TEST_ASSERT_EQUAL_size_t(3U, mock.write_count);
    TEST_ASSERT_EQUAL_UINT32(0U, device.state_flags & DW3000_DEVICE_STATE_TX_PENDING);
}

static void test_tx_fast_commands_timestamp_and_error_paths(void) {
    dw3000_mock_port_t mock;
    dw3000_device_t    device;
    dw3000_txrx_timestamp_t timestamp = 0U;

    dw3000_mock_port_init(&mock, &device);
    dw3000_hal_default_config(&device.config);
    define_tx_registers(&mock);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_read_timestamp(&device, &timestamp)
    );
    TEST_ASSERT_EQUAL_HEX64(UINT64_C(0x1122334455), timestamp);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_start_immediate(&device)
    );
    TEST_ASSERT_EQUAL_size_t(1U, mock.fast_command_count);
    TEST_ASSERT_EQUAL_UINT8(DW3000_FCMD_TX, mock.last_fast_command);
    TEST_ASSERT_EQUAL_UINT8(
        dw3000_hal_fcmd_header(DW3000_FCMD_TX),
        mock.last_fast_command_header
    );
    TEST_ASSERT_TRUE((device.state_flags & DW3000_DEVICE_STATE_TX_PENDING) != 0U);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_tx_start_delayed(&device)
    );
    TEST_ASSERT_EQUAL_size_t(1U, mock.fast_command_count);

    device.state_flags = DW3000_DEVICE_STATE_IDLE_PLL;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_start(&device, DW3000_FCMD_RX)
    );
    TEST_ASSERT_EQUAL_size_t(1U, mock.fast_command_count);

    dw3000_mock_port_set_next_write_error(&mock, DW3000_ERROR_IO);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_IO,
        dw3000_hal_tx_start_delayed_ref_w4r(&device)
    );
    TEST_ASSERT_EQUAL_size_t(1U, mock.fast_command_count);
    TEST_ASSERT_FALSE((device.state_flags & DW3000_DEVICE_STATE_TX_PENDING) != 0U);
}

void dw3000_hal_mock_run_tx_tests(void) {
    RUN_TEST(test_tx_prepare_frame_packs_buffer_and_frame_control);
    RUN_TEST(test_tx_fast_commands_timestamp_and_error_paths);
}
