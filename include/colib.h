#ifndef CO_STDLIBS_H
#define CO_STDLIBS_H

#include "sv.h"

typedef enum {
    COSTRING,
    COARRAY,
} CoLib;

CoLib colibfromname(SV name);
const char *headerof(CoLib lib);
const char *tagof(CoLib lib);

#endif