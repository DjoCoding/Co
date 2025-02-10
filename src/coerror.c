#include "coerror.h"

#include <stdio.h>
#include <stdlib.h>
#include "malloc.h"
#include "sv.h"

#define ERROR_MAX_LENGTH 4 * 1024

// escape colors
#define BLUE "\x1b[36m"
#define RED "\x1b[31m"
#define RESET "\x1b[0m"

#define append(mview, ...) \
    do {   \
        char buffer[ERROR_MAX_LENGTH] = {0}; \
        snprintf(buffer, ERROR_MAX_LENGTH, __VA_ARGS__); \
        mview = svappend(mview, svc(buffer)); \
    } while(0)

#define red(s) RED s RESET 
#define blue(s) BLUE s RESET

ErrorFrom errfromlexer(LexerError err) {
    return (ErrorFrom) {
        .lexer = err
    };
} 

ErrorFrom errfromparser(ParserError err) {
    return (ErrorFrom) {
        .parser = err
    };
}

ErrorFrom errfromcontext(ContextError err) {
    return (ErrorFrom) {
        .context = err,
    };
}

ErrorFrom errfromtype(TypeError err) {
    return (ErrorFrom) {
        .type = err
    };
}


Error error(Stage stage, ErrorFrom from, SV filename) {
    return(Error) {
        .stage = stage, 
        .from = from,
        .filename = filename
    };
}

char *allocmes() {
    char *m = alloc(sizeof(char) * ERROR_MAX_LENGTH);
    m[ERROR_MAX_LENGTH - 1] = 0;
    return m;
}

SV throw_lexerr(SV mview, LexerError err) {
    append(mview, ":%zu:%zu:", err.loc.line, err.loc.offset);

    append(mview, " ");
    append(mview, red("error:"));
    append(mview, " ");

    switch(err.code) {
        case INVALID_TOKEN: 
            append(mview, "invalid token `" SV_FMT "`", SV_UNWRAP(err.lasttok.value));
            break;
        case INVALID_STRING:
            {
                bool islong = err.lasttok.value.count > 10;
                SV s = islong ? sv(err.lasttok.value.content, 5) : err.lasttok.value;

                if(islong) { append(mview, "invalid string " red("\"" SV_FMT "...\""), SV_UNWRAP(s)); } 
                else { append(mview, "invalid string " red("\"" SV_FMT "\""), SV_UNWRAP(s)); }

                append(mview, " ");
                append(mview, "->");
                append(mview, " ");

                append(mview, blue("expected `\"` at the end"));
            }
            break;
        default:
            UNREACHABLE();
    }

    return mview;
}

SV throw_parserr(SV mview, ParserError err) {
    append(mview, ":%zu:%zu:", err.currtoken.loc.line, err.currtoken.loc.offset);

    append(mview, " ");
    append(mview, red("error:"));
    append(mview, " ");

    switch (err.code) {
        case UNKNOWN_TYPE_NAME:
            append(mview, "unknown type name `" red(SV_FMT) "`", SV_UNWRAP(err.currtoken.value));
            break;
        case EXPECTED_TOKEN_KIND_BUT_FOUND_ANOTHER:
            append(
                mview, 
                "expected `" blue("%s") "` token but found token `" red("%s") "` instead (" red(SV_FMT) ")",
                tokenkind_cstr(err.expectedkind),
                tokenkind_cstr(err.currtoken.kind),
                SV_UNWRAP(err.currtoken.value)
            );
            break;
        case EXPECTED_EXPRESSION:   
            append(
                mview, 
                "expected expression instead of `" red(SV_FMT) "`",
                SV_UNWRAP(err.currtoken.value)
            );
            break;
        case INVALID_START_OF_STATEMENT:
            append(
                mview,
                "invalid start of statement, starts with `" red(SV_FMT) "`",
                SV_UNWRAP(err.currtoken.value)
            );
            break;
        default:
            UNREACHABLE()
    }

    return mview;
}

SV throw_contexterr(SV mview, ContextError err) {
    append(mview, " ");
    append(mview, red("error:"));
    append(mview, " ");

    switch (err.code) {
        case FUNCTION_ALREADY_DECLARED:
            append(mview, "function `" red(SV_FMT) "` already declared within the same scope", SV_UNWRAP(err.name));
            break;
        case VARIABLE_ALREADY_DECLARED:
            append(mview, "variable `" red(SV_FMT) "` already declared within the same scope", SV_UNWRAP(err.name));
            break;
        case VARIABLE_NOT_DECLARED:
            append(
                mview, 
                "variable `" red(SV_FMT) "` not yet defined", 
                SV_UNWRAP(err.name)
            );
            break;
        case FUNCTION_NOT_DECLARED:
            append(
                mview,
                "function `" red(SV_FMT) "` not yet defined", 
                SV_UNWRAP(err.name)
            );
            break;
        case INVALID_NUMBER_OF_PARAMS:  
            append(
                mview, 
                "invalid number of arguments passed to function `" red(SV_FMT) "`",
                SV_UNWRAP(err.name)
            );
            break;
        default:
            UNREACHABLE()
    }

    return mview;
}

SV throw_typerr(SV mview, TypeError err) {
    switch(err.code) {
        case TYPE_ERROR:
            append(
                mview, 
                "invalid type, expected `" blue(SV_FMT) "` but got `" red(SV_FMT) "`", 
                SV_UNWRAP(err.expectedtype),
                SV_UNWRAP(err.foundtype) 
            );
            break;
        case INVALID_OPERATION_BETWEEN_TYPES:
            append(
                mview, 
                "invalid operation between `" red(SV_FMT) "` and `" red(SV_FMT) "` types",
                SV_UNWRAP(err.expectedtype),
                SV_UNWRAP(err.foundtype)
            );
            break;
        default:
            UNREACHABLE();
    }

    return mview;
}


void throw(Error err) {
    char *m = allocmes();
    SV mview = svc(m);

    append(mview, SV_FMT, SV_UNWRAP(err.filename));

    switch(err.stage) {
        case LEXER: 
            mview = throw_lexerr(mview, err.from.lexer);
            break;
        case PARSER:
            mview = throw_parserr(mview, err.from.parser);
            break;
        case CONTEXT:
            mview = throw_contexterr(mview, err.from.context);
            break;
        case TYPE:
            mview = throw_typerr(mview, err.from.type);
            break;
        default:
            TODO("throw errors not fully implemented");
    }

    fprintf(stderr, SV_FMT "\n", SV_UNWRAP(mview));
    free(m);
    exit(EXIT_FAILURE);
}

void report(Error err) {
    (void)err;
}
