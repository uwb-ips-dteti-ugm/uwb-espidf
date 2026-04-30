#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/calib.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/sync.h"
#include "dw3000_hal/tx_cal.h"
#include "dw3000_types/calib.h"
#include "dw3000_types/sync.h"
#include "dw3000_types/tx_cal.h"
#include "unity.h"

static void test_calib_validation_and_rx_cal_result_helpers(void) {
    dw3000_device_config_t        device_config;
    dw3000_calib_rx_cal_result_t  result = {
        .i = 1U,
        .q = 2U,
    };

    dw3000_hal_default_config(&device_config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_calib_validate_config(&device_config.calib)
    );

    device_config.calib.xtal_trim = DW3000_CALIB_XTAL_TRIM_MASK;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_calib_validate_config(&device_config.calib)
    );

    device_config.calib.xtal_trim = (dw3000_calib_xtal_trim_t)(
        DW3000_CALIB_XTAL_TRIM_MASK + 1U
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_calib_validate_config(&device_config.calib)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_calib_validate_config(NULL)
    );

    TEST_ASSERT_TRUE(dw3000_hal_calib_rx_cal_result_is_valid(&result));
    TEST_ASSERT_TRUE(dw3000_hal_sync_rx_cal_result_is_valid(&result));

    result.i = DW3000_CALIB_RX_CAL_RESULT_FAIL;
    TEST_ASSERT_FALSE(dw3000_hal_calib_rx_cal_result_is_valid(&result));
    TEST_ASSERT_FALSE(dw3000_hal_sync_rx_cal_result_is_valid(&result));

    result.i = 1U;
    result.q = DW3000_CALIB_RX_CAL_RESULT_FAIL;
    TEST_ASSERT_FALSE(dw3000_hal_calib_rx_cal_result_is_valid(&result));
    TEST_ASSERT_FALSE(dw3000_hal_sync_rx_cal_result_is_valid(&result));

    TEST_ASSERT_FALSE(dw3000_hal_calib_rx_cal_result_is_valid(NULL));
    TEST_ASSERT_FALSE(dw3000_hal_sync_rx_cal_result_is_valid(NULL));
}

static void test_tx_cal_validation_rejects_invalid_pgc_fields(void) {
    dw3000_tx_cal_config_t config = {
        .pgc_ctrl  = (dw3000_tx_cal_pgc_ctrl_t)(
            DW3000_TX_CAL_PGC_START |
            DW3000_TX_CAL_PGC_AUTOCAL_EN
        ),
        .pg_target = DW3000_TX_CAL_PG_TARGET_MASK,
    };

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_tx_cal_validate_config(&config)
    );

    config.pgc_ctrl = (dw3000_tx_cal_pgc_ctrl_t)(1U << 2U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_cal_validate_config(&config)
    );

    config.pgc_ctrl  = DW3000_TX_CAL_PGC_START;
    config.pg_target = (dw3000_tx_cal_pg_target_t)(DW3000_TX_CAL_PG_TARGET_MASK + 1U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_cal_validate_config(&config)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_tx_cal_validate_config(NULL)
    );
}

static void test_sync_ostr_wait_and_config_validation(void) {
    dw3000_sync_config_t config = {
        .flags = 0U,
        .wait  = 0U,
    };

    TEST_ASSERT_FALSE(dw3000_hal_sync_osts_wait_is_valid(0U));
    TEST_ASSERT_TRUE(dw3000_hal_sync_osts_wait_is_valid(1U));
    TEST_ASSERT_TRUE(dw3000_hal_sync_osts_wait_is_valid(
        DW3000_SYNC_OSTS_WAIT_RECOMMENDED
    ));
    TEST_ASSERT_TRUE(dw3000_hal_sync_osts_wait_is_valid(
        (dw3000_sync_osts_wait_t)(DW3000_SYNC_OSTS_WAIT_MAX - 2U)
    ));
    TEST_ASSERT_FALSE(dw3000_hal_sync_osts_wait_is_valid(
        DW3000_SYNC_OSTS_WAIT_MAX
    ));
    TEST_ASSERT_FALSE(dw3000_hal_sync_osts_wait_is_valid(34U));

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sync_validate_config(&config)
    );

    config.flags = DW3000_SYNC_OSTR_MODE;
    config.wait  = DW3000_SYNC_OSTS_WAIT_RECOMMENDED;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sync_validate_config(&config)
    );

    config.wait = 34U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sync_validate_config(&config)
    );

    config.flags = (dw3000_sync_ec_ctrl_flags_t)(1UL << 12U);
    config.wait  = DW3000_SYNC_OSTS_WAIT_RECOMMENDED;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sync_validate_config(&config)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sync_validate_config(NULL)
    );
}

void dw3000_hal_pure_run_calib_sync_tests(void) {
    RUN_TEST(test_calib_validation_and_rx_cal_result_helpers);
    RUN_TEST(test_tx_cal_validation_rejects_invalid_pgc_fields);
    RUN_TEST(test_sync_ostr_wait_and_config_validation);
}
