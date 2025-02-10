#ifndef SHARED_H
#define SHARED_H 

#include "node.h"

typedef struct {
    Operation op;
    const char *value;
} OperationStringMapper; 

typedef struct {
    PreDefinedType type;
    const char *type_as_cstr;
} PreDefinedTypeMap;

extern const OperationStringMapper opstrmapper[OPERATIONS_COUNT];
extern const PreDefinedTypeMap predeftypes[PRE_DEFINED_TYPE_COUNT];

const char *mapoptostr(Operation op);

typedef struct {
    size_t line;
    size_t offset;
} Location;

Location location(size_t line, size_t offset);

#endif