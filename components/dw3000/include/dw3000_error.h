#ifndef DW3000_ERROR_H
#define DW3000_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_ERROR_LIST(X)                           \
    X(DW3000_ERROR_OK, 0, "OK")                        \
    X(DW3000_ERROR_INVALID_ARG, -1, "Invalid arg")     \
    X(DW3000_ERROR_INVALID_STATE, -2, "Invalid state") \
    X(DW3000_ERROR_INVALID_SIZE, -3, "Invalid size")   \
    X(DW3000_ERROR_NOT_SUPPORTED, -4, "Not supported") \
    X(DW3000_ERROR_TIMEOUT, -5, "Timeout")             \
    X(DW3000_ERROR_IO, -6, "IO error")                 \
    X(DW3000_ERROR_BUSY, -7, "Busy")                   \
    X(DW3000_ERROR_NO_MEMORY, -8, "No memory")

typedef enum {
#define DW3000_ERROR_ENUM(name_, value_, str_) name_ = value_,
    DW3000_ERROR_LIST(DW3000_ERROR_ENUM)
#undef DW3000_ERROR_ENUM
} dw3000_error_t;

static inline const char* dw3000_error_to_string(dw3000_error_t err) {
    switch (err) {
#define DW3000_ERROR_CASE(name_, value_, str_) \
    case name_:                                \
        return str_;
        DW3000_ERROR_LIST(DW3000_ERROR_CASE)
#undef DW3000_ERROR_CASE
        default:
            return "unknown error";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* DW3000_ERROR_H */
