#ifndef DW3000_TYPES_ERR_H
#define DW3000_TYPES_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

#define DW3000_ERR_LIST(X)                           \
    X(DW3000_OK, 0, "OK")                            \
    X(DW3000_ERR_INVALID_ARG, -1, "Invalid arg")     \
    X(DW3000_ERR_INVALID_STATE, -2, "Invalid state") \
    X(DW3000_ERR_INVALID_SIZE, -3, "Invalid size")   \
    X(DW3000_ERR_NOT_SUPPORTED, -4, "Not supported") \
    X(DW3000_ERR_TIMEOUT, -5, "Timeout")             \
    X(DW3000_ERR_IO, -6, "IO error")                 \
    X(DW3000_ERR_BUSY, -7, "Busy")                   \
    X(DW3000_ERR_NO_MEMORY, -8, "No memory")

typedef enum {
#define DW3000_ERR_ENUM(name_, value_, str_) name_ = value_,
    DW3000_ERR_LIST(DW3000_ERR_ENUM)
#undef DW3000_ERR_ENUM
} dw3000_err_t;

static inline const char* dw3000_err_to_str(dw3000_err_t err) {
    switch (err) {
#define DW3000_ERR_CASE(name_, value_, str_) \
    case name_:                              \
        return str_;
        DW3000_ERR_LIST(DW3000_ERR_CASE)
#undef DW3000_ERR_CASE
        default:
            return "unknown error";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* DW3000_TYPES_ERR_H */
