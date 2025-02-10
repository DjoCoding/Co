#include "coerror.h"

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


Error error(Stage stage, ErrorFrom from) {
    return(Error) {
        .stage = stage, 
        .from = from
    };
}

char *allocmes() {
    char *m = alloc(sizeof(char) * ERROR_MAX_LENGTH);
    m[ERROR_MAX_LENGTH - 1] = 0;
    return m;
}

void throw_lexerr(LexerError err) {
    // for message: global message displayed on the screen
    char *m = allocmes();

    // view on the message, to perform operations on it
    SV mview = svc(m);

    append(mview, SV_FMT ":%zu:%zu:", SV_UNWRAP(err.filename), err.loc.line, err.loc.offset);

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
    
    fprintf(stderr, SV_FMT "\n", SV_UNWRAP(mview));
    free(m);

    exit(EXIT_FAILURE);
}

void throw_parserr(ParserError err) {
    char *m = allocmes();
    SV mview = svc(m);

    append(mview, SV_FMT ":%zu:%zu:", SV_UNWRAP(err.filename), err.currtoken.loc.line, err.currtoken.loc.offset);

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

    fprintf(stderr, SV_FMT "\n", SV_UNWRAP(mview));
    free(m);

    exit(EXIT_FAILURE);
}

void throw_contexterr(ContextError err) {
    char *m = allocmes();
    SV mview = svc(m);

    append(mview, SV_FMT ":", SV_UNWRAP(err.filename));

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
        default:
            UNREACHABLE()
    }

    fprintf(stderr, SV_FMT "\n", SV_UNWRAP(mview));
    free(m);

    exit(EXIT_FAILURE);
}


void throw(Error err) {
    switch(err.stage) {
        case LEXER: 
            return throw_lexerr(err.from.lexer);
        case PARSER:
            return throw_parserr(err.from.parser);
        case CONTEXT:
            return throw_contexterr(err.from.context);
        default:
            TODO("throw errors not fully implemented");
    }
}

void report(Error err) {
    (void)err;
}
