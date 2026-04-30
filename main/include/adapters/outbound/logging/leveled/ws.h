#ifndef ADAPTERS_OUTBOUND_LOGGING_LEVELED_WS_H
#define ADAPTERS_OUTBOUND_LOGGING_LEVELED_WS_H

#include "adapters/outbound/logging/leveled/ws_types.h"
#include "domain/models/error.h"
#include "ports/outbound/logging/leveled.h"

#ifdef __cplusplus
extern "C" {
#endif

po_logging_leveled_t* ao_logging_leveled_ws_new(esp_websocket_client_handle_t ws_client, const ao_logging_leveled_ws_config_t* cfg);

void ao_logging_leveled_ws_delete(po_logging_leveled_t* self);

domain_models_error_t ao_logging_leveled_ws_init(po_logging_leveled_t* self);

void ao_logging_leveled_ws_deinit(po_logging_leveled_t* self);

void ao_logging_leveled_ws_error(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_ws_warn(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_ws_info(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_ws_debug(po_logging_leveled_t* self, const char* tag, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_OUTBOUND_LOGGING_LEVELED_WS_H */