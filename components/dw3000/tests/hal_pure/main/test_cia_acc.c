#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/acc.h"
#include "dw3000_hal/cia.h"
#include "dw3000_hal/core.h"
#include "dw3000_types/acc.h"
#include "dw3000_types/cia.h"
#include "dw3000_types/phy.h"
#include "unity.h"

static void test_cia_validation_accepts_limits_and_rejects_invalid_fields(void) {
    dw3000_device_config_t config;

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    config.cia.conf_flags           = DW3000_CIA_CONF_MINDIAG;
    config.cia.fp_conf.fp_agreed_th = 0x7U;
    config.cia.fp_conf.tc_rcfg      = 0xFFU;
    config.cia.ip_conf.ntm          = 0x1FU;
    config.cia.ip_conf.pmult        = 0x3U;
    config.cia.ip_conf.rtm          = 0x1FU;
    config.cia.sts_conf.ntm         = 0x1FU;
    config.cia.sts_conf.pmult       = 0x3U;
    config.cia.sts_conf.rtm         = 0U;
    config.cia.sts_conf.mnth        = 0x7FU;
    config.cia.sts_conf.cq_en       = true;
    config.cia.adjust               = 0x3FFFU;

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.conf_flags = (dw3000_cia_conf_flags_t)(1UL << 21U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.fp_conf.fp_agreed_th = 0x8U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.ip_conf.ntm = 0x20U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.ip_conf.pmult = 0x4U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.ip_conf.rtm = 0x20U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.sts_conf.ntm = 0x20U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.sts_conf.pmult = 0x4U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.sts_conf.rtm = 1U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.sts_conf.mnth = 0x80U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    dw3000_hal_default_config(&config);
    config.cia.adjust = 0x4000U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(&config.cia)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_cia_validate_config(NULL)
    );
}

static void test_acc_sample_span_and_cir_spans(void) {
    dw3000_device_t device = {0};
    uint16_t        start_sample;
    size_t          sample_count;

    dw3000_hal_default_config(&device.config);

    TEST_ASSERT_TRUE(dw3000_hal_acc_sample_span_is_valid(0U, 0U));
    TEST_ASSERT_TRUE(dw3000_hal_acc_sample_span_is_valid(
        0U,
        DW3000_ACC_SAMPLE_COUNT
    ));
    TEST_ASSERT_TRUE(dw3000_hal_acc_sample_span_is_valid(
        DW3000_ACC_SAMPLE_COUNT - 1U,
        1U
    ));
    TEST_ASSERT_FALSE(dw3000_hal_acc_sample_span_is_valid(
        DW3000_ACC_SAMPLE_COUNT,
        0U
    ));
    TEST_ASSERT_FALSE(dw3000_hal_acc_sample_span_is_valid(
        DW3000_ACC_SAMPLE_COUNT - 1U,
        2U
    ));
    TEST_ASSERT_FALSE(dw3000_hal_acc_sample_span_is_valid(
        0U,
        DW3000_ACC_SAMPLE_COUNT + 1U
    ));

    TEST_ASSERT_EQUAL_UINT32(
        DW3000_ACC_IPATOV_64M_SAMPLES,
        (uint32_t)dw3000_hal_acc_ipatov_sample_count(NULL)
    );
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_ACC_IPATOV_64M_SAMPLES,
        (uint32_t)dw3000_hal_acc_ipatov_sample_count(&device)
    );

    device.config.phy.prf = DW3000_PHY_PRF_16_MHZ;
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_ACC_IPATOV_16M_SAMPLES,
        (uint32_t)dw3000_hal_acc_ipatov_sample_count(&device)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_acc_cir_span(
            &device,
            DW3000_ACC_CIR_IPATOV,
            &start_sample,
            &sample_count
        )
    );
    TEST_ASSERT_EQUAL_UINT16(DW3000_ACC_IPATOV_START_SAMPLE, start_sample);
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_ACC_IPATOV_16M_SAMPLES,
        (uint32_t)sample_count
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_acc_cir_span(
            &device,
            DW3000_ACC_CIR_STS0,
            &start_sample,
            &sample_count
        )
    );
    TEST_ASSERT_EQUAL_UINT16(DW3000_ACC_STS0_START_SAMPLE, start_sample);
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_ACC_STS_SAMPLES,
        (uint32_t)sample_count
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_acc_cir_span(
            &device,
            DW3000_ACC_CIR_STS1,
            &start_sample,
            &sample_count
        )
    );
    TEST_ASSERT_EQUAL_UINT16(DW3000_ACC_STS1_START_SAMPLE, start_sample);
    TEST_ASSERT_EQUAL_UINT32(
        DW3000_ACC_STS_SAMPLES,
        (uint32_t)sample_count
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_acc_cir_span(
            &device,
            (dw3000_acc_cir_t)3U,
            &start_sample,
            &sample_count
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_acc_cir_span(
            &device,
            DW3000_ACC_CIR_IPATOV,
            NULL,
            &sample_count
        )
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_acc_cir_span(
            &device,
            DW3000_ACC_CIR_IPATOV,
            &start_sample,
            NULL
        )
    );
}

static void test_acc_i24_and_sample_decode(void) {
    const uint8_t zero[3]                            = {0x00U, 0x00U, 0x00U};
    const uint8_t max_pos[3]                         = {0xFFU, 0xFFU, 0x7FU};
    const uint8_t min_neg[3]                         = {0x00U, 0x00U, 0x80U};
    const uint8_t minus_one[3]                       = {0xFFU, 0xFFU, 0xFFU};
    const uint8_t raw_sample[DW3000_ACC_SAMPLE_SIZE] = {
        0x01U,
        0x00U,
        0x00U,
        0xFEU,
        0xFFU,
        0xFFU,
    };
    dw3000_acc_sample_t sample;

    TEST_ASSERT_EQUAL_INT32(0, dw3000_hal_acc_decode_i24(NULL));
    TEST_ASSERT_EQUAL_INT32(0, dw3000_hal_acc_decode_i24(zero));
    TEST_ASSERT_EQUAL_INT32(8388607, dw3000_hal_acc_decode_i24(max_pos));
    TEST_ASSERT_EQUAL_INT32(-8388608, dw3000_hal_acc_decode_i24(min_neg));
    TEST_ASSERT_EQUAL_INT32(-1, dw3000_hal_acc_decode_i24(minus_one));

    sample = dw3000_hal_acc_decode_sample(NULL);
    TEST_ASSERT_EQUAL_INT32(0, sample.real);
    TEST_ASSERT_EQUAL_INT32(0, sample.imag);

    sample = dw3000_hal_acc_decode_sample(raw_sample);
    TEST_ASSERT_EQUAL_INT32(1, sample.real);
    TEST_ASSERT_EQUAL_INT32(-2, sample.imag);
}

void dw3000_hal_pure_run_cia_acc_tests(void) {
    RUN_TEST(test_cia_validation_accepts_limits_and_rejects_invalid_fields);
    RUN_TEST(test_acc_sample_span_and_cir_spans);
    RUN_TEST(test_acc_i24_and_sample_decode);
}
