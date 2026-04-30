#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/rf.h"
#include "dw3000_types/device.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/rf.h"
#include "unity.h"

static void test_rf_channel_helpers_map_supported_channels(void) {
    TEST_ASSERT_EQUAL_HEX32(
        DW3000_RF_FORCE_TX_CH5,
        dw3000_hal_rf_force_tx_value(DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL_HEX32(
        DW3000_RF_FORCE_TX_CH9,
        dw3000_hal_rf_force_tx_value(DW3000_PHY_CHANNEL_9)
    );
    TEST_ASSERT_EQUAL_HEX32(
        0U,
        dw3000_hal_rf_force_tx_value((dw3000_phy_channel_t)2U)
    );

    TEST_ASSERT_EQUAL_HEX8(
        DW3000_RF_TX_CTRL_1_OPT,
        dw3000_hal_rf_tx_ctrl_1_value(DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL_HEX8(
        DW3000_RF_TX_CTRL_1_OPT,
        dw3000_hal_rf_tx_ctrl_1_value(DW3000_PHY_CHANNEL_9)
    );
    TEST_ASSERT_EQUAL_HEX8(
        0U,
        dw3000_hal_rf_tx_ctrl_1_value((dw3000_phy_channel_t)2U)
    );

    TEST_ASSERT_EQUAL_HEX32(
        DW3000_RF_TX_CTRL_2_CH5,
        dw3000_hal_rf_tx_ctrl_2_value(DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL_HEX32(
        DW3000_RF_TX_CTRL_2_CH9,
        dw3000_hal_rf_tx_ctrl_2_value(DW3000_PHY_CHANNEL_9)
    );
    TEST_ASSERT_EQUAL_HEX32(
        0U,
        dw3000_hal_rf_tx_ctrl_2_value((dw3000_phy_channel_t)2U)
    );
}

static void test_rf_validation_paths_fail_before_register_transport(void) {
    dw3000_device_t       busy_device = {
        .state_flags = DW3000_DEVICE_STATE_RX_ON,
    };
    uint32_t              rf_enable;
    dw3000_rf_switch_t    rf_switch;
    dw3000_rf_tx_ctrl_1_t tx_ctrl_1;
    dw3000_rf_tx_ctrl_2_t tx_ctrl_2;
    dw3000_rf_tx_test_t   tx_test;
    dw3000_rf_sar_test_t  sar_test;
    dw3000_rf_ldo_tune_t  ldo_tune = {0};
    dw3000_rf_ldo_ctrl_t  ldo_ctrl;
    dw3000_rf_ldo_rload_t ldo_rload;

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_enable(NULL, &rf_enable)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_enable(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_enable(NULL, 0U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_enable(&busy_device, 0U)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_force_tx_path(NULL, DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_force_tx_path(&busy_device, (dw3000_phy_channel_t)2U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_force_tx_path(&busy_device, DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_clear_forced_path(NULL)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_switch(NULL, &rf_switch)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_switch(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_switch(&busy_device, 0U)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_tx_ctrl_1(NULL, &tx_ctrl_1)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_tx_ctrl_1(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_tx_ctrl_1(&busy_device, DW3000_RF_TX_CTRL_1_OPT)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_tx_ctrl_2(NULL, &tx_ctrl_2)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_tx_ctrl_2(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_tx_ctrl_2(&busy_device, DW3000_RF_TX_CTRL_2_CH5)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_configure_tx_channel(NULL, DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_configure_tx_channel(
            &busy_device,
            (dw3000_phy_channel_t)2U
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_configure_tx_channel(&busy_device, DW3000_PHY_CHANNEL_5)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_tx_test(NULL, &tx_test)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_tx_test(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_tx_test(NULL, DW3000_RF_TX_TEST_NORMAL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_tx_test(&busy_device, 0x10U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_tx_test(&busy_device, DW3000_RF_TX_TEST_CW)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_sar_test(NULL, &sar_test)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_sar_test(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_sar_test(NULL, 0U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_sar_test(&busy_device, 0x01U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_set_sar_read_enable(&busy_device, true)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_ldo_tune(NULL, &ldo_tune)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_ldo_tune(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_ldo_tune(NULL, &ldo_tune)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_ldo_tune(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_ldo_tune(&busy_device, &ldo_tune)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_ldo_ctrl(NULL, &ldo_ctrl)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_ldo_ctrl(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_ldo_ctrl(NULL, 0U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_ldo_ctrl(
            &busy_device,
            (dw3000_rf_ldo_ctrl_t)(1UL << 31U)
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_ldo_ctrl(&busy_device, DW3000_RF_LDO_VDDPLL)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_ldo_rload(NULL, &ldo_rload)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_read_ldo_rload(&busy_device, NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_rf_write_ldo_rload(NULL, DW3000_RF_LDO_RLOAD_OPT)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_BUSY,
        dw3000_hal_rf_write_ldo_rload(&busy_device, DW3000_RF_LDO_RLOAD_OPT)
    );
}

void dw3000_hal_pure_run_rf_tests(void) {
    RUN_TEST(test_rf_channel_helpers_map_supported_channels);
    RUN_TEST(test_rf_validation_paths_fail_before_register_transport);
}
