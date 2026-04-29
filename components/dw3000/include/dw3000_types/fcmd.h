#ifndef DW3000_TYPES_FCMD_H
#define DW3000_TYPES_FCMD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Fast-command opcodes (5 bits). Issued via the dedicated SPI fast-
   command transaction (1-byte header, no payload) for low-latency
   control of the TX/RX state machines. The opcode of the most recent
   command is observable in DIG_DIAG.FCMD_STAT (file 0x0F:3C).

   Naming reference (DW3000 user manual §9.3):
     CMD_*       — radio operation triggers
     CMD_DTX_*   — delayed TX variants (TS=timestamp, RS=ref-sync, REF=
                   absolute reference)
     CMD_DRX_*   — delayed RX variants
     *_W4R       — same as base command but auto-arms RX after TX
                   (wait-for-response)
     CMD_CCA_TX  — TX guarded by clear-channel assessment
     CMD_CLR_IRQS — clear all IRQs in SYS_STATUS
     CMD_DB_TOGGLE — flip the active double-buffered RX bank */
typedef enum {
    DW3000_FCMD_TXRXOFF     = 0x00U,
    DW3000_FCMD_TX          = 0x01U,
    DW3000_FCMD_RX          = 0x02U,
    DW3000_FCMD_DTX         = 0x03U,
    DW3000_FCMD_DRX         = 0x04U,
    DW3000_FCMD_DTX_TS      = 0x05U,
    DW3000_FCMD_DRX_TS      = 0x06U,
    DW3000_FCMD_DTX_RS      = 0x07U,
    DW3000_FCMD_DRX_RS      = 0x08U,
    DW3000_FCMD_DTX_REF     = 0x09U,
    DW3000_FCMD_DRX_REF     = 0x0AU,
    DW3000_FCMD_CCA_TX      = 0x0BU,
    DW3000_FCMD_TX_W4R      = 0x0CU,
    DW3000_FCMD_DTX_W4R     = 0x0DU,
    DW3000_FCMD_DTX_TS_W4R  = 0x0EU,
    DW3000_FCMD_DTX_RS_W4R  = 0x0FU,
    DW3000_FCMD_DTX_REF_W4R = 0x10U,
    DW3000_FCMD_CCA_TX_W4R  = 0x11U,
    DW3000_FCMD_CLR_IRQS    = 0x12U,
    DW3000_FCMD_DB_TOGGLE   = 0x13U,
} dw3000_fcmd_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_FCMD_H */
