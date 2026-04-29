#ifndef DW3000_HAL_PMSC_H
#define DW3000_HAL_PMSC_H

#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/pmsc.h"

#ifdef __cplusplus
extern "C" {
#endif

dw3000_error_t dw3000_hal_pmsc_validate_clock_ctrl(
    const dw3000_pmsc_clk_ctrl_t* clock
);

dw3000_error_t dw3000_hal_pmsc_validate_seq_flags(
    dw3000_pmsc_seq_ctrl_flags_t flags
);

dw3000_error_t dw3000_hal_pmsc_validate_config(
    const dw3000_pmsc_config_t* config
);

dw3000_error_t dw3000_hal_pmsc_read_clock_ctrl(
    dw3000_device_t*          device,
    dw3000_pmsc_clk_ctrl_t*   clock
);

dw3000_error_t dw3000_hal_pmsc_configure_clock_ctrl(
    dw3000_device_t*                device,
    const dw3000_pmsc_clk_ctrl_t*   clock
);

dw3000_error_t dw3000_hal_pmsc_set_clock_flags(
    dw3000_device_t*          device,
    dw3000_pmsc_clk_flags_t   flags
);

dw3000_error_t dw3000_hal_pmsc_clear_clock_flags(
    dw3000_device_t*          device,
    dw3000_pmsc_clk_flags_t   flags
);

dw3000_error_t dw3000_hal_pmsc_read_seq_ctrl(
    dw3000_device_t*                 device,
    dw3000_pmsc_seq_ctrl_flags_t*    flags
);

/* Persistent sequencer configuration. FORCE2INIT is a pulse control bit;
   use dw3000_hal_pmsc_force_idle_rc() rather than setting it here. */
dw3000_error_t dw3000_hal_pmsc_configure_seq_ctrl(
    dw3000_device_t*                device,
    dw3000_pmsc_seq_ctrl_flags_t    flags
);

dw3000_error_t dw3000_hal_pmsc_set_seq_flags(
    dw3000_device_t*                device,
    dw3000_pmsc_seq_ctrl_flags_t    flags
);

dw3000_error_t dw3000_hal_pmsc_clear_seq_flags(
    dw3000_device_t*                device,
    dw3000_pmsc_seq_ctrl_flags_t    flags
);

/* Set AINIT2IDLE and wait for CPLOCK. A zero timeout performs a single
   status check after requesting the transition. */
dw3000_error_t dw3000_hal_pmsc_enter_idle_pll(
    dw3000_device_t* device,
    uint32_t         timeout_us
);

dw3000_error_t dw3000_hal_pmsc_wait_for_pll_lock(
    dw3000_device_t* device,
    uint32_t         timeout_us
);

/* Force IDLE_PLL back to IDLE_RC using the documented FAST_RC/FORCE2INIT
   sequence, then restores SYS_CLK to AUTO. */
dw3000_error_t dw3000_hal_pmsc_force_idle_rc(dw3000_device_t* device);

/* Pulse one or more SOFT_RST bits low then high. */
dw3000_error_t dw3000_hal_pmsc_soft_reset_blocks(
    dw3000_device_t*             device,
    dw3000_pmsc_soft_rst_t       mask
);

dw3000_error_t dw3000_hal_pmsc_read_txfseq(
    dw3000_device_t*       device,
    dw3000_pmsc_txfseq_t*  value
);

dw3000_error_t dw3000_hal_pmsc_set_txfseq(
    dw3000_device_t*       device,
    dw3000_pmsc_txfseq_t   value
);

dw3000_error_t dw3000_hal_pmsc_read_led_ctrl(
    dw3000_device_t*         device,
    dw3000_pmsc_led_ctrl_t*  led
);

dw3000_error_t dw3000_hal_pmsc_configure_led_ctrl(
    dw3000_device_t*               device,
    const dw3000_pmsc_led_ctrl_t*  led
);

dw3000_error_t dw3000_hal_pmsc_trigger_leds(
    dw3000_device_t*          device,
    dw3000_pmsc_led_flags_t   leds
);

dw3000_error_t dw3000_hal_pmsc_read_bias_ctrl(
    dw3000_device_t*          device,
    dw3000_pmsc_bias_ctrl_t*  value
);

dw3000_error_t dw3000_hal_pmsc_set_bias_ctrl(
    dw3000_device_t*          device,
    dw3000_pmsc_bias_ctrl_t   value
);

/* Applies clock, persistent sequencer, TXFSEQ, and LED settings. A zero
   bias_ctrl leaves BIAS_CTRL unchanged; calibration/OTP code should write
   non-zero OTP trim explicitly once that HAL exists. */
dw3000_error_t dw3000_hal_pmsc_configure(
    dw3000_device_t*              device,
    const dw3000_pmsc_config_t*   config
);

dw3000_error_t dw3000_hal_pmsc_configure_current(dw3000_device_t* device);

#ifdef __cplusplus
}
#endif

#endif /* DW3000_HAL_PMSC_H */
