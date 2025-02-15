#ifndef STD_COARRAY_H
#define STD_COARRAY_H

#include <stdio.h>

#define CoArray(T) \
    typedef struct { \
        T *items; \
        size_t count; \
        size_t size; \
    } CoArrayOf##T

#endif