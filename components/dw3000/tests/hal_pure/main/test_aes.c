#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/aes.h"
#include "dw3000_hal/core.h"
#include "dw3000_types/aes.h"
#include "unity.h"

static void test_aes_size_helpers_and_status_errors(void) {
    TEST_ASSERT_EQUAL_size_t(0U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_NONE
    ));
    TEST_ASSERT_EQUAL_size_t(4U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_4
    ));
    TEST_ASSERT_EQUAL_size_t(6U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_6
    ));
    TEST_ASSERT_EQUAL_size_t(8U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_8
    ));
    TEST_ASSERT_EQUAL_size_t(10U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_10
    ));
    TEST_ASSERT_EQUAL_size_t(12U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_12
    ));
    TEST_ASSERT_EQUAL_size_t(14U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_14
    ));
    TEST_ASSERT_EQUAL_size_t(16U, dw3000_hal_aes_tag_size_bytes(
        DW3000_AES_TAG_SIZE_16
    ));
    TEST_ASSERT_EQUAL_size_t(0U, dw3000_hal_aes_tag_size_bytes(
        (dw3000_aes_tag_size_t)8U
    ));

    TEST_ASSERT_EQUAL_size_t(16U, dw3000_hal_aes_key_size_bytes(
        DW3000_AES_KEY_SIZE_128
    ));
    TEST_ASSERT_EQUAL_size_t(24U, dw3000_hal_aes_key_size_bytes(
        DW3000_AES_KEY_SIZE_192
    ));
    TEST_ASSERT_EQUAL_size_t(32U, dw3000_hal_aes_key_size_bytes(
        DW3000_AES_KEY_SIZE_256
    ));
    TEST_ASSERT_EQUAL_size_t(0U, dw3000_hal_aes_key_size_bytes(
        (dw3000_aes_key_size_t)3U
    ));

    TEST_ASSERT_FALSE(dw3000_hal_aes_status_has_error(DW3000_AES_STS_AES_DONE));
    TEST_ASSERT_TRUE(dw3000_hal_aes_status_has_error(DW3000_AES_STS_AUTH_ERR));
    TEST_ASSERT_TRUE(dw3000_hal_aes_status_has_error(DW3000_AES_STS_TRANS_ERR));
    TEST_ASSERT_TRUE(dw3000_hal_aes_status_has_error(DW3000_AES_STS_MEM_CONF));
    TEST_ASSERT_TRUE(dw3000_hal_aes_status_has_error(
        (dw3000_aes_status_t)(DW3000_AES_STS_AES_DONE | DW3000_AES_STS_AUTH_ERR)
    ));
}

static void test_aes_dma_validation_rejects_invalid_fields(void) {
    dw3000_device_config_t config;
    dw3000_aes_dma_cfg_t   dma;

    dw3000_hal_default_config(&config);
    dma = config.aes.dma;

    TEST_ASSERT_EQUAL(DW3000_ERROR_OK, dw3000_hal_aes_validate_dma_cfg(&dma));

    dma.src_port = DW3000_AES_PORT_STS_KEY;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    dma = config.aes.dma;
    dma.dst_port = DW3000_AES_PORT_STS_KEY;
    TEST_ASSERT_EQUAL(DW3000_ERROR_OK, dw3000_hal_aes_validate_dma_cfg(&dma));

    dma.dst_port = DW3000_AES_PORT_NONE;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    dma = config.aes.dma;
    dma.endianness = (dw3000_aes_endianness_t)2U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    dma = config.aes.dma;
    dma.src_addr = 0x0400U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    dma = config.aes.dma;
    dma.dst_addr = 0x0400U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    dma = config.aes.dma;
    dma.hdr_size = 0x80U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    dma = config.aes.dma;
    dma.pyld_size = 0x0400U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(&dma)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_dma_cfg(NULL)
    );
}

static void test_aes_config_validation_rejects_invalid_and_unsupported_modes(void) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    config.aes.mode = (dw3000_aes_mode_t)2U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.key_size = (dw3000_aes_key_size_t)3U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.tag_size = (dw3000_aes_tag_size_t)8U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.core = (dw3000_aes_core_t)2U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.key_src = (dw3000_aes_key_src_t)2U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.flags = (dw3000_aes_cfg_flags_t)(1U << 15U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.key_addr = DW3000_AES_KEY_RAM_SLOTS;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.core     = DW3000_AES_CORE_CCM;
    config.aes.key_size = DW3000_AES_KEY_SIZE_192;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_NOT_SUPPORTED,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.key_size = DW3000_AES_KEY_SIZE_192;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_NOT_SUPPORTED,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    config.aes.flags = DW3000_AES_CFG_KEY_OTP;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.key_src  = DW3000_AES_KEY_SRC_RAM;
    config.aes.key_size = DW3000_AES_KEY_SIZE_256;
    config.aes.key_addr = DW3000_AES_KEY_RAM_SLOTS - 2U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    config.aes.key_addr = DW3000_AES_KEY_RAM_SLOTS - 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_aes_validate_cfg(&config.aes)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_cfg(NULL)
    );
}

static void test_aes_transfer_validation_checks_port_ranges_and_tag_size(void) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);
    config.aes.dma.hdr_size  = 7U;
    config.aes.dma.pyld_size = 120U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    config.aes.dma.pyld_size = 121U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.tag_size      = DW3000_AES_TAG_SIZE_16;
    config.aes.dma.pyld_size = 111U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    config.aes.dma.pyld_size = 112U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.dma.src_port  = DW3000_AES_PORT_TX;
    config.aes.dma.dst_port  = DW3000_AES_PORT_STS_KEY;
    config.aes.dma.pyld_size = DW3000_AES_STS_KEY_SIZE;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    config.aes.dma.pyld_size = DW3000_AES_STS_KEY_SIZE + 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    dw3000_hal_default_config(&config);
    config.aes.dma.src_addr  = DW3000_AES_SCRATCH_RAM_SIZE;
    config.aes.dma.pyld_size = 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_SIZE,
        dw3000_hal_aes_validate_transfer(&config.aes)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_aes_validate_transfer(NULL)
    );
}

void dw3000_hal_pure_run_aes_tests(void) {
    RUN_TEST(test_aes_size_helpers_and_status_errors);
    RUN_TEST(test_aes_dma_validation_rejects_invalid_fields);
    RUN_TEST(test_aes_config_validation_rejects_invalid_and_unsupported_modes);
    RUN_TEST(test_aes_transfer_validation_checks_port_ranges_and_tag_size);
}
