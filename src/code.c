#include "code.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "malloc.h"

CodeGenerator *code(AST *tree) {
    CodeGenerator *c = alloc(sizeof(CodeGenerator));
    c->tree = tree;
    c->gcontext = gcontext();
    c->state = (CodeGeneratorState) {0};
    return c;
}

void generate_indent(CodeGenerator *this) {
    for(size_t i = 0; i < this->state.indent; ++i) {
        printf("\t");
    }
}

void generate_funccall(CodeGenerator *this, FunctionCall funccall, bool inexpr);
void generate_varvalue(CodeGenerator *this, SV varname);
void generate_funcdecl(CodeGenerator *this, FunctionDeclaration funcdecl);
void generate_node(CodeGenerator *this, Node *n, bool infuncbody);
void generate_expression_code(CodeGenerator *this, Expression *e);
void generate_type(CodeGenerator *this, Type type);

void generate_binop_expression_code(CodeGenerator *this, BinOpExpression binop) {
    printf("(");
    generate_expression_code(this, binop.lhs);
    printf(" ");
    printf("%s", mapoptostr(binop.op));
    printf(" ");
    generate_expression_code(this, binop.rhs);
    printf(")");
}

void generate_expression_code(CodeGenerator *this, Expression *e) {
    if(e->kind == EXPRESSION_KIND_INTEGER) { 
        printf("%d", e->as.integer);
        return;
    }

    if(e->kind == EXPRESSION_KIND_STRING) {
        printf("\"" SV_FMT "\"", SV_UNWRAP(e->as.string));
        return;
    }

    if(e->kind == EXPRESSION_KIND_BINARY_OPERATION) {
        return generate_binop_expression_code(this, e->as.binop);
    }

    if(e->kind == EXPRESSION_KIND_FUNCTION_CALL) {
        return generate_funccall(this, e->as.funccall, true);
    }

    if(e->kind == EXPRESSION_KIND_VARIABLE) {
        return generate_varvalue(this, e->as.var.name);
    }

    assert(false && "expression kind logging not implemented yet");
}

void generate_funccall_code(CodeGenerator *this, FunctionCall funccall, bool inexpr) {
    if(!inexpr) { 
        generate_indent(this);
    }

    printf(SV_FMT, SV_UNWRAP(funccall.name));
    printf("(");
    for(size_t i = 0; i < funccall.args.count; ++i) {
        Argument arg = funccall.args.items[i];
        generate_expression_code(this, arg.e);
        if(i == funccall.args.count - 1)  { break; }
        printf(",");
        printf(" ");
    }
    printf(")");
}

void generate_predef_type_code(PreDefinedType predef) {
    switch(predef) {
        case PRE_DEFINED_TYPE_INT:
            printf("int");
            break;
        case PRE_DEFINED_TYPE_VOID:
            printf("void");
            break;
        default:
        assert(false && "unreachable");
    }
}

void generate_funcdecl_param_code(CodeGenerator *this, Parameter p) {
    generate_type(this, p.type);
    printf(" ");
    printf(SV_FMT, SV_UNWRAP(p.name));
}

void generate_funcdecl_params_code(CodeGenerator *this, ARRAY_OF(Parameter) params) {
    if(params.count == 0) { 
        printf("void");
        return;
    }

    for(size_t i = 0; i < params.count; ++i) {
        Parameter p = params.items[i];
        generate_funcdecl_param_code(this, p);
        if(i == params.count - 1) { break; }
        printf(", ");
        printf(" ");
    }
}

void generate_funcdecl_body(CodeGenerator *this, Body body) {
    for(size_t i = 0; i < body.count; ++i) {
        Node *n = body.items[i];
        generate_node(this, n, true);
        printf("\n");
    }
}

void generate_funcdecl_code(CodeGenerator *this, FunctionDeclaration funcdecl) {
    generate_indent(this);

    generate_type(this, funcdecl.rettype);

    printf(" ");
    
    printf(SV_FMT, SV_UNWRAP(funcdecl.name));
    
    printf("(");
    generate_funcdecl_params_code(this, funcdecl.params);
    printf(")");
    printf(" ");
    
    printf("{\n");
    
    this->state.indent += 1;
    generate_funcdecl_body(this, funcdecl.body);
    this->state.indent -= 1;
    
    printf("}\n");
}

