#ifndef CODE_H
#define CODE_H

#include "ast.h"
#include "context.h"

typedef struct {
    AST *tree;
    CodeGeneratorContext *gcontext;
} CodeGenerator;

CodeGenerator *code(AST *tree);
void generate(CodeGenerator *this);

#endif