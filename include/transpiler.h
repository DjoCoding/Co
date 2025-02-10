#ifndef TRANSPILER_H
#define TRANSPILER_H

#include "lexer.h"
#include "parser.h"
#include "code.h"
#include "da.h"

typedef struct {
    const char *filename;
    const char *output;
    SV source;
    Lexer *l;
    Parser *p;
    CodeGenerator *c;
    ARRAY_OF(Token) tokens;
    AST *tree;
} Transpiler;

Transpiler *transpiler(const char *filename, const char *output);
bool transpiler_read_source(Transpiler *this);
void transpiler_setup_lexer(Transpiler *this);
void transpiler_lex_source(Transpiler *this);
void transpiler_setup_parser(Transpiler *this);
void transpiler_parse_tokens(Transpiler *this);
void transpiler_setup_generator(Transpiler *this);
void transpiler_gencode(Transpiler *this);
void transpiler_cleanup(Transpiler *this);

#endif