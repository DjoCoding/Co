#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"
#include "lexer.h"
#include "parser.h"
#include "logger.h"
#include "code.h"
#include "utils.h"

typedef struct {
    const char *ifile; // input file name
    const char *ofile; // output file name
    bool loglexer;
    bool logparser;
    bool gencode;
} Configuration;

typedef struct {
    int argc;
    int curr;
    char **argv;
} FlagsParser;

void move(FlagsParser *this) {
    this->curr += 1;
}

char *getcurrent(FlagsParser *this) {
    return this->argv[this->curr];
} 

bool isend(FlagsParser *this) {
    return getcurrent(this) == NULL && this->curr == this->argc;
}

// parse the flags
Configuration fparse(FlagsParser *this) {
    // consume the exec name
    move(this);

    Configuration config = {0};

    while(!isend(this)) {
        char *curr = getcurrent(this);
        SV s = svc(curr);
        if(svcmp(s, svc("-i"))) {
            move(this);
            if(isend(this)) {
                fprintf(stderr, "expected input filepath\n");
                exit(EXIT_FAILURE);
            }
            config.ifile = getcurrent(this);
            move(this);
        } else if(svcmp(s, svc("-o"))) {
            move(this);
            if(isend(this)) {
                fprintf(stderr, "expected output filepath\n");
                exit(EXIT_FAILURE);
            }
            config.gencode = true;
            config.ofile = getcurrent(this);
            move(this);
        } else if(svcmp(s, svc("-l"))) {
            move(this);
            config.loglexer = true;
        } else if(svcmp(s, svc("-p"))) {
            move(this);
            config.logparser = true;
        } else if(svcmp(s, svc("-c"))) {
            move(this);
            config.gencode = true;
        } else {
            if(config.ifile) {
                fprintf(stderr, "invalid option %s\n", getcurrent(this));
                exit(EXIT_FAILURE);
            }
            config.ifile = getcurrent(this);
            move(this);
        }
    }

    return config;
}


int main(int argc, char **argv) {
    FlagsParser fparser = {
        .argc = argc,
        .argv = argv
    };

    Configuration global = fparse(&fparser);
    
    // check for input file path
    if(!global.ifile) {
        fprintf(stderr, "input file path required\n");
        exit(EXIT_FAILURE);
    }

    // check for output file path
    if(global.gencode && !global.ofile) {
        fprintf(stderr, "output file path required\n");
        exit(EXIT_FAILURE);
    }

    // getting source code
    char *source = fcontent((const char *)global.ifile);

    // lexing
    Lexer *l = lexer(svc(source));
    lexerofile(l, global.ifile);
    ARRAY_OF(Token) tokens = lex(l);
    if(global.loglexer) log_tokens(tokens);
    

    // parsing
    Parser *p = parser(tokens);
    AST *a = parse(p);
    if(global.logparser) log_ast(a);


    // generating code
    if(global.gencode) {
        CodeGenerator *c = code(a);
        code_setup(c, global.ofile);
        generate(c);
    }

    // cleanup
    free(source);
    return 0;
}