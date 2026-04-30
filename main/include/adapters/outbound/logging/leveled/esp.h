#ifndef ADAPTERS_OUTBOUND_LOGGING_LEVELED_ESP_H
#define ADAPTERS_OUTBOUND_LOGGING_LEVELED_ESP_H

#include "adapters/outbound/logging/leveled/esp_types.h"
#include "ports/outbound/logging/leveled.h"

#ifdef __cplusplus
extern "C" {
#endif

po_logging_leveled_t* ao_logging_leveled_esp_new(const ao_logging_leveled_esp_config_t* cfg);

void ao_logging_leveled_esp_delete(po_logging_leveled_t* self);

void ao_logging_leveled_esp_error(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_esp_warn(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_esp_info(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_esp_debug(po_logging_leveled_t* self, const char* tag, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_OUTBOUND_LOGGING_LEVELED_ESP_H */