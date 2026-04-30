#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/phy.h"
#include "dw3000_types/phy.h"
#include "unity.h"

void dw3000_hal_pure_run_sts_tests(void);
void dw3000_hal_pure_run_gpio_tests(void);
void dw3000_hal_pure_run_fcmd_tests(void);
void dw3000_hal_pure_run_txrx_tests(void);

static void test_phy_sfd_timeout_formula_and_validation(void) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_phy_validate_config(&config.phy)
    );
    TEST_ASSERT_EQUAL_UINT16(64U, dw3000_hal_phy_preamble_symbols(DW3000_PHY_PREAMBLE_LEN_64));
    TEST_ASSERT_EQUAL_UINT8(8U, dw3000_hal_phy_pac_symbols(DW3000_PHY_PAC_SIZE_8));
    TEST_ASSERT_EQUAL_UINT8(8U, dw3000_hal_phy_sfd_symbols(DW3000_PHY_SFD_TYPE_IEEE_802154Z));
    TEST_ASSERT_EQUAL_UINT16(65U, dw3000_hal_phy_sfd_timeout(&config.phy));

    config.phy.preamble_length = DW3000_PHY_PREAMBLE_LEN_128;
    config.phy.pac_size        = DW3000_PHY_PAC_SIZE_8;
    config.phy.sfd_type        = DW3000_PHY_SFD_TYPE_DECAWAVE_8;

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_phy_validate_config(&config.phy)
    );
    TEST_ASSERT_EQUAL_UINT16(129U, dw3000_hal_phy_sfd_timeout(&config.phy));

    config.phy.sfd_type = DW3000_PHY_SFD_TYPE_DECAWAVE_16;
    TEST_ASSERT_EQUAL_UINT16(137U, dw3000_hal_phy_sfd_timeout(&config.phy));

    config.phy.sfd_type = (dw3000_phy_sfd_type_t)4U;
    TEST_ASSERT_EQUAL(DW3000_ERROR_INVALID_ARG, dw3000_hal_phy_validate_config(&config.phy));
    TEST_ASSERT_EQUAL_UINT16(0U, dw3000_hal_phy_sfd_timeout(&config.phy));
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_phy_sfd_timeout_formula_and_validation);
    dw3000_hal_pure_run_sts_tests();
    dw3000_hal_pure_run_gpio_tests();
    dw3000_hal_pure_run_fcmd_tests();
    dw3000_hal_pure_run_txrx_tests();
    (void)UNITY_END();
}
