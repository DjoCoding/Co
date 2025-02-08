#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "sv.h"

typedef struct {
    SV filename;
    SV source;
    size_t line;
    size_t offset;
    size_t current;
    Token currentok;
} Lexer;    

Lexer *lexer(SV source);
void lexerofile(Lexer *this, const char *filename);
ARRAY_OF(Token) lex(Lexer *this);

#endif