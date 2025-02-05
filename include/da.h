#ifndef DA_H
#define DA_H

#include "malloc.h"

#define LENGTH(a) (sizeof(a) / sizeof(*(a)))

#define ARRAY_OF(T) ArrayOf##T

#define ARRAY(T) (ARRAY_OF(T)) {0}

#define DEF_ARRAY(T) \
    typedef struct { \
        T *items; \
        size_t count; \
        size_t size; \
    } ARRAY_OF(T)

#define RESIZE(a) \
    do { \
        if((a)->size == 0) { \
            (a)->size = 2; \
        } \
        (a)->size *= 2; \
        (a)->items = ralloc((a)->items, sizeof(*(a)->items) * (a)->size); \
    } while(0)

#define APPEND(a, item) \
    do { \
        if((a)->count >= (a)->size) { RESIZE(a); } \
        (a)->items[(a)->count++] = item; \
    } while(0)


#endif