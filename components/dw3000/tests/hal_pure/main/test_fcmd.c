#include <stdbool.h>
#include <stdint.h>

#include "dw3000_hal/fcmd.h"
#include "dw3000_hal/rx.h"
#include "dw3000_hal/tx.h"
#include "dw3000_types/fcmd.h"
#include "unity.h"

static void assert_tx_start_command(dw3000_fcmd_t command) {
    TEST_ASSERT_TRUE(dw3000_hal_fcmd_is_valid(command));
    TEST_ASSERT_TRUE(dw3000_hal_tx_is_start_command(command));
    TEST_ASSERT_FALSE(dw3000_hal_rx_is_start_command(command));
}

static void assert_rx_start_command(dw3000_fcmd_t command) {
    TEST_ASSERT_TRUE(dw3000_hal_fcmd_is_valid(command));
    TEST_ASSERT_FALSE(dw3000_hal_tx_is_start_command(command));
    TEST_ASSERT_TRUE(dw3000_hal_rx_is_start_command(command));
}

static void assert_control_command(dw3000_fcmd_t command) {
    TEST_ASSERT_TRUE(dw3000_hal_fcmd_is_valid(command));
    TEST_ASSERT_FALSE(dw3000_hal_tx_is_start_command(command));
    TEST_ASSERT_FALSE(dw3000_hal_rx_is_start_command(command));
}

static void test_fcmd_headers_and_start_command_classification(void) {
    TEST_ASSERT_TRUE(dw3000_hal_fcmd_is_valid(DW3000_FCMD_TXRXOFF));
    TEST_ASSERT_TRUE(dw3000_hal_fcmd_is_valid(DW3000_FCMD_DB_TOGGLE));
    TEST_ASSERT_FALSE(dw3000_hal_fcmd_is_valid((dw3000_fcmd_t)0x14U));

    TEST_ASSERT_EQUAL_HEX8(0x81U, dw3000_hal_fcmd_header(DW3000_FCMD_TXRXOFF));
    TEST_ASSERT_EQUAL_HEX8(0x83U, dw3000_hal_fcmd_header(DW3000_FCMD_TX));
    TEST_ASSERT_EQUAL_HEX8(0x85U, dw3000_hal_fcmd_header(DW3000_FCMD_RX));
    TEST_ASSERT_EQUAL_HEX8(0xA5U, dw3000_hal_fcmd_header(DW3000_FCMD_CLR_IRQS));
    TEST_ASSERT_EQUAL_HEX8(0xA7U, dw3000_hal_fcmd_header(DW3000_FCMD_DB_TOGGLE));

    assert_tx_start_command(DW3000_FCMD_TX);
    assert_tx_start_command(DW3000_FCMD_DTX);
    assert_tx_start_command(DW3000_FCMD_DTX_TS);
    assert_tx_start_command(DW3000_FCMD_DTX_RS);
    assert_tx_start_command(DW3000_FCMD_DTX_REF);
    assert_tx_start_command(DW3000_FCMD_CCA_TX);
    assert_tx_start_command(DW3000_FCMD_TX_W4R);
    assert_tx_start_command(DW3000_FCMD_DTX_W4R);
    assert_tx_start_command(DW3000_FCMD_DTX_TS_W4R);
    assert_tx_start_command(DW3000_FCMD_DTX_RS_W4R);
    assert_tx_start_command(DW3000_FCMD_DTX_REF_W4R);
    assert_tx_start_command(DW3000_FCMD_CCA_TX_W4R);

    assert_rx_start_command(DW3000_FCMD_RX);
    assert_rx_start_command(DW3000_FCMD_DRX);
    assert_rx_start_command(DW3000_FCMD_DRX_TS);
    assert_rx_start_command(DW3000_FCMD_DRX_RS);
    assert_rx_start_command(DW3000_FCMD_DRX_REF);

    assert_control_command(DW3000_FCMD_TXRXOFF);
    assert_control_command(DW3000_FCMD_CLR_IRQS);
    assert_control_command(DW3000_FCMD_DB_TOGGLE);

    TEST_ASSERT_FALSE(dw3000_hal_tx_is_start_command((dw3000_fcmd_t)0x14U));
    TEST_ASSERT_FALSE(dw3000_hal_rx_is_start_command((dw3000_fcmd_t)0x14U));
}

void dw3000_hal_pure_run_fcmd_tests(void) {
    RUN_TEST(test_fcmd_headers_and_start_command_classification);
}
