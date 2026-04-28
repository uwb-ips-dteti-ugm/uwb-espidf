#ifndef DW3000_TYPES_OTP_H
#define DW3000_TYPES_OTP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t address;
    uint32_t value;
} dw3000_otp_word_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_OTP_H */
