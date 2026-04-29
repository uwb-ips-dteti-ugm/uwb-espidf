#ifndef DW3000_TYPES_AON_H
#define DW3000_TYPES_AON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AON_DIG_CFG (0x0A:00) — actions the digital block performs on wake.
   Programmed before sleep is asserted; latched into AON memory by the
   AON_CTRL.CFG_UPLOAD command. */
typedef enum {
    DW3000_AON_DIG_ONW_AON_DLD = 1U << 0,  /* reload AON RAM into regs     */
    DW3000_AON_DIG_ONW_RUN_SAR = 1U << 1,  /* sample SAR ADC on wake       */
    DW3000_AON_DIG_ONW_GO2IDLE = 1U << 8,  /* enter IDLE_PLL after wake    */
    DW3000_AON_DIG_ONW_GO2RX   = 1U << 9,  /* start RX after wake          */
    DW3000_AON_DIG_ONW_PGFCAL  = 1U << 11, /* run PGF cal on wake          */
} dw3000_aon_dig_cfg_t;

/* AON_CTRL (0x0A:04) — imperative commands on the AON memory and
   sleep state machine. Bits are self-clearing. */
typedef enum {
    DW3000_AON_CTRL_RESTORE      = 1U << 0, /* copy AON RAM -> regs       */
    DW3000_AON_CTRL_SAVE         = 1U << 1, /* copy regs -> AON RAM       */
    DW3000_AON_CTRL_CFG_UPLOAD   = 1U << 2, /* refresh sleep config       */
    DW3000_AON_CTRL_DCA_READ     = 1U << 3, /* direct AON memory read     */
    DW3000_AON_CTRL_DCA_WRITE    = 1U << 4, /* direct AON memory write    */
    DW3000_AON_CTRL_DCA_WRITE_HI = 1U << 5, /* write address >= 0x100     */
    DW3000_AON_CTRL_DCA_ENAB     = 1U << 7, /* direct AON access enable   */
} dw3000_aon_ctrl_t;

/* AON_ADDR (0x0A:0C) — 9-bit byte address into AON RAM. */
typedef uint16_t dw3000_aon_addr_t;

#define DW3000_AON_ADDR_MASK             0x01FFU
#define DW3000_AON_SLEEP_TIM_ADDR_LO     0x0102U
#define DW3000_AON_SLEEP_TIM_ADDR_HI     0x0103U
#define DW3000_AON_SLEEP_TIM_LP_TICKS    4096U

/* SLEEP_TIM is the upper 16 bits of a 28-bit low-power oscillator counter. */
typedef uint16_t dw3000_aon_sleep_time_t;

/* AON_RDATA (0x0A:08) read port and AON_WDATA (0x0A:10) write port —
   one byte per access, paired with AON_ADDR. */
typedef uint8_t dw3000_aon_byte_t;

/* AON_CFG (0x0A:14) — sleep arming + wake-source enables. SLEEP_EN must
   be set together with whichever wake source(s) you want; SAVE in
   AON_CTRL then puts the device to sleep. */
typedef enum {
    DW3000_AON_CFG_SLEEP_EN   = 1U << 0, /* arm sleep state                */
    DW3000_AON_CFG_WAKE_CNT   = 1U << 1, /* wake when sleep counter expires*/
    DW3000_AON_CFG_BROUT_EN   = 1U << 2, /* enable brownout detector       */
    DW3000_AON_CFG_WAKE_CSN   = 1U << 3, /* wake on SPI CSn falling edge   */
    DW3000_AON_CFG_WAKE_WUP   = 1U << 4, /* wake on WAKEUP pin             */
    DW3000_AON_CFG_PRES_SLEEP = 1U << 5, /* stay asleep across short waks  */
} dw3000_aon_cfg_flags_t;

typedef struct {
    dw3000_aon_dig_cfg_t    dig_cfg;
    dw3000_aon_cfg_flags_t  cfg;
    dw3000_aon_sleep_time_t sleep_time;
} dw3000_aon_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_AON_H */
