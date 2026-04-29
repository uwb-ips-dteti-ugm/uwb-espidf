# DW3000 HAL GPIO IRQ App

This ESP-IDF app is the next hardware step after `hal_aes_engine`. It validates
the active DW3000 IRQ pin path without requiring a second UWB node:

- initialize the device with the default HAL configuration
- clear pending status events and enable only AES_DONE/AES_ERR interrupts
- run one AES scratch-RAM encrypt operation to generate AES_DONE
- verify `SYS_STATUS.AES_DONE`, `SYS_STATUS.IRQS`, and the ESP32 IRQ GPIO
- clear the AES event and verify the IRQ GPIO deasserts

Default wiring matches MakerFabs ESP32 UWB DW3000:

| Signal | GPIO |
| ------ | ---- |
| SCK    | 18   |
| MISO   | 19   |
| MOSI   | 23   |
| CS     | 4    |
| RSTn   | 27   |
| IRQ    | 34   |

Build and flash:

```sh
cd components/dw3000/test_apps/hal_gpio_irq
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI host, SPI clock, or reset/IRQ GPIO
setup. The expected success marker is `DW3000_HAL_GPIO_IRQ_PASS`.

Remaining hardware apps should cover RX/TX between nodes and ACC/CIR
inspection after a received frame.
