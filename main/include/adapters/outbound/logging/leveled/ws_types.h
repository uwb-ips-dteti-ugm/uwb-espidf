#ifndef ADAPTERS_OUTBOUND_LOGGING_LEVELED_WS_TYPES_H
#define ADAPTERS_OUTBOUND_LOGGING_LEVELED_WS_TYPES_H

#include <stdbool.h>

#include "esp_websocket_client.h"
#include "portmacro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AO_LOGGING_LEVELED_WS_LEVEL_NONE,
    AO_LOGGING_LEVELED_WS_LEVEL_ERROR,
    AO_LOGGING_LEVELED_WS_LEVEL_WARN,
    AO_LOGGING_LEVELED_WS_LEVEL_INFO,
    AO_LOGGING_LEVELED_WS_LEVEL_DEBUG,
} ao_logging_leveled_ws_level;

typedef struct {
    ao_logging_leveled_ws_level level;
    TickType_t                  send_timeout;
} ao_logging_leveled_ws_config_t;

typedef struct {
    ao_logging_leveled_ws_config_t cfg;
    esp_websocket_client_handle_t  ws_client;
    bool                           is_requested;
} ao_logging_leveled_ws_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_OUTBOUND_LOGGING_LEVELED_WS_TYPES_H */