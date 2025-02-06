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
    "if",
    ">",
    ">=",
    "<",
    "<=",
    "==",
    "for",
    ";",
    ":=",
    "end"    
};

const TokenKind addition[ADDITION_TOKEN_KIND_COUNT] = {
    TOKEN_KIND_PLUS,
    TOKEN_KIND_MINUS
};

const TokenKind multiplication[MULTIPLICATION_TOKEN_KIND_COUNT] = {
    TOKEN_KIND_STAR, 
    TOKEN_KIND_SLASH
};

const TokenKind comparaison[COMPARAISON_TOKEN_KIND_COUNT] = {
    TOKEN_KIND_LESS,
    TOKEN_KIND_LESS_OR_EQ,
    TOKEN_KIND_GREATER,
    TOKEN_KIND_GREATER_OR_EQ,
    TOKEN_KIND_EQ
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

bool iscomparaison(TokenKind kind) {
    for(size_t i = 0; i < COMPARAISON_TOKEN_KIND_COUNT; ++i) {
        if(comparaison[i] == kind) { return true; }
    }
    return false;
}

bool isaddition(TokenKind kind) {
    for(size_t i = 0; i < ADDITION_TOKEN_KIND_COUNT; ++i) {
        if(addition[i] == kind) { return true; }
    }
    return false;
}

bool ismultiplication(TokenKind kind) {
    for(size_t i = 0; i < MULTIPLICATION_TOKEN_KIND_COUNT; ++i) {
        if(multiplication[i] == kind) { return true; }
    }
    return false;
}