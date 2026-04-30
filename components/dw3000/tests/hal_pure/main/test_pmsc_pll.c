#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/pll.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_types/phy.h"
#include "dw3000_types/pll.h"
#include "dw3000_types/pmsc.h"
#include "unity.h"

static void test_pmsc_validation_rejects_invalid_clock_seq_led_and_bias(void) {
    dw3000_device_config_t config;
    dw3000_pmsc_clk_ctrl_t clock;

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_pmsc_validate_config(&config.pmsc)
    );

    clock = config.pmsc.clk_ctrl;
    clock.sys_clk = DW3000_PMSC_CLK_SRC_FAST_RC;
    clock.rx_clk  = DW3000_PMSC_CLK_SRC_FORCE_PLL_LOW;
    clock.tx_clk  = DW3000_PMSC_CLK_SRC_FORCE_PLL_HIGH;
    clock.flags   = (dw3000_pmsc_clk_flags_t)(
        DW3000_PMSC_CLK_ACC_CLK_EN |
        DW3000_PMSC_CLK_CIA_CLK_EN |
        DW3000_PMSC_CLK_SAR_CLK_EN |
        DW3000_PMSC_CLK_ACC_MCLK_EN |
        DW3000_PMSC_CLK_GPIO_CLK_EN |
        DW3000_PMSC_CLK_GPIO_DCLK_EN |
        DW3000_PMSC_CLK_GPIO_DRST_N |
        DW3000_PMSC_CLK_LP_CLK_EN
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_pmsc_validate_clock_ctrl(&clock)
    );

    clock.sys_clk = (dw3000_pmsc_clk_src_t)4U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_clock_ctrl(&clock)
    );

    clock = config.pmsc.clk_ctrl;
    clock.flags = (dw3000_pmsc_clk_flags_t)(1UL << 31U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_clock_ctrl(&clock)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_clock_ctrl(NULL)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_pmsc_validate_seq_flags((dw3000_pmsc_seq_ctrl_flags_t)(
            DW3000_PMSC_SEQ_AINIT2IDLE |
            DW3000_PMSC_SEQ_ATX2SLP |
            DW3000_PMSC_SEQ_ARX2SLP |
            DW3000_PMSC_SEQ_PLL_SYNC |
            DW3000_PMSC_SEQ_CIARUNE
        ))
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_seq_flags(DW3000_PMSC_SEQ_FORCE2INIT)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_seq_flags((dw3000_pmsc_seq_ctrl_flags_t)1U)
    );

    config.pmsc.seq_flags = DW3000_PMSC_SEQ_FORCE2INIT;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_config(&config.pmsc)
    );

    dw3000_hal_default_config(&config);
    config.pmsc.led_ctrl.flags = DW3000_PMSC_LED_BLNKNOW_TX;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_config(&config.pmsc)
    );

    dw3000_hal_default_config(&config);
    config.pmsc.bias_ctrl = (dw3000_pmsc_bias_ctrl_t)(DW3000_PMSC_BIAS_CTRL_MASK + 1U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_config(&config.pmsc)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pmsc_validate_config(NULL)
    );
}

static void test_pll_channel_mapping_and_config_validation(void) {
    dw3000_pll_config_t config = {
        .cfg         = DW3000_PLL_CFG_CH5,
        .coarse_code = DW3000_PLL_COARSE_CODE_MASK,
        .cal_flags   = (dw3000_pll_cal_flags_t)(
            DW3000_PLL_CAL_USE_OLD |
            DW3000_PLL_CAL_EN
        ),
    };

    TEST_ASSERT_EQUAL_HEX16(
        DW3000_PLL_CFG_CH5,
        dw3000_hal_pll_cfg_for_channel(DW3000_PHY_CHANNEL_5)
    );
    TEST_ASSERT_EQUAL_HEX16(
        DW3000_PLL_CFG_CH9,
        dw3000_hal_pll_cfg_for_channel(DW3000_PHY_CHANNEL_9)
    );
    TEST_ASSERT_EQUAL_HEX16(
        0U,
        dw3000_hal_pll_cfg_for_channel((dw3000_phy_channel_t)0U)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_pll_validate_config(&config)
    );

    config.cfg = 0U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pll_validate_config(&config)
    );

    config.cfg         = DW3000_PLL_CFG_CH9;
    config.coarse_code = (dw3000_pll_coarse_code_t)(DW3000_PLL_COARSE_CODE_MASK + 1U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pll_validate_config(&config)
    );

    config.coarse_code = 0U;
    config.cal_flags   = (dw3000_pll_cal_flags_t)(1U << 1U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pll_validate_config(&config)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_pll_validate_config(NULL)
    );
}

void dw3000_hal_pure_run_pmsc_pll_tests(void) {
    RUN_TEST(test_pmsc_validation_rejects_invalid_clock_seq_led_and_bias);
    RUN_TEST(test_pll_channel_mapping_and_config_validation);
}
