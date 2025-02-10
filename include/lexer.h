#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "sv.h"
#include "shared.h"

typedef struct {
    SV filename;
    SV source;
    Location loc;
    size_t current;
    Token currentok;
} Lexer;    

Lexer *lexer(const char *filename);
void lexer_set_source(Lexer *this, SV source);
ARRAY_OF(Token) lex(Lexer *this);

#endif