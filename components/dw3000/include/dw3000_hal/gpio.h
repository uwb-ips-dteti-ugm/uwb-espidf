#ifndef DW3000_HAL_GPIO_H
#define DW3000_HAL_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_error_t dw3000_hal_gpio_validate_pin_mask(dw3000_gpio_pin_t pins);

dw3000_error_t dw3000_hal_gpio_validate_pin_index(
    dw3000_gpio_pin_index_t pin
);

dw3000_error_t dw3000_hal_gpio_validate_mode(
    const dw3000_gpio_mode_t* mode
);

dw3000_error_t dw3000_hal_gpio_validate_irq_cfg(
    const dw3000_gpio_irq_cfg_t* irq
);

dw3000_error_t dw3000_hal_gpio_validate_config(
    const dw3000_gpio_config_t* config
);

dw3000_error_t dw3000_hal_gpio_enable_clocks(
    dw3000_device_t* device,
    bool             enable_debounce_clock
);

dw3000_error_t dw3000_hal_gpio_disable_clocks(
    dw3000_device_t* device,
    bool             disable_debounce_clock
);

dw3000_error_t dw3000_hal_gpio_read_mode(
    dw3000_device_t*      device,
    dw3000_gpio_mode_t*   mode
);

dw3000_error_t dw3000_hal_gpio_configure_mode(
    dw3000_device_t*            device,
    const dw3000_gpio_mode_t*   mode
);

dw3000_error_t dw3000_hal_gpio_set_pin_function(
    dw3000_device_t*           device,
    dw3000_gpio_pin_index_t    pin,
    dw3000_gpio_function_t     function
);

dw3000_error_t dw3000_hal_gpio_read_pull_enable(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t*  pins
);

dw3000_error_t dw3000_hal_gpio_set_pull_enable(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   pins
);

dw3000_error_t dw3000_hal_gpio_read_direction(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t*  input_pins
);

dw3000_error_t dw3000_hal_gpio_set_direction(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   input_pins
);

dw3000_error_t dw3000_hal_gpio_read_output(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t*  output_pins
);

dw3000_error_t dw3000_hal_gpio_set_output(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   output_pins
);

dw3000_error_t dw3000_hal_gpio_set_pins(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   pins
);

dw3000_error_t dw3000_hal_gpio_clear_pins(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   pins
);

dw3000_error_t dw3000_hal_gpio_read_raw(
    dw3000_device_t*    device,
    dw3000_gpio_raw_t*  raw
);

dw3000_error_t dw3000_hal_gpio_read_irq_status(
    dw3000_device_t*              device,
    dw3000_gpio_irq_status_t*     status
);

dw3000_error_t dw3000_hal_gpio_clear_irq_status(
    dw3000_device_t*             device,
    dw3000_gpio_irq_status_t     status
);

dw3000_error_t dw3000_hal_gpio_read_irq_cfg(
    dw3000_device_t*         device,
    dw3000_gpio_irq_cfg_t*   irq
);

dw3000_error_t dw3000_hal_gpio_configure_irq(
    dw3000_device_t*               device,
    const dw3000_gpio_irq_cfg_t*    irq
);

dw3000_error_t dw3000_hal_gpio_configure(
    dw3000_device_t*              device,
    const dw3000_gpio_config_t*   config
);

dw3000_error_t dw3000_hal_gpio_configure_current(dw3000_device_t* device);

dw3000_error_t dw3000_hal_gpio_configure_led_functions(
    dw3000_device_t*    device,
    dw3000_gpio_pin_t   led_pins
);

dw3000_error_t dw3000_hal_gpio_configure_external_pa_lna(
    dw3000_device_t* device,
    bool             enable_extpa,
    bool             enable_exttxe,
    bool             enable_extrxe
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_GPIO_H */
