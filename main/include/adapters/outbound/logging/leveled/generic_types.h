#ifndef ADAPTERS_OUTBOUND_LOGGING_LEVELED_GENERIC_TYPES_H
#define ADAPTERS_OUTBOUND_LOGGING_LEVELED_GENERIC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AO_LOGGING_LEVELED_GENERIC_LEVEL_NONE,
    AO_LOGGING_LEVELED_GENERIC_LEVEL_ERROR,
    AO_LOGGING_LEVELED_GENERIC_LEVEL_WARN,
    AO_LOGGING_LEVELED_GENERIC_LEVEL_INFO,
    AO_LOGGING_LEVELED_GENERIC_LEVEL_DEBUG,
} ao_logging_leveled_generic_level;

typedef struct {
    ao_logging_leveled_generic_level level;
} ao_logging_leveled_generic_config_t;

typedef struct {
    ao_logging_leveled_generic_config_t cfg;
} ao_logging_leveled_generic_ctx_t;

#ifdef __cplusplus
}
#endif

#endif /* ADAPTERS_OUTBOUND_LOGGING_LEVELED_GENERIC_TYPES_H */