#ifndef AST_H
#define AST_H

#include "node.h"

typedef struct {
    Node **items;
    size_t count;
    size_t size;
} AST;

AST *ast();

#endif