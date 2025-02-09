#ifndef COERROR_H
#define COERROR_H

#include "token.h"
#include "sv.h"
#include "shared.h"

#define TODO(...) { fprintf(stdout, "%s:%d:%s -> marked as todo\n", __FILE__, __LINE__, __FUNCTION__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); abort(); }
#define THROW(...) { fprintf(stderr, "ERROR: ", __VA_ARGS__); exit(EXIT_FAILURE); }
#define UNREACHABLE() { fprintf(stderr, "%s:%d:%s -> unreachable\n", __FILE__, __LINE__, __FUNCTION__); exit(EXIT_FAILURE); }

typedef enum {
    LEXER = 0,
    PARSER,
    CODE_GENERATOR
} Stage;

typedef enum {
    ERROR_CODE_NONE = 0,
    INVALID_TOKEN,
    INVALID_STRING,
    UNKNOWN_TYPE_NAME,
    EXPECTED_TOKEN_KIND_BUT_FOUND_ANOTHER,
    EXPECTED_EXPRESSION,
} ErrorCode;

typedef struct {
    ErrorCode code;
    SV filename;
    Location loc;
    size_t current;
    Token lasttok;
} LexerError;

struct ParserError2 {
    Token currtoken;
    SV filename;
    ErrorCode code;
    TokenKind expectedkind;
};
typedef struct ParserError2 ParserError;

typedef union {
    LexerError lexer;
    ParserError parser;
} ErrorFrom;

typedef struct {
    Stage stage;
    ErrorFrom from;
} Error;

ErrorFrom errfromlexer(LexerError err);
ErrorFrom errfromparser(ParserError err);
Error error(Stage stage, ErrorFrom from);
void throw(Error err);
void report(Error err);

#endif