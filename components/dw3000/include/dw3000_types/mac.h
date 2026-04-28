#ifndef DW3000_TYPES_MAC_H
#define DW3000_TYPES_MAC_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool beacon;
    bool data;
    bool ack;
    bool mac_command;
    bool reserved;
    bool multipurpose;
    bool fragmented;
    bool extended;
    bool pan_coordinator;
    bool implicit_broadcast;
} dw3000_mac_frame_filter_config_t;

typedef struct {
    bool enabled;
    bool fast_turnaround;
    bool pend_short_any;
    bool pend_long_any;
    uint8_t ack_turnaround_symbols;
    uint32_t wait_for_response_time_us;
} dw3000_mac_auto_ack_config_t;

typedef struct {
    bool frame_filtering;
    bool auto_ack;
    bool disable_fcs_tx;
    bool disable_fcs_check;
    bool double_rx_buffer;
    bool rx_auto_reenable;
    dw3000_mac_frame_filter_config_t frame_filter;
    dw3000_mac_auto_ack_config_t ack;
} dw3000_mac_config_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_MAC_H */
