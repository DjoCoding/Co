#include "code.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "malloc.h"

CodeGenerator *code(AST *tree) {
    CodeGenerator *c = alloc(sizeof(CodeGenerator));
    c->tree = tree;
    c->gcontext = gcontext();
    return c;
}

void generate_indent(size_t indent) {
    for(size_t i = 0; i < indent; ++i) {
        printf("\t");
    }
}


void generate_funccall(CodeGenerator *this, FunctionCall funccall, size_t indent) {
    assert(false && "not implemented yet");
}

void generate_funcdecl(CodeGenerator *this, FunctionDeclaration funcdecl, size_t indent) {
    assert(false && "not implemented yet");
}

void generate_vardec(CodeGenerator *this, VariableDeclaration vardec, size_t indent) {
    assert(false && "not implemented yet");
}


void generate_node(CodeGenerator *this, Node *n, size_t indent) {
    if(n->kind == NODE_KIND_FUNCTION_CALL) {
        return generate_funccall(this, n->as.funccall, indent);
    }

    if(n->kind == NODE_KIND_FUNCTION_DECLARATION) {
        return generate_funcdecl(this, n->as.funcdecl, indent);
    }

    if(n->kind == NODE_KIND_VARIABLE_DECLARATION) {
        return generate_vardec(this, n->as.vardec, indent);
    }

    //FIXME: error 
    abort();
}

void generate(CodeGenerator *this) {
    for(size_t i = 0; i < this->tree->count; ++i) {
        Node *n = this->tree->items[i];
        generate_node(this, n, 0);
    }
}


