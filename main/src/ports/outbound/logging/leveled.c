#include "ports/outbound/logging/leveled.h"

#include <stdlib.h>

po_logging_leveled_t* po_logging_leveled_new() {
    po_logging_leveled_t* self;
    self = (po_logging_leveled_t*)calloc(1, sizeof(po_logging_leveled_t));
    return self;
}

void po_logging_leveled_delete(po_logging_leveled_t* self) {
    free(self);
}