# DW3000 HAL Basic Info App

This ESP-IDF app is a hardware smoke test for the DW3000 HAL. It does not
use Unity; it brings up the chip, reads informative registers, logs the
values, and prints `DW3000_HAL_BASIC_INFO_PASS` or `DW3000_HAL_BASIC_INFO_FAIL`.

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
cd components/dw3000/test_apps/hal_basic_info
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI host, SPI clock, or reset/IRQ GPIO
setup.

Current coverage:

- ESP-IDF DW3000 port allocation/configuration
- Core HAL bring-up and DEV_ID validation
- Informative reads from SYS_STATUS, SYS_ENABLE, EUI, PANADR, SYS_CFG,
  SYS_TIME, SYS_STATE, and FINT_STAT

Later hardware apps can cover full initialization, PHY/MAC/STS/CIA readback,
GPIO, AES, RX/TX, and ACC/CIR. Unity-based cases can live separately under a
tests folder.
