#ifndef DW3000_TYPES_SPI_H
#define DW3000_TYPES_SPI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SPI_RD_CRC (0x00:18) — chip-computed CRC of the most recent SPI read
   transaction. Host compares against its locally-computed CRC; mismatch
   surfaces as DW3000_TXRX_EVENT_SPICRCE in SYS_STATUS. Seed is set by
   SPICRCINIT (see dw3000_diag_spicrc_init_t in diag.h). */
typedef uint8_t dw3000_spi_rd_crc_t;

/* SPI_COLLISION (0x01:20) — 5-bit encoded reason for the most recent
   SPI collision (host SPI access during a sensitive hardware event).
   0 = no collision; non-zero values are looked up in user manual
   Table 13 to identify the conflicting operation. */
typedef uint8_t dw3000_spi_collision_t;

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_SPI_H */
