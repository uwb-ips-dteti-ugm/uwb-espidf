# DW3000 HAL AES Engine App

This ESP-IDF app is the next hardware step after `hal_default_init`. It uses
the DW3000 AES-DMA engine with scratch RAM and AES key RAM:

- initialize the device with the default HAL configuration
- write/read back one AES key RAM slot
- write a plaintext payload into scratch RAM
- encrypt scratch RAM to another scratch RAM region using AES-GCM with no tag
- decrypt the ciphertext into a third scratch RAM region
- verify the ciphertext changed and the recovered payload matches the original

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
cd components/dw3000/test_apps/hal_aes_engine
idf.py set-target esp32
idf.py build flash monitor
```

Edit `main/config.h` to change pins, SPI host, SPI clock, or reset/IRQ GPIO
setup. The expected success marker is `DW3000_HAL_AES_ENGINE_PASS`.

Remaining hardware apps should cover active GPIO/IRQ behavior, RX/TX between
nodes, and ACC/CIR inspection after a received frame.
