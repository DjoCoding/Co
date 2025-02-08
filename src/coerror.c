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

Error error(Stage stage, ErrorFrom from) {
    return(Error) {
        .stage = stage, 
        .from = from
    };
}

void throw_lexerr(LexerError err) {
    // for message: global message displayed on the screen
    char *m = alloc(sizeof(char) * ERROR_MAX_LENGTH);
    
    // view on the message, to perform operations on it
    SV mview = svc(m);

    append(mview, SV_FMT ":%zu:%zu:", SV_UNWRAP(err.filename), err.line, err.offset);

    append(mview, " ");
    append(mview, red("error:"));
    append(mview, " ");

    switch(err.code) {
        case INVALID_TOKEN: 
            append(mview, ": error: invalid token `" SV_FMT "`", SV_UNWRAP(err.lasttok.value));
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

void throw(Error err) {
    switch(err.stage) {
        case LEXER: 
            return throw_lexerr(err.from.lexer);
        default:
            TODO("throw errors not fully implemented");
    }
}

void report(Error err) {
    (void)err;
}
