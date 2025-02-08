#ifndef CODE_H
#define CODE_H

#include "ast.h"
#include "context.h"
#include "shared.h"
#include "typechecker.h"

typedef struct {
    size_t indent;
    const char *filpath;
    FILE *f;
} CodeGeneratorState;

typedef struct {
    AST *tree;
    CodeGeneratorContext *gcontext;
    CodeGeneratorState state;
} CodeGenerator;

CodeGenerator *code(AST *tree);
void code_setup(CodeGenerator *this, const char *filepath);
void generate(CodeGenerator *this);

#endif