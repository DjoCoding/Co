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

void print_tokens(ARRAY_OF(Token) tokens) {
    for(size_t i = 0; i < tokens.count; ++i) {
        Token t = tokens.items[i];
        printf("kind: %s\n", tokenkind_cstr(t.kind));
    }
}

int main(void) {
    char *source = fcontent("./main.co");

    {
        Lexer *l = lexer(svc(source));
        ARRAY_OF(Token) tokens = lex(l);
        // print_tokens(tokens);
        printf("[LEXING] passed\n");

        Parser *p = parser(tokens);
        AST *a = parse(p);
        // log_ast(a);
        printf("[PARSING] passed\n");

        CodeGenerator *c = code(a);
        generate(c);
    }

    free(source);
    return 0;
}