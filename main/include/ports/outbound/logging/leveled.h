#ifndef PORTS_OUTBOUND_LOGGING_LEVELED_H
#define PORTS_OUTBOUND_LOGGING_LEVELED_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct po_logging_leveled_s po_logging_leveled_t;

struct po_logging_leveled_s {
    void* ctx;
    void (*error)(po_logging_leveled_t* self, const char* tag, const char* format, ...);
    void (*warn)(po_logging_leveled_t* self, const char* tag, const char* format, ...);
    void (*info)(po_logging_leveled_t* self, const char* tag, const char* format, ...);
    void (*debug)(po_logging_leveled_t* self, const char* tag, const char* format, ...);
};

po_logging_leveled_t* po_logging_leveled_new();

void po_logging_leveled_delete(po_logging_leveled_t* self);

#ifdef __cplusplus
}
#endif

#endif /* PORTS_OUTBOUND_LOGGING_LEVELED_H */