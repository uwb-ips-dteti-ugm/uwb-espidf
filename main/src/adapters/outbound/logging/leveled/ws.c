#include "adapters/outbound/logging/leveled/ws.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "adapters/outbound/logging/leveled/ws_types.h"
#include "cJSON.h"
#include "domain/models/error.h"
#include "esp_err.h"
#include "esp_websocket_client.h"
#include "ports/outbound/logging/leveled.h"

#define TAG_PATH "logging/leveled"

static void print_log(ao_logging_leveled_ws_ctx_t* ctx, const char* level, const char* tag, const char* format, va_list args) {
    char buf[640];
    int  cur       = 0;
    int  remaining = sizeof(buf);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo);

    int written = snprintf(
        buf,
        remaining,
        "%02d/%02d/%04d %02d:%02d:%02d.%03ld [%s] [%s] ",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        tv.tv_usec / 1000,
        level,
        tag ? tag : ""
    );
    if (written > 0 && written < remaining) {
        cur += written;
        remaining -= written;
    }

    int msg_len = vsnprintf(buf + cur, remaining, format, args);
    int total_len;
    if (msg_len < 0) {
        total_len = cur;
    } else if (msg_len >= remaining) {
        total_len = sizeof(buf) - 1;
    } else {
        total_len = cur + msg_len;
    }

    if (total_len < (int)sizeof(buf) - 1) {
        buf[total_len++] = '\n';
        buf[total_len]   = '\0';
    }

    printf("%s", buf);
    if (esp_websocket_client_is_connected(ctx->ws_client) && ctx->is_requested) {
        esp_websocket_client_send_text(ctx->ws_client, buf, total_len, ctx->cfg.send_timeout);
    }
}

static void ws_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    const char*                  tag  = TAG_PATH "/ws_event";
    po_logging_leveled_t*        self = (po_logging_leveled_t*)handler_args;
    ao_logging_leveled_ws_ctx_t* ctx  = self->ctx;
    esp_websocket_event_data_t*  data = (esp_websocket_event_data_t*)event_data;

    switch (event_id) {
        case WEBSOCKET_EVENT_BEGIN:
            ao_logging_leveled_ws_info(self, tag, "Websocket started");
            break;

        case WEBSOCKET_EVENT_CONNECTED:
            ao_logging_leveled_ws_info(self, tag, "Websocket connected");
            break;

        case WEBSOCKET_EVENT_DISCONNECTED:
            ao_logging_leveled_ws_warn(self, tag, "Websocket disconnected");
            break;

        case WEBSOCKET_EVENT_CLOSED:
            ao_logging_leveled_ws_warn(self, tag, "Websocket closed");
            break;

        case WEBSOCKET_EVENT_DATA:
            ao_logging_leveled_ws_info(self, tag, "Websocket data retrieved");

            if ((data->op_code == 0x01 || data->op_code == 0x02) && data->data_ptr != NULL) {
                cJSON* root = cJSON_ParseWithLength(data->data_ptr, data->data_len);
                if (root == NULL) {
                    const char* error_ptr = cJSON_GetErrorPtr();
                    if (error_ptr != NULL) {
                        ao_logging_leveled_ws_error(self, tag, "JSON Parse Error before: %s", error_ptr);
                    }
                    break;
                }

                cJSON* command = cJSON_GetObjectItemCaseSensitive(root, "command");
                if (cJSON_IsString(command) && (command->valuestring != NULL)) {
                    if (strcmp(command->valuestring, "START") == 0) {
                        ctx->is_requested = true;
                    } else if (strcmp(command->valuestring, "STOP") == 0) {
                        ctx->is_requested = false;
                    }
                }

                cJSON_Delete(root);
            }
            break;

        case WEBSOCKET_EVENT_ERROR:
            ao_logging_leveled_ws_error(
                self,
                tag,
                "Websocket error occured. Status Code: %d. TLS Error: %s. TLS Stack Error: %d. Socket Error: %d",
                data->error_handle.esp_ws_handshake_status_code,
                esp_err_to_name(data->error_handle.esp_tls_last_esp_err),
                data->error_handle.esp_tls_stack_err,
                data->error_handle.esp_transport_sock_errno
            );
            break;
    }
}

po_logging_leveled_t* ao_logging_leveled_ws_new(esp_websocket_client_handle_t ws_client, const ao_logging_leveled_ws_config_t* cfg) {
    ao_logging_leveled_ws_ctx_t* ctx = (ao_logging_leveled_ws_ctx_t*)calloc(1, sizeof(ao_logging_leveled_ws_ctx_t));
    if (!ctx) {
        return NULL;
    }

    if (cfg) {
        memcpy(&ctx->cfg, cfg, sizeof(ao_logging_leveled_ws_config_t));
    }
    ctx->ws_client = ws_client;

    po_logging_leveled_t* self = po_logging_leveled_new();
    if (!self) {
        free(ctx);
        return NULL;
    }

    self->ctx   = ctx;
    self->error = ao_logging_leveled_ws_error;
    self->warn  = ao_logging_leveled_ws_warn;
    self->info  = ao_logging_leveled_ws_info;
    self->debug = ao_logging_leveled_ws_debug;
    return self;
}

void ao_logging_leveled_ws_delete(po_logging_leveled_t* self) {
    if (!self) {
        return;
    }

    free(self->ctx);
    po_logging_leveled_delete(self);
}

domain_models_error_t ao_logging_leveled_ws_init(po_logging_leveled_t* self) {
    const char* tag = TAG_PATH "/init";

    if (!self || !self->ctx) {
        return DOMAIN_MODELS_ERROR_INVALID_ARGUMENT;
    }

    ao_logging_leveled_ws_ctx_t* ctx = self->ctx;

    esp_err_t err = esp_websocket_register_events(
        ctx->ws_client,
        WEBSOCKET_EVENT_ANY,
        ws_event_handler,
        (void*)self
    );
    if (err != ESP_OK) {
        ao_logging_leveled_ws_error(self, tag, "Failed to register websocket event: %s", esp_err_to_name(err));
        return DOMAIN_MODELS_ERROR_MALLOC;
    }
    ao_logging_leveled_ws_info(self, tag, "Websocket event registered");

    return DOMAIN_MODELS_ERROR_OK;
}

void ao_logging_leveled_ws_deinit(po_logging_leveled_t* self) {
    const char* tag = TAG_PATH "/deinit";

    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_ws_ctx_t* ctx = self->ctx;

    esp_websocket_unregister_events(ctx->ws_client, WEBSOCKET_EVENT_ANY, ws_event_handler);
    ao_logging_leveled_ws_info(self, tag, "Websocket event unregistered");
}

void ao_logging_leveled_ws_error(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_ws_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_WS_LEVEL_ERROR) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "ERROR", tag, format, args);
    va_end(args);
}

void ao_logging_leveled_ws_warn(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_ws_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_WS_LEVEL_WARN) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "WARN", tag, format, args);
    va_end(args);
}

void ao_logging_leveled_ws_info(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_ws_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_WS_LEVEL_INFO) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "INFO", tag, format, args);
    va_end(args);
}

void ao_logging_leveled_ws_debug(po_logging_leveled_t* self, const char* tag, const char* format, ...) {
    if (!self || !self->ctx) {
        return;
    }

    ao_logging_leveled_ws_ctx_t* ctx = self->ctx;
    if (ctx->cfg.level < AO_LOGGING_LEVELED_WS_LEVEL_DEBUG) {
        return;
    }

    va_list args;
    va_start(args, format);
    print_log(ctx, "DEBUG", tag, format, args);
    va_end(args);
}