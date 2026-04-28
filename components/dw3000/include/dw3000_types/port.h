#ifndef DW3000_TYPES_PORT_H
#define DW3000_TYPES_PORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef dw3000_err_t (*dw3000_spi_write_fn)(
    void* ctx,
    const uint8_t* header,
    size_t header_len,
    const void* data,
    size_t data_len
);

typedef dw3000_err_t (*dw3000_spi_read_fn)(
    void* ctx,
    const uint8_t* header,
    size_t header_len,
    void* data,
    size_t data_len
);

typedef void (*dw3000_reset_fn)(void* ctx, bool asserted);

typedef int (*dw3000_irq_read_fn)(void* ctx);

typedef void (*dw3000_delay_us_fn)(void* ctx, uint32_t delay_us);

typedef uint64_t (*dw3000_get_time_us_fn)(void* ctx);

typedef void (*dw3000_lock_fn)(void* ctx);

typedef void (*dw3000_unlock_fn)(void* ctx);

typedef struct {
    void* ctx;
    dw3000_spi_write_fn spi_write;
    dw3000_spi_read_fn spi_read;
    dw3000_reset_fn reset;
    dw3000_irq_read_fn irq_read;
    dw3000_delay_us_fn delay_us;
    dw3000_get_time_us_fn get_time_us;
    dw3000_lock_fn lock;
    dw3000_unlock_fn unlock;
} dw3000_port_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_PORT_H */
