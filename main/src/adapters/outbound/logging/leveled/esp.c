#include "adapters/outbound/logging/leveled/esp.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "adapters/outbound/logging/leveled/esp_types.h"
#include "esp_log_write.h"
#include "ports/outbound/logging/leveled.h"

po_logging_leveled_t* ao_logging_leveled_esp_new(const ao_logging_leveled_esp_config_t* cfg) {
    ao_logging_leveled_esp_ctx_t* ctx = (ao_logging_leveled_esp_ctx_t*)calloc(1, sizeof(ao_logging_leveled_esp_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (cfg) {
        memcpy(&ctx->cfg, cfg, sizeof(ao_logging_leveled_esp_config_t));
    }

    po_logging_leveled_t* self = po_logging_leveled_new();
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->ctx   = ctx;
    self->error = ao_logging_leveled_esp_error;
    self->warn  = ao_logging_leveled_esp_warn;
    self->info  = ao_logging_leveled_esp_info;
    self->debug = ao_logging_leveled_esp_debug;
    return self;
}

void ao_logging_leveled_esp_delete(po_logging_leveled_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    po_logging_leveled_delete(self);
}

void ao_logging_leveled_esp_error(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_ERROR, tag, format, args);
    va_end(args);
}

void ao_logging_leveled_esp_warn(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_WARN, tag, format, args);
    va_end(args);
}

void ao_logging_leveled_esp_info(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, tag, format, args);
    va_end(args);
}

void ao_logging_leveled_esp_debug(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    (void)self;
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_DEBUG, tag, format, args);
    va_end(args);
}
