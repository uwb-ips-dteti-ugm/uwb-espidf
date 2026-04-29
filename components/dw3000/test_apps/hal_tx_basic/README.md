# DW3000 HAL TX Basic App

This ESP-IDF app is the TX half of the two-node RX/TX hardware test. Flash
`hal_rx_basic` on a second board first, then flash or reset this app on the TX
board. It transmits a known IEEE 802.15.4 short-address data frame repeatedly
and verifies each TX completion through `SYS_STATUS.TXFRS`.

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
cd components/dw3000/test_apps/hal_tx_basic
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI timing, frame addresses, frame count,
or TX interval. The expected success marker is `DW3000_HAL_TX_BASIC_PASS`.
