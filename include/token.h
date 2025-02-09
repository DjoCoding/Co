#ifndef TOKEN_H
#define TOKEN_H

#include "sv.h"
#include "da.h"
#include "shared.h"

#define END_TOKEN (Token) { .kind = TOKEN_KIND_END, .value = SV_NULL }

typedef enum {
    TOKEN_KIND_NONE = 0, 
    TOKEN_KIND_INVALID,
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
    TOKEN_KIND_EQUAL,                       // =
    TOKEN_KIND_RETURN,
    TOKEN_KIND_COLON,
    TOKEN_KIND_IF,
    TOKEN_KIND_GREATER,
    TOKEN_KIND_GREATER_OR_EQ,
    TOKEN_KIND_LESS,
    TOKEN_KIND_LESS_OR_EQ,
    TOKEN_KIND_EQ,                          // == 
    TOKEN_KIND_FOR,
    TOKEN_KIND_SEMI_COLON,
    TOKEN_KIND_COLON_EQ,                    // :=
    TOKEN_KIND_END
} TokenKind;    

typedef struct {
    SV value;
    TokenKind kind;
    Location loc;
} Token;

#define TOKEN_NONE (Token) { .value = SV_NULL, .kind = TOKEN_KIND_NONE }

DEF_ARRAY(Token);

Token token(SV value, TokenKind kind, Location loc);
const char *tokenkind_cstr(TokenKind kind);

#define ADDITION_TOKEN_KIND_COUNT 2
extern const TokenKind addition[ADDITION_TOKEN_KIND_COUNT];

#define MULTIPLICATION_TOKEN_KIND_COUNT 2
extern const TokenKind multiplication[MULTIPLICATION_TOKEN_KIND_COUNT];

#define COMPARAISON_TOKEN_KIND_COUNT 5
extern const TokenKind comparaison[COMPARAISON_TOKEN_KIND_COUNT];

bool iscomparaison(TokenKind kind);
bool isaddition(TokenKind kind);
bool ismultiplication(TokenKind kind);

#endif