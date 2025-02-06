#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"
#include "lexer.h"
#include "parser.h"
#include "logger.h"
#include "code.h"

char *fcontent(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if(!f) { abort(); }

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = alloc(sizeof(char) * (fsize + 1));
    fread(content, sizeof(char), fsize, f);
    content[fsize] = 0;

    fclose(f);
    return content;
}

typedef struct {
    const char *source_code_filepath;
    const char *output_filepath;
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
        if(svcmp(svc(curr), svc("-i"))) {
            move(this);
            if(isend(this)) {
                fprintf(stderr, "expected input filepath\n");
                exit(EXIT_FAILURE);
            }
            config.source_code_filepath = getcurrent(this);
            move(this);
        } else if(svcmp(svc(curr), svc("-o"))) {
            move(this);
            if(isend(this)) {
                fprintf(stderr, "expected output filepath\n");
                exit(EXIT_FAILURE);
            }
            config.output_filepath = getcurrent(this);
            move(this);
        } else {
            if(config.source_code_filepath) {
                fprintf(stderr, "invalid option %s\n", getcurrent(this));
                exit(EXIT_FAILURE);
            }
            config.source_code_filepath = getcurrent(this);
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
    if(!global.source_code_filepath) {
        fprintf(stderr, "input file path required\n");
        exit(EXIT_FAILURE);
    }

    // check for output file path
    if(!global.output_filepath) {
        fprintf(stderr, "output file path required\n");
        exit(EXIT_FAILURE);
    }

    char *source = fcontent((const char *)global.source_code_filepath);

    Lexer *l = lexer(svc(source));
    ARRAY_OF(Token) tokens = lex(l);
    printf("[LEXING] passed\n");

    Parser *p = parser(tokens);
    AST *a = parse(p);
    printf("[PARSING] passed\n");

    CodeGenerator *c = code(a);
    code_setfilepath(c, global.output_filepath);
    generate(c);

    free(source);
    return 0;
}