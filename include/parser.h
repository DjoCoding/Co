#ifndef PARSER_H
#define PARSER_H

#include "coerror.h"
#include "da.h"
#include "token.h"
#include "node.h"
#include "ast.h"

typedef struct {
    SV filename;
    ARRAY_OF(Token) tokens;
    size_t current;
} Parser;


Parser *parser(const char *filename);
void parser_set_tokens(Parser *this, ARRAY_OF(Token) tokens);
AST *parse(Parser *this);

#endif