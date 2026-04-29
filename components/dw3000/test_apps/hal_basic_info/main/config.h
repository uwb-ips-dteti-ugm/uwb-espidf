#ifndef DW3000_HAL_BASIC_INFO_CONFIG_H
#define DW3000_HAL_BASIC_INFO_CONFIG_H

#include <stdbool.h>

#include "hal/spi_types.h"
#include "soc/gpio_num.h"

static const spi_host_device_t _spi_stub  = SPI3_HOST;
static const gpio_num_t        _gpio_stub = GPIO_NUM_18;

#define DW3000_HAL_BASIC_INFO_SPI_HOST              SPI3_HOST
#define DW3000_HAL_BASIC_INFO_SPI_CLOCK_HZ          16000000
#define DW3000_HAL_BASIC_INFO_PIN_SCK               GPIO_NUM_18
#define DW3000_HAL_BASIC_INFO_PIN_MISO              GPIO_NUM_19
#define DW3000_HAL_BASIC_INFO_PIN_MOSI              GPIO_NUM_23
#define DW3000_HAL_BASIC_INFO_PIN_CS                GPIO_NUM_4
#define DW3000_HAL_BASIC_INFO_PIN_RST               GPIO_NUM_27
#define DW3000_HAL_BASIC_INFO_PIN_IRQ               GPIO_NUM_34
#define DW3000_HAL_BASIC_INFO_CONFIGURE_RST         true
#define DW3000_HAL_BASIC_INFO_CONFIGURE_IRQ         true
#define DW3000_HAL_BASIC_INFO_SPI_MAX_TRANSFER_SIZE 1030U

#endif /* DW3000_HAL_BASIC_INFO_CONFIG_H */
