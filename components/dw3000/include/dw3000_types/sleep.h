#ifndef DW3000_TYPES_SLEEP_H
#define DW3000_TYPES_SLEEP_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DW3000_SLEEP_MODE_SLEEP = 0,
    DW3000_SLEEP_MODE_DEEPSLEEP,
} dw3000_sleep_mode_t;

typedef struct {
    dw3000_sleep_mode_t mode;
    bool preserve_config;
    bool wake_on_spi;
    bool wake_on_wakeup_pin;
    uint32_t sleep_counter_ticks;
} dw3000_sleep_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_SLEEP_H */
