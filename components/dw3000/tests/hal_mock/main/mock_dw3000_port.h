#ifndef MOCK_DW3000_PORT_H
#define MOCK_DW3000_PORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dw3000_device.h"
#include "dw3000_error.h"
#include "dw3000_types/reg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_MOCK_PORT_MAX_REGS 32U
#define DW3000_MOCK_PORT_MAX_REG_LEN 32U

typedef struct {
    bool     used;
    bool     write_one_to_clear;
    uint8_t  file_id;
    uint16_t offset;
    uint8_t  data[DW3000_MOCK_PORT_MAX_REG_LEN];
    size_t   len;
} dw3000_mock_reg_t;

typedef struct {
    dw3000_mock_reg_t regs[DW3000_MOCK_PORT_MAX_REGS];

    size_t read_count;
    size_t write_count;
    size_t lock_count;
    size_t unlock_count;

    uint8_t  last_read_file_id;
    uint16_t last_read_offset;
    size_t   last_read_len;

    uint8_t  last_write_file_id;
    uint16_t last_write_offset;
    uint8_t  last_write_data[DW3000_MOCK_PORT_MAX_REG_LEN];
    size_t   last_write_len;

    size_t  fast_command_count;
    uint8_t last_fast_command_header;
    uint8_t last_fast_command;

    dw3000_error_t next_read_error;
    dw3000_error_t next_write_error;
} dw3000_mock_port_t;

void dw3000_mock_port_init(
    dw3000_mock_port_t* mock,
    dw3000_device_t*    device
);

dw3000_error_t dw3000_mock_port_define_reg(
    dw3000_mock_port_t* mock,
    dw3000_reg_desc_t   reg,
    const void*         data,
    size_t              data_len,
    bool                write_one_to_clear
);

dw3000_error_t dw3000_mock_port_set_u48(
    dw3000_mock_port_t* mock,
    dw3000_reg_desc_t   reg,
    uint64_t            value
);

dw3000_error_t dw3000_mock_port_get_u48(
    const dw3000_mock_port_t* mock,
    dw3000_reg_desc_t         reg,
    uint64_t*                 value
);

dw3000_error_t dw3000_mock_port_get_reg_data(
    const dw3000_mock_port_t* mock,
    dw3000_reg_desc_t         reg,
    void*                     data,
    size_t                    data_len
);

uint64_t dw3000_mock_port_last_write_u48(const dw3000_mock_port_t* mock);

void dw3000_mock_port_set_next_read_error(
    dw3000_mock_port_t* mock,
    dw3000_error_t      error
);

void dw3000_mock_port_set_next_write_error(
    dw3000_mock_port_t* mock,
    dw3000_error_t      error
);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_DW3000_PORT_H */
