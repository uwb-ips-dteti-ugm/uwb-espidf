#ifndef ADAPTERS_OUTBOUND_LOGGING_LEVELED_GENERIC_H
#define ADAPTERS_OUTBOUND_LOGGING_LEVELED_GENERIC_H

#include "adapters/outbound/logging/leveled/generic_types.h"
#include "ports/outbound/logging/leveled.h"

#ifdef __cplusplus
extern "C" {
#endif

po_logging_leveled_t* ao_logging_leveled_generic_new(const ao_logging_leveled_generic_config_t* cfg);

void ao_logging_leveled_generic_delete(po_logging_leveled_t* self);

void ao_logging_leveled_generic_error(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_generic_warn(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_generic_info(po_logging_leveled_t* self, const char* tag, const char* format, ...);

void ao_logging_leveled_generic_debug(po_logging_leveled_t* self, const char* tag, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_OUTBOUND_LOGGING_LEVELED_GENERIC_H */