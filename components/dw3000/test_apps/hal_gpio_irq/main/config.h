#ifndef DW3000_HAL_GPIO_IRQ_CONFIG_H
#define DW3000_HAL_GPIO_IRQ_CONFIG_H

#include <stdbool.h>

#include "hal/spi_types.h"
#include "soc/gpio_num.h"

#define DW3000_HAL_GPIO_IRQ_SPI_HOST              SPI3_HOST
#define DW3000_HAL_GPIO_IRQ_SPI_CLOCK_HZ          6000000
#define DW3000_HAL_GPIO_IRQ_PIN_SCK               GPIO_NUM_18
#define DW3000_HAL_GPIO_IRQ_PIN_MISO              GPIO_NUM_19
#define DW3000_HAL_GPIO_IRQ_PIN_MOSI              GPIO_NUM_23
#define DW3000_HAL_GPIO_IRQ_PIN_CS                GPIO_NUM_4
#define DW3000_HAL_GPIO_IRQ_PIN_RST               GPIO_NUM_27
#define DW3000_HAL_GPIO_IRQ_PIN_IRQ               GPIO_NUM_34
#define DW3000_HAL_GPIO_IRQ_CONFIGURE_RST         true
#define DW3000_HAL_GPIO_IRQ_CONFIGURE_IRQ         true
#define DW3000_HAL_GPIO_IRQ_LOAD_OTP_CALIBRATION  true
#define DW3000_HAL_GPIO_IRQ_ENTER_IDLE_PLL        true
#define DW3000_HAL_GPIO_IRQ_SPI_MAX_TRANSFER_SIZE 1030U

#endif /* DW3000_HAL_GPIO_IRQ_CONFIG_H */
