#include "shared.h"

#include <assert.h>

const OperationStringMapper opstrmapper[OPERATIONS_COUNT] = {
    { OPERATION_ADD,  "+" },
    { OPERATION_SUB, "-" },
    { OPERATION_MUL, "*" },
    { OPERATION_DIV, "/" }
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