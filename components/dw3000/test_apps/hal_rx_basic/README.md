# DW3000 HAL RX Basic App

This ESP-IDF app is the RX half of the two-node RX/TX hardware test. Flash
and monitor this app on one board first, then run `hal_tx_basic` on a second
board. It arms RX, waits for a known IEEE 802.15.4 short-address data frame,
reads `RX_FINFO`, reads the RX buffer, verifies the frame, and logs the RX
timestamp.

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
cd components/dw3000/test_apps/hal_rx_basic
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI timing, expected frame addresses, or
the RX wait timeout. The expected success marker is
`DW3000_HAL_RX_BASIC_PASS`.
