#include "transpiler.h"

#include "utils.h"
#include "malloc.h"

Transpiler *transpiler(const char *filename, const char *output) {
    Transpiler *t = alloc(sizeof(*t));
    t->filename = filename;
    t->output = output;

    t->l = lexer(filename);
    t->p = parser(filename);
    t->c = code(filename, t->output);

    t->tokens = ARRAY(Token);
    t->tree = NULL;

    return t;
}

bool transpiler_read_source(Transpiler *this) {
    char *content = fcontent(this->filename);
    if(!content) { 
        return false;
    }

    this->source = svc(content);
    return true;
}

void transpiler_setup_lexer(Transpiler *this) {
    lexer_set_source(this->l, this->source);
}

void transpiler_lex_source(Transpiler *this) {
    ARRAY_OF(Token) tokens = lex(this->l);
    this->tokens = tokens;
}

void transpiler_setup_parser(Transpiler *this) {
    parser_set_tokens(this->p, this->tokens);
}


void transpiler_parse_tokens(Transpiler *this) {
    AST *tree = parse(this->p);
    this->tree = tree;
}


void transpiler_setup_generator(Transpiler *this) {
    code_set_tree(this->c, this->tree);
}

void transpiler_gencode(Transpiler *this) {
    generate(this->c);
}

// this will be used once the global allocator is made
void transpiler_cleanup(Transpiler *this) {
    (void)this;
}  