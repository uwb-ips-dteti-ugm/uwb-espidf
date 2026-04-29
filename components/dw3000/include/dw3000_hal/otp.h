#ifndef DW3000_HAL_OTP_H
#define DW3000_HAL_OTP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/otp.h"
#include "dw3000_types/phy.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_HAL_OTP_PROGRAM_DEFAULT_TIMEOUT_US 10000U

typedef struct {
    bool     allow_non_customer_addr;
    bool     allow_non_empty_word;
    bool     require_vpp_ok;
    uint32_t timeout_us;
} dw3000_hal_otp_program_options_t;

void dw3000_hal_otp_default_program_options(
    dw3000_hal_otp_program_options_t* options
);

bool dw3000_hal_otp_addr_is_valid(dw3000_otp_addr_t addr);

bool dw3000_hal_otp_addr_is_customer(dw3000_otp_addr_t addr);

bool dw3000_hal_otp_ops_sel_is_valid(dw3000_otp_ops_sel_t ops_sel);

dw3000_otp_ops_sel_t dw3000_hal_otp_ops_sel_for_preamble(
    dw3000_phy_preamble_length_t preamble_length
);

dw3000_error_t dw3000_hal_otp_read_config(
    dw3000_device_t*          device,
    dw3000_otp_cfg_flags_t*   flags,
    dw3000_otp_ops_sel_t*     ops_sel,
    dw3000_phy_channel_t*     dgc_channel
);

dw3000_error_t dw3000_hal_otp_read_status(
    dw3000_device_t*      device,
    dw3000_otp_status_t*  status
);

dw3000_error_t dw3000_hal_otp_read_word(
    dw3000_device_t*    device,
    dw3000_otp_addr_t   addr,
    dw3000_otp_word_t*  word
);

dw3000_error_t dw3000_hal_otp_read_words(
    dw3000_device_t*    device,
    dw3000_otp_addr_t   start_addr,
    dw3000_otp_word_t*  words,
    size_t              word_count
);

dw3000_error_t dw3000_hal_otp_read_special_register(
    dw3000_device_t*    device,
    dw3000_otp_word_t*  word
);

dw3000_error_t dw3000_hal_otp_kick(
    dw3000_device_t*         device,
    dw3000_otp_cfg_flags_t   kicks,
    dw3000_phy_channel_t     dgc_channel,
    dw3000_otp_ops_sel_t     ops_sel
);

dw3000_error_t dw3000_hal_otp_kick_dgc(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel
);

dw3000_error_t dw3000_hal_otp_kick_ldo(dw3000_device_t* device);

dw3000_error_t dw3000_hal_otp_kick_bias(dw3000_device_t* device);

dw3000_error_t dw3000_hal_otp_kick_ops(
    dw3000_device_t*      device,
    dw3000_otp_ops_sel_t  ops_sel
);

dw3000_error_t dw3000_hal_otp_kick_factory_calibration(
    dw3000_device_t*      device,
    dw3000_phy_channel_t  channel,
    dw3000_otp_ops_sel_t  ops_sel
);

dw3000_error_t dw3000_hal_otp_wait_program_done(
    dw3000_device_t* device,
    uint32_t         timeout_us
);

/* Program a word with conservative defaults unless options are supplied:
   customer-address range only, empty cell only, and VPP_OK required. OTP
   programming is irreversible; use allow_* options only for manufacturing
   flows that have already validated the target cell map. */
dw3000_error_t dw3000_hal_otp_program_word(
    dw3000_device_t*                            device,
    dw3000_otp_addr_t                           addr,
    dw3000_otp_word_t                           word,
    const dw3000_hal_otp_program_options_t*     options
);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_OTP_H */
