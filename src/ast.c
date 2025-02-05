#include "ast.h"
#include "malloc.h"

AST *ast() { 
    AST *this = alloc(sizeof(AST));
    this->items = NULL;
    this->count = 0;
    this->size = 0;
    return this;
}