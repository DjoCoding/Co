#include <stdio.h>
#include <stdlib.h>

#include "da.h"
#include "malloc.h"
#include "logger.h"
#include "utils.h"
#include "transpiler.h"

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

    Transpiler *t = transpiler(global.ifile, global.ofile);
    if(!transpiler_read_source(t)) {
        perror("reading source code failed");
        exit(EXIT_FAILURE);
    }

    transpiler_setup_lexer(t);
    transpiler_lex_source(t);
    if(global.loglexer) {
        ARRAY_OF(Token) tokens = t->tokens;
        log_tokens(tokens);
    }

    transpiler_setup_parser(t);
    transpiler_parse_tokens(t);
    if(global.logparser) {
        AST *tree = t->tree;
        log_ast(tree);
    }

    if(global.gencode) {    
        transpiler_setup_generator(t);
        transpiler_gencode(t);
    }

    return 0;
}