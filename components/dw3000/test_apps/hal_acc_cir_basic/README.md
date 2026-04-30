# DW3000 HAL ACC/CIR Basic App

This ESP-IDF app proves ACC/CIR reads on actual hardware. Flash and monitor
this app on one board first, then run `hal_tx_basic` on a second board. It
uses the same MakerFabs-compatible RX profile as `hal_rx_basic`, waits for a
known frame, verifies `CIADONE`, reads IP diagnostics, then reads and summarizes
a small Ipatov CIR window from `ACC_MEM`.

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
cd components/dw3000/test_apps/hal_acc_cir_basic
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI timing, expected frame addresses, the
RX wait timeout, or the CIR window size. The expected success marker is
`DW3000_HAL_ACC_CIR_BASIC_PASS`.
