#include "token.h"

const char *token_kind_cstr[] = {
    "None",
    "fn",
    "id",
    "(",
    ")",
    "{",
    "}",
    ",",
    "integer",
    "string",
    "+",
    "-",
    "*",
    "/",
    "=",
    "return",
    ":",
    "end"    
};

Token token(SV value, TokenKind kind) {
    return (Token) {
        .value = value, 
        .kind = kind
    };
}

const char *tokenkind_cstr(TokenKind kind) {
    return token_kind_cstr[(size_t)kind];
}