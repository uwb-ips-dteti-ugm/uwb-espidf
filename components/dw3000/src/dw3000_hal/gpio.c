#include "dw3000_hal/gpio.h"

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_hal/pmsc.h"
#include "dw3000_register.h"

#define DW3000_HAL_GPIO_MODE_FIELD_BITS 3U
#define DW3000_HAL_GPIO_MODE_MASK       0x07U
#define DW3000_HAL_GPIO_REG_MASK        ((uint16_t)DW3000_GPIO_PIN_MASK)
#define DW3000_HAL_GPIO_REG_MASK_U32    ((uint32_t)DW3000_GPIO_PIN_MASK)
#define DW3000_HAL_GPIO_LED_PINS \
    ((uint16_t)DW3000_GPIO_PIN_0 | \
     (uint16_t)DW3000_GPIO_PIN_1 | \
     (uint16_t)DW3000_GPIO_PIN_2 | \
     (uint16_t)DW3000_GPIO_PIN_3)

static bool dw3000_hal_gpio_is_idle(const dw3000_device_t* device) {
    return (device->state_flags & (DW3000_DEVICE_STATE_RX_ON |
                                   DW3000_DEVICE_STATE_TX_PENDING |
                                   DW3000_DEVICE_STATE_SLEEPING)) == 0U;
}

static uint32_t dw3000_hal_gpio_encode_mode(
    const dw3000_gpio_mode_t* mode
) {
    uint32_t raw = 0U;

    for (uint8_t i = 0U; i < DW3000_GPIO_PIN_COUNT; ++i) {
        raw |= ((uint32_t)mode->msgp[i] & DW3000_HAL_GPIO_MODE_MASK) <<
               (i * DW3000_HAL_GPIO_MODE_FIELD_BITS);
    }

    return raw;
}

static void dw3000_hal_gpio_decode_mode(
    uint32_t               raw,
    dw3000_gpio_mode_t*    mode
) {
    for (uint8_t i = 0U; i < DW3000_GPIO_PIN_COUNT; ++i) {
        mode->msgp[i] = (dw3000_gpio_function_t)(
            (raw >> (i * DW3000_HAL_GPIO_MODE_FIELD_BITS)) &
            DW3000_HAL_GPIO_MODE_MASK
        );
    }
}

static bool dw3000_hal_gpio_irq_uses_debounce(
    const dw3000_gpio_irq_cfg_t* irq
) {
    return (((uint16_t)irq->enable & (uint16_t)irq->debounce &
             DW3000_HAL_GPIO_REG_MASK) != 0U);
}

