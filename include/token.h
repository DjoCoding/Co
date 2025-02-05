#ifndef TOKEN_H
#define TOKEN_H

#include "sv.h"
#include "da.h"

#define END_TOKEN (Token) { .kind = TOKEN_KIND_END, .value = SV_NULL }

typedef enum {
    TOKEN_KIND_NONE = 0, 
    TOKEN_KIND_FN,
    TOKEN_KIND_IDENTIFIER,
    TOKEN_KIND_OPEN_PAREN,
    TOKEN_KIND_CLOSE_PAREN,
    TOKEN_KIND_OPEN_CURLY,
    TOKEN_KIND_CLOSE_CURLY,
    TOKEN_KIND_COMMA,
    TOKEN_KIND_INTEGER,
    TOKEN_KIND_STRING,
    TOKEN_KIND_PLUS,
    TOKEN_KIND_MINUS,
    TOKEN_KIND_STAR,
    TOKEN_KIND_SLASH,
    TOKEN_KIND_EQUAL,
    TOKEN_KIND_RETURN,
    TOKEN_KIND_END
} TokenKind;    

typedef struct {
    SV value;
    TokenKind kind;
} Token;

DEF_ARRAY(Token);

Token token(SV value, TokenKind kind);
const char *tokenkind_cstr(TokenKind kind);

#endif