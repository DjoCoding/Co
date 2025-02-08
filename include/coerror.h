#ifndef COERROR_H
#define COERROR_H

#include "token.h"
#include "sv.h"

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
} ErrorCode;

typedef struct {
    ErrorCode code;
    SV filename;
    size_t line;
    size_t offset;
    size_t current;
    Token lasttok;
} LexerError;

typedef struct {
    LexerError lexer;
} ErrorFrom;

typedef struct {
    Stage stage;
    ErrorFrom from;
} Error;

ErrorFrom errfromlexer(LexerError err);
Error error(Stage stage, ErrorFrom from);
void throw(Error err);
void report(Error err);

#endif