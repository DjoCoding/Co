#ifndef CODE_H
#define CODE_H

#include "ast.h"
#include "context.h"
#include "shared.h"
#include "typechecker.h"

typedef struct {
    size_t indent;
    const char *input;
    const char *output;
    FILE *f;
} CodeGeneratorState;

typedef struct {
    AST *tree;
    CodeGeneratorContext *gcontext;
    CodeGeneratorState state;
} CodeGenerator;

CodeGenerator *code(const char *input, const char *output);
void code_set_tree(CodeGenerator *this, AST *tree);
void generate(CodeGenerator *this);

#endif