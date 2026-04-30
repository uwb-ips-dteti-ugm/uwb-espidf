#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/sts.h"
#include "dw3000_types/cia.h"
#include "dw3000_types/sts.h"
#include "unity.h"

static void test_sts_validation_and_timestamp_quality(void) {
    dw3000_device_config_t config;
    dw3000_cia_path_ts_t   timestamp = {0};

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sts_validate_config(&config.sts)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP0,
            DW3000_STS_PDOA_MODE_DISABLED,
            0U
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP1,
            DW3000_STS_PDOA_MODE_1,
            DW3000_STS_SYS_CFG_CIA_STS
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP3,
            DW3000_STS_PDOA_MODE_3,
            DW3000_STS_SYS_CFG_CIA_STS
        )
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP0,
            DW3000_STS_PDOA_MODE_DISABLED,
            DW3000_STS_SYS_CFG_CIA_STS
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP0,
            DW3000_STS_PDOA_MODE_1,
            0U
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP1,
            DW3000_STS_PDOA_MODE_3,
            DW3000_STS_SYS_CFG_CIA_STS
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_sys_cfg(
            (dw3000_sts_packet_cfg_t)4U,
            DW3000_STS_PDOA_MODE_DISABLED,
            0U
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP1,
            (dw3000_sts_pdoa_mode_t)2U,
            0U
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_sys_cfg(
            DW3000_STS_PACKET_CFG_SP1,
            DW3000_STS_PDOA_MODE_DISABLED,
            (dw3000_sts_sys_cfg_flags_t)(1UL << 31)
        )
    );

    config.sts.cps_len = DW3000_HAL_STS_CPS_LEN_MIN - 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_sts_validate_config(&config.sts)
    );

    TEST_ASSERT_EQUAL_UINT16(0U, dw3000_hal_sts_length_units(0U));
    TEST_ASSERT_EQUAL_UINT16(64U, dw3000_hal_sts_length_units(7U));
    TEST_ASSERT_EQUAL_UINT16(2048U, dw3000_hal_sts_length_units(255U));

    TEST_ASSERT_EQUAL_UINT16(0U, dw3000_hal_sts_acc_qual_threshold(0U));
    TEST_ASSERT_EQUAL_UINT16(39U, dw3000_hal_sts_acc_qual_threshold(7U));
    TEST_ASSERT_EQUAL_UINT16(1229U, dw3000_hal_sts_acc_qual_threshold(255U));

    TEST_ASSERT_FALSE(dw3000_hal_sts_acc_qual_is_sufficient(7U, 38U));
    TEST_ASSERT_TRUE(dw3000_hal_sts_acc_qual_is_sufficient(7U, 39U));
    TEST_ASSERT_FALSE(dw3000_hal_sts_acc_qual_is_sufficient(0U, 4095U));

    TEST_ASSERT_TRUE(dw3000_hal_sts_timestamp_is_reliable(
        7U,
        &timestamp,
        39U
    ));

    timestamp.toast = 1U;
    TEST_ASSERT_FALSE(dw3000_hal_sts_timestamp_is_reliable(
        7U,
        &timestamp,
        39U
    ));

    timestamp.toast = 0U;
    TEST_ASSERT_FALSE(dw3000_hal_sts_timestamp_is_reliable(
        7U,
        &timestamp,
        38U
    ));
    TEST_ASSERT_FALSE(dw3000_hal_sts_timestamp_is_reliable(7U, NULL, 39U));
}

void dw3000_hal_pure_run_sts_tests(void) {
    RUN_TEST(test_sts_validation_and_timestamp_quality);
}