void generate_vardec_code(CodeGenerator *this, VariableDeclaration vardec) {
    generate_indent(this);

    generate_type(this, vardec.type);
    
    printf(" ");

    printf(SV_FMT, SV_UNWRAP(vardec.name));

    if(vardec.expr) {
        printf(" ");
        printf("=");
        printf(" ");
        generate_expression_code(this, vardec.expr);
    }

    printf(";");
}

void generate_varvalue(CodeGenerator *this, SV varname) {
    ContextVariable *var = gcontext_findvar(this->gcontext, varname);
    if(!var) {
        //FIXME: error 
        // VARIABLE NOT DEFINED IN THE CONTEXT ERROR 
        abort();    
    }

    printf(SV_FMT, SV_UNWRAP(var->name));
} 

void generate_funccall(CodeGenerator *this, FunctionCall funccall, bool inexpr) {
    ContextFunction *func = gcontext_findfunc(this->gcontext, funccall.name);
    if(!func) {
        //FIXME: error 
        // FUNCTION NOT DEFINED IN THE CONTEXT ERROR 
        abort();
    }

    if(func->params.count != funccall.args.count) { 
        //FIXME: error 
        // EXPECTED (func->params.count) arguments but got (funccall.args.count)
        abort();
    }

    for(size_t i = 0; i < func->params.count; ++i) {
        Parameter p = func->params.items[i];
        Argument arg = funccall.args.items[i];
        if(!typecheck(p.type, arg.e)) { 
            //FIXME: error
            // EXPECTED EXPRESSION TO BE OF TYPE (p.type) BUT GOT (typeof(arg.e))
            abort();
        }
    }

    generate_funccall_code(this, funccall, inexpr);
}


void generate_funcdecl(CodeGenerator *this, FunctionDeclaration funcdecl) {
    ContextFunction *func = gcontext_findfunc(this->gcontext, funcdecl.name);
    if(func) {
        //FIXME: error
        // FUNCTION ALREADY EXISTS ERROR
        abort();
    }

    // push the function in the current context 
    gcontext_pushfunc(this->gcontext, contextfunc(funcdecl));

    // push a new context for the function
    gcontext_push(this->gcontext);

    for(size_t i = 0; i < funcdecl.params.count; ++i) {
        Parameter p = funcdecl.params.items[i];
        gcontext_pushvar(this->gcontext, (ContextVariable) {
            .name = p.name, 
            .type = p.type
        });
    }

    // generate the function declaration source code
    generate_funcdecl_code(this, funcdecl);

    // pop the function context
    gcontext_pop(this->gcontext);
}

void generate_vardec(CodeGenerator *this, VariableDeclaration vardec) {
    ContextVariable *var = gcontext_findvar(this->gcontext, vardec.name);
    if(var) {
        //FIXME: error
        // VARIABLE ALREADY EXISTS ERROR
        abort();    
    }

    gcontext_pushvar(this->gcontext, contextvar(vardec));
    generate_vardec_code(this, vardec);
}

void generate_type(CodeGenerator *this, Type type) {
    if(type.kind == TYPE_KIND_PRE_DEFINED) {
        return generate_predef_type_code(type.as.predef);
    }

    //FIXME: error 
    // INVALID TYPE
    abort();
}

void generate_return(CodeGenerator *this, Return ret) {
    generate_indent(this);
    printf("return");
    printf(" ");
    generate_expression_code(this, ret.expr);
    printf(";");
}

void generate_node(CodeGenerator *this, Node *n, bool infuncbody) {
    if(n->kind == NODE_KIND_FUNCTION_CALL) {
        return generate_funccall(this, n->as.funccall, false);
    }

    if(n->kind == NODE_KIND_FUNCTION_DECLARATION) {
        return generate_funcdecl(this, n->as.funcdecl);
    }

    if(n->kind == NODE_KIND_VARIABLE_DECLARATION) {
        return generate_vardec(this, n->as.vardec);
    }

    if(infuncbody) {
        if(n->kind == NODE_KIND_RETURN_STATEMENT) {
            return generate_return(this, n->as.ret);
        }
    }

    //FIXME: error 
    // INVALID
    abort();
}

void generate(CodeGenerator *this) {
    // push the global transpiler context
    gcontext_push(this->gcontext);

    for(size_t i = 0; i < this->tree->count; ++i) {
        Node *n = this->tree->items[i];
        generate_node(this, n, false);
        printf("\n");
    }
}


