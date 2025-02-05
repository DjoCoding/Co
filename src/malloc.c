#include "malloc.h"

#include <stdlib.h>
#include <assert.h>

void *alloc(size_t size) {
    void *p = malloc(size);
    assert(p && "memory allocation failed");
    return p;
}

void *ralloc(void *org, size_t size) {
    void *p = realloc(org, size);
    assert(p && "memory allocation failed");
    return p;
}

