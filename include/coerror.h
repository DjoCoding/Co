#ifndef COERROR_H
#define COERROR_H

#include <stdlib.h>
#include "token.h"
#include "sv.h"
#include "shared.h"

#define TODO(...) { fprintf(stdout, "%s:%d:%s -> marked as todo\n", __FILE__, __LINE__, __FUNCTION__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); abort(); }
#define THROW(...) { fprintf(stderr, "ERROR: ", __VA_ARGS__); exit(EXIT_FAILURE); }
#define UNREACHABLE() { fprintf(stderr, "%s:%d:%s -> unreachable\n", __FILE__, __LINE__, __FUNCTION__); exit(EXIT_FAILURE); }

typedef enum {
    LEXER = 0,
    PARSER,
    CONTEXT,
    TYPE,
    CODE_GENERATOR
} Stage;

typedef enum {
    ERROR_CODE_NONE = 0,

    // lexing related error codes
    INVALID_TOKEN,
    INVALID_STRING,
    UNKNOWN_TYPE_NAME,

    // parsing related error codes
    INVALID_START_OF_STATEMENT,
    EXPECTED_TOKEN_KIND_BUT_FOUND_ANOTHER,
    EXPECTED_EXPRESSION,

    // context related error codes
    FUNCTION_ALREADY_DECLARED,
    VARIABLE_ALREADY_DECLARED,
    VARIABLE_NOT_DECLARED,
    FUNCTION_NOT_DECLARED,
    INVALID_NUMBER_OF_PARAMS,

    // type checking error codes
    INVALID_EXPRESSION_TYPE_ON_ASSIGNEMENT,
    TYPE_ERROR,
    INVALID_OPERATION_BETWEEN_TYPES,
    INVALID_RETURN_TYPE_OF_FUNCTION
} ErrorCode;

typedef struct {
    ErrorCode code;
    Location loc;
    size_t current;
    Token lasttok;
} LexerError;

typedef struct {
    Token currtoken;
    ErrorCode code;
    TokenKind expectedkind;
} ParserError;

typedef struct {
    ErrorCode code;
    SV name;
} ContextError;

typedef struct {
    ErrorCode code;
    SV expectedtype;
    SV foundtype;
} TypeError;

typedef union {
    LexerError lexer;
    ParserError parser;
    ContextError context;
    TypeError type;
} ErrorFrom;

typedef struct {
    Stage stage;
    ErrorFrom from;
    SV filename;
} Error;

ErrorFrom errfromlexer(LexerError err);
ErrorFrom errfromparser(ParserError err);
ErrorFrom errfromcontext(ContextError err);
ErrorFrom errfromtype(TypeError err);
Error error(Stage stage, ErrorFrom from, SV filename);
void throw(Error err);
void report(Error err);

#endif