static dw3000_error_t dw3000_hal_gpio_read_pin_reg(
    dw3000_device_t*    device,
    dw3000_reg_desc_t   reg,
    dw3000_gpio_pin_t*  pins
) {
    dw3000_error_t err;
    uint16_t       raw;

    if ((device == NULL) || (pins == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u16(device, reg, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    *pins = (dw3000_gpio_pin_t)(raw & DW3000_HAL_GPIO_REG_MASK);
    return DW3000_ERROR_OK;
}

static dw3000_error_t dw3000_hal_gpio_write_pin_reg(
    dw3000_device_t*    device,
    dw3000_reg_desc_t   reg,
    dw3000_gpio_pin_t   pins
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_validate_pin_mask(pins);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (!dw3000_hal_gpio_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    return dw3000_reg_write_u16(
        device,
        reg,
        (uint16_t)pins & DW3000_HAL_GPIO_REG_MASK
    );
}

dw3000_error_t dw3000_hal_gpio_validate_pin_mask(dw3000_gpio_pin_t pins) {
    if (((uint16_t)pins & ~DW3000_HAL_GPIO_REG_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_validate_pin_index(
    dw3000_gpio_pin_index_t pin
) {
    return (pin < DW3000_GPIO_PIN_COUNT) ?
        DW3000_ERROR_OK :
        DW3000_ERROR_INVALID_ARG;
}

dw3000_error_t dw3000_hal_gpio_validate_mode(
    const dw3000_gpio_mode_t* mode
) {
    if (mode == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    for (uint8_t i = 0U; i < DW3000_GPIO_PIN_COUNT; ++i) {
        if ((mode->msgp[i] & ~DW3000_GPIO_FUNCTION_MASK) != 0U) {
            return DW3000_ERROR_INVALID_ARG;
        }
    }

    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_validate_irq_cfg(
    const dw3000_gpio_irq_cfg_t* irq
) {
    dw3000_error_t err;

    if (irq == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_validate_pin_mask(irq->enable);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_validate_pin_mask(irq->sense_low);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_validate_pin_mask(irq->edge_mode);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_validate_pin_mask(irq->both_edges);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_gpio_validate_pin_mask(irq->debounce);
}

dw3000_error_t dw3000_hal_gpio_validate_config(
    const dw3000_gpio_config_t* config
) {
    dw3000_error_t err;

    if (config == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_validate_mode(&config->mode);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_validate_pin_mask(config->pull_enable);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_validate_pin_mask(config->input);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_validate_pin_mask(config->output);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_gpio_validate_irq_cfg(&config->irq);
}

dw3000_error_t dw3000_hal_gpio_enable_clocks(
    dw3000_device_t* device,
    bool             enable_debounce_clock
) {
    dw3000_pmsc_clk_flags_t flags = (dw3000_pmsc_clk_flags_t)(
        DW3000_PMSC_CLK_GPIO_CLK_EN |
        DW3000_PMSC_CLK_GPIO_DCLK_EN |
        DW3000_PMSC_CLK_GPIO_DRST_N
    );

    if (enable_debounce_clock) {
        flags = (dw3000_pmsc_clk_flags_t)(flags | DW3000_PMSC_CLK_LP_CLK_EN);
    }

    return dw3000_hal_pmsc_set_clock_flags(device, flags);
}

dw3000_error_t dw3000_hal_gpio_disable_clocks(
    dw3000_device_t* device,
    bool             disable_debounce_clock
) {
    dw3000_pmsc_clk_flags_t flags = (dw3000_pmsc_clk_flags_t)(
        DW3000_PMSC_CLK_GPIO_CLK_EN |
        DW3000_PMSC_CLK_GPIO_DCLK_EN |
        DW3000_PMSC_CLK_GPIO_DRST_N
    );

    if (disable_debounce_clock) {
        flags = (dw3000_pmsc_clk_flags_t)(flags | DW3000_PMSC_CLK_LP_CLK_EN);
    }

    return dw3000_hal_pmsc_clear_clock_flags(device, flags);
}

dw3000_error_t dw3000_hal_gpio_read_mode(
    dw3000_device_t*      device,
    dw3000_gpio_mode_t*   mode
) {
    dw3000_error_t err;
    uint32_t       raw;

    if ((device == NULL) || (mode == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_reg_read_u32(device, DW3000_REG_GPIO_MODE, &raw);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    dw3000_hal_gpio_decode_mode(raw, mode);
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_configure_mode(
    dw3000_device_t*           device,
    const dw3000_gpio_mode_t*  mode
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_gpio_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (mode == NULL) {
        mode = &device->config.gpio.mode;
    }

    err = dw3000_hal_gpio_validate_mode(mode);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_GPIO_MODE,
        dw3000_hal_gpio_encode_mode(mode)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.gpio.mode = *mode;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_set_pin_function(
    dw3000_device_t*           device,
    dw3000_gpio_pin_index_t    pin,
    dw3000_gpio_function_t     function
) {
    dw3000_gpio_mode_t mode;
    dw3000_error_t     err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_validate_pin_index(pin);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if ((function & ~DW3000_GPIO_FUNCTION_MASK) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    mode = device->config.gpio.mode;
    mode.msgp[pin] = function;
    return dw3000_hal_gpio_configure_mode(device, &mode);
}

dw3000_error_t dw3000_hal_gpio_read_pull_enable(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t*  pins
) {
    return dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_PULL_EN, pins);
}

dw3000_error_t dw3000_hal_gpio_set_pull_enable(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   pins
) {
    dw3000_error_t err = dw3000_hal_gpio_write_pin_reg(
        device,
        DW3000_REG_GPIO_PULL_EN,
        pins
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.gpio.pull_enable = pins;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_read_direction(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t*  input_pins
) {
    return dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_DIR, input_pins);
}

dw3000_error_t dw3000_hal_gpio_set_direction(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   input_pins
) {
    dw3000_error_t err = dw3000_hal_gpio_write_pin_reg(
        device,
        DW3000_REG_GPIO_DIR,
        input_pins
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.gpio.input = input_pins;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_read_output(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t*  output_pins
) {
    return dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_OUT, output_pins);
}

dw3000_error_t dw3000_hal_gpio_set_output(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   output_pins
) {
    dw3000_error_t err = dw3000_hal_gpio_write_pin_reg(
        device,
        DW3000_REG_GPIO_OUT,
        output_pins
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.gpio.output = output_pins;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_set_pins(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   pins
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_gpio_set_output(
        device,
        (dw3000_gpio_pin_t)(device->config.gpio.output | pins)
    );
}

dw3000_error_t dw3000_hal_gpio_clear_pins(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   pins
) {
    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    return dw3000_hal_gpio_set_output(
        device,
        (dw3000_gpio_pin_t)(device->config.gpio.output & ~pins)
    );
}

dw3000_error_t dw3000_hal_gpio_read_raw(
    dw3000_device_t*    device,
    dw3000_gpio_raw_t*  raw
) {
    return dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_RAW, raw);
}

dw3000_error_t dw3000_hal_gpio_read_irq_status(
    dw3000_device_t*             device,
    dw3000_gpio_irq_status_t*    status
) {
    return dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_ISTS, status);
}

dw3000_error_t dw3000_hal_gpio_clear_irq_status(
    dw3000_device_t*             device,
    dw3000_gpio_irq_status_t     status
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_validate_pin_mask(status);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (((uint16_t)status & DW3000_HAL_GPIO_REG_MASK) == 0U) {
        return DW3000_ERROR_OK;
    }

    return dw3000_reg_write_u32(
        device,
        DW3000_REG_GPIO_ICLR,
        (uint32_t)status & DW3000_HAL_GPIO_REG_MASK_U32
    );
}

dw3000_error_t dw3000_hal_gpio_read_irq_cfg(
    dw3000_device_t*        device,
    dw3000_gpio_irq_cfg_t*  irq
) {
    dw3000_error_t err;

    if ((device == NULL) || (irq == NULL)) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_IRQE, &irq->enable);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_ISEN, &irq->sense_low);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_IMODE, &irq->edge_mode);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_IBES, &irq->both_edges);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    return dw3000_hal_gpio_read_pin_reg(device, DW3000_REG_GPIO_IDBE, &irq->debounce);
}

dw3000_error_t dw3000_hal_gpio_configure_irq(
    dw3000_device_t*              device,
    const dw3000_gpio_irq_cfg_t*  irq
) {
    dw3000_error_t err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    if (!dw3000_hal_gpio_is_idle(device)) {
        return DW3000_ERROR_BUSY;
    }

    if (irq == NULL) {
        irq = &device->config.gpio.irq;
    }

    err = dw3000_hal_gpio_validate_irq_cfg(irq);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (dw3000_hal_gpio_irq_uses_debounce(irq)) {
        err = dw3000_hal_gpio_enable_clocks(device, true);
        if (err != DW3000_ERROR_OK) {
            return err;
        }
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_GPIO_ISEN,
        (uint16_t)irq->sense_low & DW3000_HAL_GPIO_REG_MASK
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_GPIO_IMODE,
        (uint16_t)irq->edge_mode & DW3000_HAL_GPIO_REG_MASK
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_GPIO_IBES,
        (uint16_t)irq->both_edges & DW3000_HAL_GPIO_REG_MASK
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u32(
        device,
        DW3000_REG_GPIO_IDBE,
        (uint32_t)irq->debounce & DW3000_HAL_GPIO_REG_MASK_U32
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_reg_write_u16(
        device,
        DW3000_REG_GPIO_IRQE,
        (uint16_t)irq->enable & DW3000_HAL_GPIO_REG_MASK
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.gpio.irq = *irq;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_configure(
    dw3000_device_t*             device,
    const dw3000_gpio_config_t*  config
) {
    dw3000_error_t        err;
    dw3000_gpio_config_t  actual;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    actual = (config != NULL) ? *config : device->config.gpio;

    err = dw3000_hal_gpio_validate_config(&actual);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_enable_clocks(
        device,
        dw3000_hal_gpio_irq_uses_debounce(&actual.irq)
    );
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_configure_mode(device, &actual.mode);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_set_pull_enable(device, actual.pull_enable);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_set_output(device, actual.output);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_set_direction(device, actual.input);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    err = dw3000_hal_gpio_configure_irq(device, &actual.irq);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    device->config.gpio = actual;
    return DW3000_ERROR_OK;
}

dw3000_error_t dw3000_hal_gpio_configure_current(dw3000_device_t* device) {
    return dw3000_hal_gpio_configure(device, NULL);
}

dw3000_error_t dw3000_hal_gpio_configure_led_functions(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   led_pins
) {
    dw3000_gpio_mode_t mode;
    dw3000_error_t     err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_validate_pin_mask(led_pins);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    if (((uint16_t)led_pins & ~DW3000_HAL_GPIO_LED_PINS) != 0U) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_enable_clocks(device, false);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    mode = device->config.gpio.mode;
    if (((uint16_t)led_pins & (uint16_t)DW3000_GPIO_PIN_0) != 0U) {
        mode.msgp[0] = DW3000_GPIO0_FUNCTION_RXOKLED;
    }
    if (((uint16_t)led_pins & (uint16_t)DW3000_GPIO_PIN_1) != 0U) {
        mode.msgp[1] = DW3000_GPIO1_FUNCTION_SFDLED;
    }
    if (((uint16_t)led_pins & (uint16_t)DW3000_GPIO_PIN_2) != 0U) {
        mode.msgp[2] = DW3000_GPIO2_FUNCTION_RXLED;
    }
    if (((uint16_t)led_pins & (uint16_t)DW3000_GPIO_PIN_3) != 0U) {
        mode.msgp[3] = DW3000_GPIO3_FUNCTION_TXLED;
    }

    return dw3000_hal_gpio_configure_mode(device, &mode);
}

dw3000_error_t dw3000_hal_gpio_configure_external_pa_lna(
    dw3000_device_t* device,
    bool             enable_extpa,
    bool             enable_exttxe,
    bool             enable_extrxe
) {
    dw3000_gpio_mode_t mode;
    dw3000_error_t     err;

    if (device == NULL) {
        return DW3000_ERROR_INVALID_ARG;
    }

    err = dw3000_hal_gpio_enable_clocks(device, false);
    if (err != DW3000_ERROR_OK) {
        return err;
    }

    mode = device->config.gpio.mode;
    mode.msgp[4] = enable_extpa ? DW3000_GPIO4_FUNCTION_EXTPA :
        DW3000_GPIO_FUNCTION_GPIO;
    mode.msgp[5] = enable_exttxe ? DW3000_GPIO5_FUNCTION_EXTTXE :
        DW3000_GPIO_FUNCTION_GPIO;
    mode.msgp[6] = enable_extrxe ? DW3000_GPIO6_FUNCTION_EXTRXE :
        DW3000_GPIO_FUNCTION_GPIO;

    return dw3000_hal_gpio_configure_mode(device, &mode);
}
