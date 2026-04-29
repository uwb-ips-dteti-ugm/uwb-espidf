#ifndef DW3000_TYPES_GPIO_H
#define DW3000_TYPES_GPIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DW3000 exposes 9 GPIO pins (GPIO0..GPIO8). Almost every register in
   file 0x05 is a per-pin bitmap where bit N belongs to GPIO N. */
#define DW3000_GPIO_PIN_COUNT 9U

typedef enum {
    DW3000_GPIO_PIN_0 = 1U << 0,
    DW3000_GPIO_PIN_1 = 1U << 1,
    DW3000_GPIO_PIN_2 = 1U << 2,
    DW3000_GPIO_PIN_3 = 1U << 3,
    DW3000_GPIO_PIN_4 = 1U << 4,
    DW3000_GPIO_PIN_5 = 1U << 5,
    DW3000_GPIO_PIN_6 = 1U << 6,
    DW3000_GPIO_PIN_7 = 1U << 7,
    DW3000_GPIO_PIN_8 = 1U << 8,
} dw3000_gpio_pin_t;

/* GPIO_MODE (0x05:00). MSGP[i] is the 3-bit function selector for
   GPIO i. MSGP[i] = 0 routes to generic GPIO; values 1..7 are
   pin-specific peripheral aliases (RX/TX/SFD LEDs on pins 0..3,
   external PA/LNA control on pins 4..6, SPI/IRQ debug on 7..8).
   See DW3000 user manual Table 4 for the full per-pin map. */
typedef struct {
    uint8_t msgp[DW3000_GPIO_PIN_COUNT]; /* 3 bits each */
} dw3000_gpio_mode_t;

/* IRQ configuration spread across GPIO_IRQE (0x10), GPIO_ISEN (0x18),
   GPIO_IMODE (0x1C), GPIO_IBES (0x20), GPIO_IDBE (0x28). Each member
   is a per-pin bitmap; bit set = feature enabled for that pin.
     - sense_low:  ISEN bit set = active-low / falling-edge
     - edge_mode:  IMODE bit set = edge-triggered (else level)
     - both_edges: IBES bit set = trigger on both edges (overrides
                   sense_low for that pin)
     - debounce:   IDBE bit set = enable input debounce filter */
typedef struct {
    dw3000_gpio_pin_t enable;
    dw3000_gpio_pin_t sense_low;
    dw3000_gpio_pin_t edge_mode;
    dw3000_gpio_pin_t both_edges;
    dw3000_gpio_pin_t debounce;
} dw3000_gpio_irq_cfg_t;

/* Bundle. GPIO_DIR uses 1 = input; GPIO_PULL_EN uses 1 = pull enabled.
   Pull polarity follows direction: pulls are up on inputs, down on
   outputs. */
typedef struct {
    dw3000_gpio_mode_t    mode;
    dw3000_gpio_pin_t     pull_enable; /* GPIO_PULL_EN (0x05:04) */
    dw3000_gpio_pin_t     input;       /* GPIO_DIR     (0x05:08) */
    dw3000_gpio_pin_t     output;      /* GPIO_OUT     (0x05:0C) */
    dw3000_gpio_irq_cfg_t irq;
} dw3000_gpio_config_t;

/* GPIO_ISTS (0x05:14) — pending IRQ flags. Acknowledge by writing the
   same bits to GPIO_ICLR (0x05:24). */
typedef dw3000_gpio_pin_t dw3000_gpio_irq_status_t;

/* GPIO_RAW (0x05:2C) — raw input state regardless of direction. */
typedef dw3000_gpio_pin_t dw3000_gpio_raw_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_GPIO_H */
