#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/status.h"
#include "dw3000_types/txrx.h"
#include "mock_dw3000_port.h"
#include "unity.h"

static void assert_event_equal(
    dw3000_txrx_event_t expected,
    dw3000_txrx_event_t actual
) {
    TEST_ASSERT_EQUAL_HEX32((uint32_t)expected, (uint32_t)actual);
    TEST_ASSERT_EQUAL_HEX32(
        (uint32_t)(expected >> 32U),
        (uint32_t)(actual >> 32U)
    );
}

static uint64_t mock_get_u48(
    const dw3000_mock_port_t* mock,
    dw3000_reg_desc_t         reg
) {
    uint64_t value = 0U;

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_get_u48(mock, reg, &value)
    );
    return value;
}

static void test_status_read_clear_and_cached_enable_updates(void) {
    dw3000_mock_port_t        mock;
    dw3000_device_t           device;
    dw3000_txrx_event_t       status;
    dw3000_txrx_event_t       enabled;
    const dw3000_txrx_event_t initial_status =
        DW3000_TXRX_EVENT_IRQS |
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_RXFCG |
        DW3000_TXRX_EVENT_RXPREJ |
        DW3000_TXRX_EVENT_AES_DONE;
    const dw3000_txrx_event_t clear_request =
        DW3000_TXRX_EVENT_IRQS |
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_RXFCG;
    const dw3000_txrx_event_t clear_written =
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_RXFCG;
    const dw3000_txrx_event_t enable_request =
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_RXFCG |
        DW3000_TXRX_EVENT_AES_DONE |
        (UINT64_C(1) << 63U);
    dw3000_txrx_event_t expected_enabled =
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_RXFCG |
        DW3000_TXRX_EVENT_AES_DONE;

    dw3000_mock_port_init(&mock, &device);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_set_u48(&mock, DW3000_REG_SYS_STATUS, initial_status)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_read(&device, &status)
    );
    assert_event_equal(initial_status, status);
    TEST_ASSERT_EQUAL_size_t(1U, mock.read_count);
    TEST_ASSERT_EQUAL_size_t(1U, mock.lock_count);
    TEST_ASSERT_EQUAL_size_t(1U, mock.unlock_count);

    TEST_ASSERT_TRUE(dw3000_hal_status_has_event(status, DW3000_TXRX_EVENT_RXFCG));
    TEST_ASSERT_FALSE(dw3000_hal_status_has_event(status, DW3000_TXRX_EVENT_RXFTO));

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_clear(&device, clear_request)
    );
    assert_event_equal(clear_written, dw3000_mock_port_last_write_u48(&mock));
    TEST_ASSERT_EQUAL_UINT8(DW3000_REG_SYS_STATUS.file_id, mock.last_write_file_id);
    TEST_ASSERT_EQUAL_UINT16(DW3000_REG_SYS_STATUS.offset, mock.last_write_offset);
    assert_event_equal(initial_status & ~clear_written, mock_get_u48(&mock, DW3000_REG_SYS_STATUS));

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_set_enabled(&device, enable_request)
    );
    assert_event_equal(expected_enabled, mock_get_u48(&mock, DW3000_REG_SYS_ENABLE));
    assert_event_equal(expected_enabled, device.enabled_events);
    TEST_ASSERT_TRUE(device.enabled_events_valid);

    TEST_ASSERT_EQUAL_size_t(1U, mock.read_count);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_enable(&device, DW3000_TXRX_EVENT_RXFTO)
    );
    expected_enabled |= DW3000_TXRX_EVENT_RXFTO;
    assert_event_equal(expected_enabled, mock_get_u48(&mock, DW3000_REG_SYS_ENABLE));
    assert_event_equal(expected_enabled, device.enabled_events);
    TEST_ASSERT_EQUAL_size_t(1U, mock.read_count);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_disable(&device, DW3000_TXRX_EVENT_TXFRS)
    );
    expected_enabled &= ~DW3000_TXRX_EVENT_TXFRS;
    assert_event_equal(expected_enabled, mock_get_u48(&mock, DW3000_REG_SYS_ENABLE));
    assert_event_equal(expected_enabled, device.enabled_events);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_read_enabled(&device, &enabled)
    );
    assert_event_equal(expected_enabled, enabled);
}

static void test_status_uncached_enable_and_io_error_propagation(void) {
    dw3000_mock_port_t  mock;
    dw3000_device_t     device;
    dw3000_txrx_event_t expected_enabled =
        DW3000_TXRX_EVENT_TXFRS |
        DW3000_TXRX_EVENT_RXFCG;

    dw3000_mock_port_init(&mock, &device);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_mock_port_set_u48(&mock, DW3000_REG_SYS_ENABLE, DW3000_TXRX_EVENT_TXFRS)
    );

    TEST_ASSERT_FALSE(device.enabled_events_valid);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_status_enable(&device, DW3000_TXRX_EVENT_RXFCG)
    );
    TEST_ASSERT_EQUAL_size_t(1U, mock.read_count);
    TEST_ASSERT_EQUAL_size_t(1U, mock.write_count);
    assert_event_equal(expected_enabled, mock_get_u48(&mock, DW3000_REG_SYS_ENABLE));
    assert_event_equal(expected_enabled, device.enabled_events);
    TEST_ASSERT_TRUE(device.enabled_events_valid);

    device.enabled_events_valid = false;
    dw3000_mock_port_set_next_read_error(&mock, DW3000_ERROR_IO);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_IO,
        dw3000_hal_status_enable(&device, DW3000_TXRX_EVENT_AES_DONE)
    );
    TEST_ASSERT_EQUAL_size_t(1U, mock.write_count);
    TEST_ASSERT_FALSE(device.enabled_events_valid);

    dw3000_mock_port_set_next_write_error(&mock, DW3000_ERROR_IO);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_IO,
        dw3000_hal_status_set_enabled(&device, DW3000_TXRX_EVENT_AES_DONE)
    );
    assert_event_equal(expected_enabled, mock_get_u48(&mock, DW3000_REG_SYS_ENABLE));
    TEST_ASSERT_FALSE(device.enabled_events_valid);
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_status_read_clear_and_cached_enable_updates);
    RUN_TEST(test_status_uncached_enable_and_io_error_propagation);
    (void)UNITY_END();
}
