#include "colib.h"

#include "coerror.h"
#include "da.h"

typedef struct {
    CoLib lib;
    const char *header;
    const char *tag;
} CoLibObject;

CoLibObject map[] = {
    { COSTRING, "costring", "STD_COSTRING_IMPLEMENTATION" },
    { COARRAY, "coarray", "STD_COARRAY_IMPLEMENTATION" },
};

CoLib colibfromname(SV name) {
    for(size_t i = 0; i < LENGTH(map); ++i) {
        if(svcmp(name, svc((char *)map[i].header))) {
            return map[i].lib;
        }
    }

    UNREACHABLE();
}

const char *headerof(CoLib lib) {
    for(size_t i = 0; i< LENGTH(map); ++i) {
        if(lib == map[i].lib) { 
            return map[i].header;
        }
    }

    UNREACHABLE();
} 

const char *tagof(CoLib lib) {
    for(size_t i = 0; i< LENGTH(map); ++i) {
        if(lib == map[i].lib) { 
            return map[i].tag;
        }
    }

    UNREACHABLE();
}