#include "adapters/outbound/logging/leveled/generic.h"

#include <reent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "adapters/outbound/logging/leveled/generic_types.h"
#include "ports/outbound/logging/leveled.h"

static void print_log(const char* level, const char* tag, const char* format, va_list args) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo);

    printf(
        "%02d/%02d/%04d %02d:%02d:%02d.%03ld [%s] [%s] ",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        tv.tv_usec / 1000,
        level,
        tag
    );
    vprintf(format, args);
    printf("\n");
}

po_logging_leveled_t* ao_logging_leveled_generic_new(const ao_logging_leveled_generic_config_t* cfg) {
    ao_logging_leveled_generic_ctx_t* ctx = (ao_logging_leveled_generic_ctx_t*)calloc(1, sizeof(ao_logging_leveled_generic_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (cfg) {
        memcpy(&ctx->cfg, cfg, sizeof(ao_logging_leveled_generic_config_t));
    }

    po_logging_leveled_t* self = po_logging_leveled_new();
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->ctx   = ctx;
    self->error = ao_logging_leveled_generic_error;
    self->warn  = ao_logging_leveled_generic_warn;
    self->info  = ao_logging_leveled_generic_info;
    self->debug = ao_logging_leveled_generic_debug;
    return self;
}

void ao_logging_leveled_generic_delete(po_logging_leveled_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    po_logging_leveled_delete(self);
}

void ao_logging_leveled_generic_error(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_generic_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_GENERIC_LEVEL_ERROR) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("ERROR", tag, format, args);
    va_end(args);
}

void ao_logging_leveled_generic_warn(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_generic_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_GENERIC_LEVEL_WARN) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("WARN", tag, format, args);
    va_end(args);
}

void ao_logging_leveled_generic_info(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_generic_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_GENERIC_LEVEL_INFO) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("INFO", tag, format, args);
    va_end(args);
}

void ao_logging_leveled_generic_debug(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_generic_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_GENERIC_LEVEL_DEBUG) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log("DEBUG", tag, format, args);
    va_end(args);
}