#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>

void *alloc(size_t size);
void *ralloc(void *org, size_t size);

#endif