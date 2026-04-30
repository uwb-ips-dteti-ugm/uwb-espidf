#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_hal/core.h"
#include "dw3000_hal/gpio.h"
#include "dw3000_types/gpio.h"
#include "unity.h"

static void test_gpio_validation_accepts_default_and_rejects_out_of_range(void) {
    dw3000_device_config_t config;
    dw3000_gpio_mode_t     mode = {0};
    dw3000_gpio_irq_cfg_t  irq  = {0};

    dw3000_hal_default_config(&config);

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_config(&config.gpio)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_pin_mask(0U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_pin_mask(DW3000_GPIO_PIN_MASK)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_pin_mask((dw3000_gpio_pin_t)0x0200U)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_pin_index(0U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_pin_index(DW3000_GPIO_PIN_COUNT - 1U)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_pin_index(DW3000_GPIO_PIN_COUNT)
    );

    for (uint8_t i = 0U; i < DW3000_GPIO_PIN_COUNT; ++i) {
        mode.msgp[i] = DW3000_GPIO_FUNCTION_GPIO;
    }
    mode.msgp[0] = DW3000_GPIO0_FUNCTION_RXOKLED;
    mode.msgp[3] = DW3000_GPIO3_FUNCTION_TXLED;
    mode.msgp[4] = DW3000_GPIO4_FUNCTION_EXTPA;
    mode.msgp[5] = DW3000_GPIO5_FUNCTION_EXTTXE;
    mode.msgp[6] = DW3000_GPIO6_FUNCTION_EXTRXE;

    TEST_ASSERT_EQUAL(DW3000_ERROR_OK, dw3000_hal_gpio_validate_mode(&mode));

    mode.msgp[2] = (dw3000_gpio_function_t)(DW3000_GPIO_FUNCTION_MASK + 1U);
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_mode(&mode)
    );

    irq.enable     = (dw3000_gpio_pin_t)(DW3000_GPIO_PIN_0 | DW3000_GPIO_PIN_4);
    irq.sense_low  = DW3000_GPIO_PIN_0;
    irq.edge_mode  = irq.enable;
    irq.both_edges = DW3000_GPIO_PIN_4;
    irq.debounce   = DW3000_GPIO_PIN_0;

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_OK,
        dw3000_hal_gpio_validate_irq_cfg(&irq)
    );

    irq.debounce = (dw3000_gpio_pin_t)0x0200U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_irq_cfg(&irq)
    );

    config.gpio.output = (dw3000_gpio_pin_t)0x0200U;
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_config(&config.gpio)
    );

    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_config(NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_mode(NULL)
    );
    TEST_ASSERT_EQUAL(
        DW3000_ERROR_INVALID_ARG,
        dw3000_hal_gpio_validate_irq_cfg(NULL)
    );
}

void dw3000_hal_pure_run_gpio_tests(void) {
    RUN_TEST(test_gpio_validation_accepts_default_and_rejects_out_of_range);
}
