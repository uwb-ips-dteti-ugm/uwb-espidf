#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/aes.h"
#include "dw3000_hal/aon.h"
#include "dw3000_hal/calib.h"
#include "dw3000_hal/cia.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/gpio.h"
#include "dw3000_hal/mac.h"
#include "dw3000_hal/phy.h"
#include "dw3000_hal/pmsc.h"
#include "dw3000_hal/sts.h"
#include "dw3000_hal/sync.h"
#include "dw3000_types/device.h"
#include "dw3000_types/phy.h"
#include "unity.h"

static void test_core_default_options_and_config_are_valid(void) {
    dw3000_hal_bringup_t      bringup = {0};
    dw3000_hal_init_options_t options = {0};
    dw3000_device_config_t    config;

    dw3000_hal_default_bringup(&bringup);
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_DEFAULT_RESET_ASSERT_US,
        bringup.reset_assert_us
    );
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_DEFAULT_RESET_SETTLE_US,
        bringup.reset_settle_us
    );
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_DEFAULT_READY_TIMEOUT_US,
        bringup.ready_timeout_us
    );

    dw3000_hal_default_init_options(&options);
    TEST_ASSERT_EQUAL_UINT32(DW3000_HAL_INIT_STEP_ALL, options.steps);
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_HAL_DEFAULT_READY_TIMEOUT_US,
        options.idle_pll_timeout_us
    );

    dw3000_hal_default_bringup(NULL);
    dw3000_hal_default_init_options(NULL);
    dw3000_hal_default_config(NULL);

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(DW3000_PHY_CHANNEL_5, config.phy.channel);
    TEST_ASSERT_EQUAL(DW3000_PHY_PRF_64_MHZ, config.phy.prf);
    TEST_ASSERT_EQUAL(DW3000_PHY_DATA_RATE_6M81, config.phy.data_rate);
    TEST_ASSERT_EQUAL(DW3000_PHY_PREAMBLE_LEN_64, config.phy.preamble_length);
    TEST_ASSERT_EQUAL(DW3000_PHY_SFD_TYPE_IEEE_802154Z, config.phy.sfd_type);
    TEST_ASSERT_EQUAL(DW3000_PHY_PAC_SIZE_8, config.phy.pac_size);
    TEST_ASSERT_TRUE(config.use_double_buffer);
    TEST_ASSERT_TRUE(config.load_otp_calibration);
    TEST_ASSERT_TRUE(config.auto_init_pll);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_phy_validate_config(&config.phy)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_phy_validate_rx_tune(&config.rx_tune)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_mac_validate_config(&config.mac)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sts_validate_config(&config.sts)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_cia_validate_config(&config.cia)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_pmsc_validate_config(&config.pmsc)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aon_validate_config(&config.aon)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_config(&config.gpio)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_calib_validate_config(&config.calib)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_sync_validate_config(&config.sync)
    );
}

static void test_core_device_id_support_and_capabilities(void) {
    dw3000_device_id_t id = {
        .rev    = 0x02U,
        .ver    = 0x00U,
        .model  = DW3000_DEVICE_MODEL_DW3000,
        .ridtag = DW3000_DEVICE_RIDTAG_DECAWAVE,
    };

    TEST_ASSERT_TRUE(dw3000_hal_is_supported_device_id(&id));
    TEST_ASSERT_EQUAL(
        DW3000_DEVICE_CAP_NONE,
        dw3000_hal_capabilities_from_device_id(&id)
    );

    id.ver = 0x01U;
    TEST_ASSERT_TRUE(dw3000_hal_is_supported_device_id(&id));
    TEST_ASSERT_EQUAL(
        DW3000_DEVICE_CAP_PDOA,
        dw3000_hal_capabilities_from_device_id(&id)
    );

    id.ver = 0x02U;
    TEST_ASSERT_FALSE(dw3000_hal_is_supported_device_id(&id));
    TEST_ASSERT_EQUAL(
        DW3000_DEVICE_CAP_NONE,
        dw3000_hal_capabilities_from_device_id(&id)
    );

    id.ver = 0x00U;
    id.rev = 0x01U;
    TEST_ASSERT_FALSE(dw3000_hal_is_supported_device_id(&id));

    id.rev   = 0x02U;
    id.model = 0x04U;
    TEST_ASSERT_FALSE(dw3000_hal_is_supported_device_id(&id));

    id.model  = DW3000_DEVICE_MODEL_DW3000;
    id.ridtag = 0xDECBU;
    TEST_ASSERT_FALSE(dw3000_hal_is_supported_device_id(&id));

    TEST_ASSERT_FALSE(dw3000_hal_is_supported_device_id(NULL));
    TEST_ASSERT_EQUAL(
        DW3000_DEVICE_CAP_NONE,
        dw3000_hal_capabilities_from_device_id(NULL)
    );
}

void dw3000_hal_pure_run_core_tests(void) {
    RUN_TEST(test_core_default_options_and_config_are_valid);
    RUN_TEST(test_core_device_id_support_and_capabilities);
}
