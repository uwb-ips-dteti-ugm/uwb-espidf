# DW3000 HAL Pure Unit Tests

This ESP-IDF Unity app is the first hardware-free unit test example for the
DW3000 component. It runs pure HAL helper tests on the ESP32 target without a
DW3000 module connected.

Build, flash, and monitor:

```sh
cd components/dw3000/tests/hal_pure
idf.py set-target esp32
idf.py build flash monitor
```

The current examples validate `dw3000_hal_phy_sfd_timeout()` and the related
PHY helper functions, STS SYS_CFG validation and timestamp-quality helpers,
GPIO mask/mode/IRQ configuration validation, plus FCMD/TX/RX command
classification, TX frame validation, RX event masks, and AES validation
helpers, MAC validation helpers, PMSC/PLL validation helpers, and AON/OTP
helpers, plus core defaults and device-ID support. The expected Unity output
contains twenty passing tests:

```text
20 Tests 0 Failures 0 Ignored
OK
```

Use this app as the template for future pure unit tests. Tests that need a
mocked SPI/register transport can be added here later, while real hardware
bring-up checks stay under `test_apps`.
