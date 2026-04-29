# DW3000 HAL Default Init App

This ESP-IDF app is the next hardware step after `hal_basic_info`. It creates
the ESP-IDF port, runs `dw3000_hal_initialize()` with the HAL default device
configuration, checks expected device state flags, and reads back key
subsystem configuration through HAL helpers.

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
cd components/dw3000/test_apps/hal_default_init
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI host, SPI clock, or reset/IRQ GPIO
setup. The expected success marker is `DW3000_HAL_DEFAULT_INIT_PASS`.

Current coverage:

- ESP-IDF DW3000 port allocation/configuration
- `dw3000_hal_initialize()` with default HAL options
- Device state flags after default initialization
- Readback of status, MAC, STS, CIA, GPIO, AES, PMSC, and selected raw
  configuration registers

Remaining hardware apps should cover active GPIO/IRQ behavior, AES engine
operation, RX/TX, and ACC/CIR inspection after a received frame.
