#ifndef SHARED_H
#define SHARED_H 

#include "node.h"

typedef struct {
    Operation op;
    const char *value;
} OperationStringMapper; 

extern const OperationStringMapper opstrmapper[OPERATIONS_COUNT];

const char *mapoptostr(Operation op);

#endif