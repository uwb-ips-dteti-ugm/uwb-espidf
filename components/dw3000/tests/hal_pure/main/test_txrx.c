#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/rx.h"
#include "dw3000_hal/tx.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/txrx.h"
#include "unity.h"

static void assert_event_mask_equal(
    dw3000_txrx_event_t expected,
    dw3000_txrx_event_t actual
) {
    TEST_ASSERT_EQUAL_HEX32(
        (uint32_t)(expected & UINT32_MAX),
        (uint32_t)(actual & UINT32_MAX)
    );
    TEST_ASSERT_EQUAL_HEX32(
        (uint32_t)(expected >> 32U),
        (uint32_t)(actual >> 32U)
    );
}

static void test_tx_validate_frame_standard_extended_and_buffer_bounds(void) {
    dw3000_device_t        device = {0};
    dw3000_txrx_tx_frame_t frame  = {0};

    dw3000_hal_default_config(&device.config);

    frame.tx_flen     = DW3000_HAL_TX_MAX_FRAME_STANDARD;
    frame.tx_b_offset = 0U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    frame.tx_flen = DW3000_HAL_TX_MAX_FRAME_STANDARD + 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    device.config.phy.phr_mode = DW3000_PHY_PHR_MODE_EXTENDED;
    frame.tx_flen             = DW3000_HAL_TX_MAX_FRAME_EXTENDED;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    frame.tx_flen = DW3000_HAL_TX_MAX_FRAME_EXTENDED + 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    frame.tx_flen     = 1U;
    frame.tx_b_offset = DW3000_HAL_TX_BUFFER_SIZE - 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    frame.tx_flen     = 2U;
    frame.tx_b_offset = DW3000_HAL_TX_BUFFER_SIZE - 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    frame.tx_flen     = 0U;
    frame.tx_b_offset = 895U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    frame.tx_b_offset = 896U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_validate_frame(&device, &frame)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_validate_frame(NULL, &frame)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_validate_frame(&device, NULL)
    );
}

static void test_rx_event_masks_group_success_error_and_all_events(void) {
    const dw3000_txrx_event_t success =
        DW3000_TXRX_EVENT_RXFR |
        DW3000_TXRX_EVENT_RXFCG;
    const dw3000_txrx_event_t errors =
        DW3000_TXRX_EVENT_RXPHE |
        DW3000_TXRX_EVENT_RXFCE |
        DW3000_TXRX_EVENT_RXFSL |
        DW3000_TXRX_EVENT_RXFTO |
        DW3000_TXRX_EVENT_CIAERR |
        DW3000_TXRX_EVENT_RXOVRR |
        DW3000_TXRX_EVENT_RXPTO |
        DW3000_TXRX_EVENT_RXSTO |
        DW3000_TXRX_EVENT_CPERR |
        DW3000_TXRX_EVENT_ARFE |
        DW3000_TXRX_EVENT_RXPREJ;
    const dw3000_txrx_event_t progress =
        DW3000_TXRX_EVENT_RXPRD |
        DW3000_TXRX_EVENT_RXSFDD |
        DW3000_TXRX_EVENT_CIADONE |
        DW3000_TXRX_EVENT_RXPHD;

    assert_event_mask_equal(success, dw3000_hal_rx_success_events());
    assert_event_mask_equal(errors, dw3000_hal_rx_error_events());
    assert_event_mask_equal(
        progress | success | errors,
        dw3000_hal_rx_all_events()
    );

    assert_event_mask_equal(0U, success & errors);
    assert_event_mask_equal(0U, progress & errors);
    assert_event_mask_equal(0U, progress & success);
    assert_event_mask_equal(
        0U,
        dw3000_hal_rx_all_events() &
            (DW3000_TXRX_EVENT_TXFRS | DW3000_TXRX_EVENT_AES_DONE)
    );
}

void dw3000_hal_pure_run_txrx_tests(void) {
    RUN_TEST(test_tx_validate_frame_standard_extended_and_buffer_bounds);
    RUN_TEST(test_rx_event_masks_group_success_error_and_all_events);
}
