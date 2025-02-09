#ifndef SHARED_H
#define SHARED_H 

#include "node.h"

typedef struct {
    Operation op;
    const char *value;
} OperationStringMapper; 

extern const OperationStringMapper opstrmapper[OPERATIONS_COUNT];

const char *mapoptostr(Operation op);

typedef struct {
    size_t line;
    size_t offset;
} Location;

Location location(size_t line, size_t offset);

#endif