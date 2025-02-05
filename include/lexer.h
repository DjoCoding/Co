#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "sv.h"

typedef struct {
    SV source;
    size_t current;
} Lexer;    

Lexer *lexer(SV source);
ARRAY_OF(Token) lex(Lexer *this);

#endif