#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/mac.h"
#include "dw3000_types/mac.h"
#include "unity.h"

static void test_mac_ack_response_validation(void) {
    dw3000_mac_ack_resp_t ack_resp = {0};

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_mac_validate_ack_response(&ack_resp)
    );

    ack_resp.ack_tim = UINT8_MAX;
    ack_resp.w4r_tim = 0x000FFFFFUL;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_mac_validate_ack_response(&ack_resp)
    );

    ack_resp.w4r_tim = 0x00100000UL;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_ack_response(&ack_resp)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_ack_response(NULL)
    );
}

static void test_mac_config_validation_rejects_invalid_flags(void) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    config.mac.ff_flags = (dw3000_mac_ff_flags_t)(
        DW3000_MAC_FF_ALLOW_DATA |
        DW3000_MAC_FF_ALLOW_ACK |
        DW3000_MAC_FF_IMPLICIT_BROADCAST |
        DW3000_MAC_FF_SHORT_SRC_PEND_ACK |
        DW3000_MAC_FF_LONG_SRC_PEND_ACK
    );
    config.mac.sys_cfg_flags = DW3000_MAC_SYS_CFG_FF_ENABLE;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    config.mac.sys_cfg_flags = (dw3000_mac_sys_cfg_flags_t)(
        DW3000_MAC_SYS_CFG_FF_ENABLE |
        DW3000_MAC_SYS_CFG_AUTO_ACK |
        DW3000_MAC_SYS_CFG_FAST_AAT
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    config.mac.sys_cfg_flags = DW3000_MAC_SYS_CFG_AUTO_ACK;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    config.mac.sys_cfg_flags = (dw3000_mac_sys_cfg_flags_t)(1UL << 19U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    config.mac.sys_cfg_flags = DW3000_MAC_SYS_CFG_FF_ENABLE;
    config.mac.ff_flags      = (dw3000_mac_ff_flags_t)(1UL << 16U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    dw3000_hal_default_config(&config);
    config.mac.ack_resp.w4r_tim = 0x00100000UL;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_config(&config.mac)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_mac_validate_config(NULL)
    );
}

void dw3000_hal_pure_run_mac_tests(void) {
    RUN_TEST(test_mac_ack_response_validation);
    RUN_TEST(test_mac_config_validation_rejects_invalid_flags);
}
