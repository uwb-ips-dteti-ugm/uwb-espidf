#ifndef DOMAIN_MODELS_ERROR_H
#define DOMAIN_MODELS_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#define DOMAIN_MODELS_ERROR(X)                                        \
    X(DOMAIN_MODELS_ERROR_OK, "Operation successful")                 \
    X(DOMAIN_MODELS_ERROR_INVALID_ARGUMENT, "Invalid argument error") \
    X(DOMAIN_MODELS_ERROR_MALLOC, "Memory allocation error")          \
    X(DOMAIN_MODELS_ERROR_SYSTEM_FAIL, "System failure error")        \
    X(DOMAIN_MODELS_ERROR_UNIMPLEMENTED, "Unimplemented error")       \
    X(DOMAIN_MODELS_ERROR_UNKNOWN, "Unknown error")

typedef enum {
#define X(cb_name, cb_string) cb_name,
    DOMAIN_MODELS_ERROR(X)
#undef X
} domain_models_error_t;

static inline const char* domain_models_error_to_str(domain_models_error_t err) {
    switch (err) {
#define X(cb_name, cb_string) \
    case cb_name:             \
        return cb_string;
        DOMAIN_MODELS_ERROR(X)
#undef X
    }
    return domain_models_error_to_str(DOMAIN_MODELS_ERROR_UNKNOWN);
}

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODELS_ERROR_H */
