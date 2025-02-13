#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>

// wrapper over malloc (prevents NULL malloc pointers)
void *alloc(size_t size);

// wrapper over the realloc function
void *ralloc(void *org, size_t size);

#endif