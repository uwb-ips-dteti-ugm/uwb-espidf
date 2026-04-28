#ifndef DW3000_TYPES_ADDRESS_H
#define DW3000_TYPES_ADDRESS_H

#include <stdint.h>

#include "core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t pan_id;
    uint16_t short_address;
    dw3000_eui_t eui;
} dw3000_address_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_ADDRESS_H */
