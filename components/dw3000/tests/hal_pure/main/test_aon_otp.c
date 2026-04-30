#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/aon.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/otp.h"
#include "dw3000_types/aon.h"
#include "dw3000_types/otp.h"
#include "dw3000_types/phy.h"
#include "unity.h"

static void test_aon_defaults_address_and_config_validation(void) {
    dw3000_device_config_t          config;
    dw3000_hal_aon_wake_options_t   options = {0};

    dw3000_hal_default_config(&config);
    dw3000_hal_aon_default_wake_options(&options);

    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_AON_WAKE_TIMEOUT_US,
        options.spi_ready_timeout_us
    );
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_AON_PLL_LOCK_TIMEOUT_US,
        options.pll_lock_timeout_us
    );
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_AON_RX_CAL_TIMEOUT_US,
        options.rx_cal_timeout_us
    );
    TEST_ASSERT_TRUE(options.reload_otp_ldo_bias);
    TEST_ASSERT_FALSE(options.run_rx_calibration);
    TEST_ASSERT_TRUE(options.enter_idle_pll);

    TEST_ASSERT_TRUE(dw3000_hal_aon_addr_is_valid(0U));
    TEST_ASSERT_TRUE(dw3000_hal_aon_addr_is_valid(DW3000_AON_ADDR_MASK));
    TEST_ASSERT_FALSE(dw3000_hal_aon_addr_is_valid(
        (dw3000_aon_addr_t)(DW3000_AON_ADDR_MASK + 1U)
    ));

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_validate_config(&config.aon)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_validate_dig_cfg((dw3000_aon_dig_cfg_t)(
            DW3000_AON_DIG_ONW_AON_DLD |
            DW3000_AON_DIG_ONW_RUN_SAR |
            DW3000_AON_DIG_ONW_GO2IDLE |
            DW3000_AON_DIG_ONW_GO2RX |
            DW3000_AON_DIG_ONW_PGFCAL
        ))
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_validate_dig_cfg((dw3000_aon_dig_cfg_t)(1UL << 12U))
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_validate_cfg((dw3000_aon_cfg_flags_t)(
            DW3000_AON_CFG_SLEEP_EN |
            DW3000_AON_CFG_WAKE_CNT |
            DW3000_AON_CFG_BROUT_EN |
            DW3000_AON_CFG_WAKE_CSN |
            DW3000_AON_CFG_WAKE_WUP |
            DW3000_AON_CFG_PRES_SLEEP
        ))
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_validate_cfg((dw3000_aon_cfg_flags_t)(1U << 6U))
    );

    config.aon.dig_cfg = (dw3000_aon_dig_cfg_t)(1UL << 12U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_validate_config(&config.aon)
    );

    dw3000_hal_default_config(&config);
    config.aon.cfg = (dw3000_aon_cfg_flags_t)(1U << 6U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_validate_config(&config.aon)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_validate_config(NULL)
    );
}

static void test_aon_sleep_time_conversion_validation(void) {
    dw3000_aon_sleep_time_t sleep_time = 0U;

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_sleep_time_from_us(1U, 32768U, &sleep_time)
    );
    TEST_ASSERT_EQUAL_UINT16(1U, sleep_time);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_sleep_time_from_us(125000U, 32768U, &sleep_time)
    );
    TEST_ASSERT_EQUAL_UINT16(1U, sleep_time);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_sleep_time_from_us(125001U, 32768U, &sleep_time)
    );
    TEST_ASSERT_EQUAL_UINT16(2U, sleep_time);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_sleep_time_from_us(0U, 32768U, &sleep_time)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_sleep_time_from_us(1U, 0U, &sleep_time)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aon_sleep_time_from_us(1U, 32768U, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_aon_sleep_time_from_us(UINT32_MAX, 1000000U, &sleep_time)
    );
}

static void test_otp_defaults_address_ranges_and_ops_selectors(void) {
    dw3000_hal_otp_program_options_t options = {0};

    dw3000_hal_otp_default_program_options(&options);

    TEST_ASSERT_FALSE(options.allow_non_customer_addr);
    TEST_ASSERT_FALSE(options.allow_non_empty_word);
    TEST_ASSERT_TRUE(options.require_vpp_ok);
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_OTP_PROGRAM_DEFAULT_TIMEOUT_US,
        options.timeout_us
    );

    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_valid(0U));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_valid(DW3000_OTP_ADDR_MASK));
    TEST_ASSERT_FALSE(dw3000_hal_otp_addr_is_valid(
        (dw3000_otp_addr_t)(DW3000_OTP_ADDR_MASK + 1U)
    ));

    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x00U));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x03U));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x10U));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x1FU));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x36U));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x5FU));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x62U));
    TEST_ASSERT_TRUE(dw3000_hal_otp_addr_is_customer(0x7FU));
    TEST_ASSERT_FALSE(dw3000_hal_otp_addr_is_customer(0x04U));
    TEST_ASSERT_FALSE(dw3000_hal_otp_addr_is_customer(0x20U));
    TEST_ASSERT_FALSE(dw3000_hal_otp_addr_is_customer(
        (dw3000_otp_addr_t)(DW3000_OTP_ADDR_MASK + 1U)
    ));

    TEST_ASSERT_TRUE(dw3000_hal_otp_ops_sel_is_valid(
        DW3000_OTP_OPS_SEL_LONG
    ));
    TEST_ASSERT_TRUE(dw3000_hal_otp_ops_sel_is_valid(
        DW3000_OTP_OPS_SEL_SHORT
    ));
    TEST_ASSERT_FALSE(dw3000_hal_otp_ops_sel_is_valid(
        (dw3000_otp_ops_sel_t)1U
    ));
}

static void test_otp_ops_selector_follows_preamble_length(void) {
    TEST_ASSERT_EQUAL(
        DW3000_OTP_OPS_SEL_SHORT,
        dw3000_hal_otp_ops_sel_for_preamble(DW3000_PHY_PREAMBLE_LEN_32)
    );
    TEST_ASSERT_EQUAL(
        DW3000_OTP_OPS_SEL_SHORT,
        dw3000_hal_otp_ops_sel_for_preamble(DW3000_PHY_PREAMBLE_LEN_64)
    );
    TEST_ASSERT_EQUAL(
        DW3000_OTP_OPS_SEL_SHORT,
        dw3000_hal_otp_ops_sel_for_preamble(DW3000_PHY_PREAMBLE_LEN_128)
    );
    TEST_ASSERT_EQUAL(
        DW3000_OTP_OPS_SEL_LONG,
        dw3000_hal_otp_ops_sel_for_preamble(DW3000_PHY_PREAMBLE_LEN_256)
    );
    TEST_ASSERT_EQUAL(
        DW3000_OTP_OPS_SEL_LONG,
        dw3000_hal_otp_ops_sel_for_preamble(DW3000_PHY_PREAMBLE_LEN_4096)
    );
    TEST_ASSERT_EQUAL(
        DW3000_OTP_OPS_SEL_LONG,
        dw3000_hal_otp_ops_sel_for_preamble((dw3000_phy_preamble_length_t)0U)
    );
}

void dw3000_hal_pure_run_aon_otp_tests(void) {
    RUN_TEST(test_aon_defaults_address_and_config_validation);
    RUN_TEST(test_aon_sleep_time_conversion_validation);
    RUN_TEST(test_otp_defaults_address_ranges_and_ops_selectors);
    RUN_TEST(test_otp_ops_selector_follows_preamble_length);
}
