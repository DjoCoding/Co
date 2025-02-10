#include "shared.h"

#include <assert.h>

const OperationStringMapper opstrmapper[OPERATIONS_COUNT] = {
    { OPERATION_ADD,  "+" },
    { OPERATION_SUB, "-" },
    { OPERATION_MUL, "*" },
    { OPERATION_DIV, "/" },
    { OPERATION_LESS, "<" },
    { OPERATION_GREATER, ">" },
    { OPERATION_LESS_OR_EQ, "<=" },
    { OPERATION_GREATER_OR_EQ, ">=" },
    { OPERATION_EQ, "==" },
};

const PreDefinedTypeMap predeftypes[PRE_DEFINED_TYPE_COUNT] = {
    { .type = PRE_DEFINED_TYPE_VOID,    .type_as_cstr = "void" },
    { .type = PRE_DEFINED_TYPE_BOOL,    .type_as_cstr = "bool" },
    { .type = PRE_DEFINED_TYPE_INT,     .type_as_cstr = "int" },
    { .type = PRE_DEFINED_TYPE_STRING,  .type_as_cstr = "string" }
};

const char *mapoptostr(Operation op) {
    for(size_t i = 0; i < LENGTH(opstrmapper); ++i) {
        OperationStringMapper current = opstrmapper[i];
        if(op == current.op) { 
            return current.value;
        }
    }

    //FIXME: error 
    assert(false && "operation logging not implemented yet");
}

Location location(size_t line, size_t offset) {
    return (Location) {
        .line = line, 
        .offset = offset
    };
}