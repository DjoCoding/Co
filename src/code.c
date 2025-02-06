#include "code.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "malloc.h"

// this macro will get the this object in the current context as not declared dependency
#define WRITE_CODE(...) fprintf(this->state.f, __VA_ARGS__)

CodeGenerator *code(AST *tree) {
    CodeGenerator *c = alloc(sizeof(CodeGenerator));
    c->tree = tree;
    c->gcontext = gcontext();
    c->state = (CodeGeneratorState) {0};
    c->state.f = stdout;
    return c;
}

bool code_open_outfile(CodeGenerator *this) {
    FILE *f = fopen(this->state.filpath, "w");
    if(!f) { return false; }
    this->state.f = f;
    return true;
}

void code_setfilepath(CodeGenerator *this, const char *filepath) {
    this->state.filpath = filepath;
    bool success = code_open_outfile(this);
    if(!success) {
        //FIXME: error 
        // RAISE SYSCALL FAILED ERROR
        abort();
    }
}

void generate_indent(CodeGenerator *this) {
    for(size_t i = 0; i < this->state.indent; ++i) {
        WRITE_CODE("\t");
    }
}

void generate_funccall(CodeGenerator *this, FunctionCall funccall, bool inexpr);
void generate_varvalue(CodeGenerator *this, SV varname);
void generate_funcdecl(CodeGenerator *this, FunctionDeclaration funcdecl);
void generate_node(CodeGenerator *this, Node *n, bool infuncbody);
void generate_expression_code(CodeGenerator *this, Expression *e);
void generate_type(CodeGenerator *this, Type type);

void generate_binop_expression_code(CodeGenerator *this, BinOpExpression binop) {
    WRITE_CODE("(");
    generate_expression_code(this, binop.lhs);
    WRITE_CODE(" ");
    WRITE_CODE("%s", mapoptostr(binop.op));
    WRITE_CODE(" ");
    generate_expression_code(this, binop.rhs);
    WRITE_CODE(")");
}

void generate_expression_code(CodeGenerator *this, Expression *e) {
    if(e->kind == EXPRESSION_KIND_INTEGER) { 
        WRITE_CODE("%d", e->as.integer);
        return;
    }

    if(e->kind == EXPRESSION_KIND_STRING) {
        WRITE_CODE("\"" SV_FMT "\"", SV_UNWRAP(e->as.string));
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

    WRITE_CODE(SV_FMT, SV_UNWRAP(funccall.name));
    WRITE_CODE("(");
    for(size_t i = 0; i < funccall.args.count; ++i) {
        Argument arg = funccall.args.items[i];
        generate_expression_code(this, arg.e);
        if(i == funccall.args.count - 1)  { break; }
        WRITE_CODE(",");
        WRITE_CODE(" ");
    }
    WRITE_CODE(")");
}

void generate_predef_type_code(CodeGenerator *this, PreDefinedType predef) {
    switch(predef) {
        case PRE_DEFINED_TYPE_INT:
            WRITE_CODE("int");
            break;
        case PRE_DEFINED_TYPE_VOID:
            WRITE_CODE("void");
            break;
        default:
        assert(false && "unreachable");
    }
}

void generate_funcdecl_param_code(CodeGenerator *this, Parameter p) {
    generate_type(this, p.type);
    WRITE_CODE(" ");
    WRITE_CODE(SV_FMT, SV_UNWRAP(p.name));
}

void generate_funcdecl_params_code(CodeGenerator *this, ARRAY_OF(Parameter) params) {
    if(params.count == 0) { 
        WRITE_CODE("void");
        return;
    }

    for(size_t i = 0; i < params.count; ++i) {
        Parameter p = params.items[i];
        generate_funcdecl_param_code(this, p);
        if(i == params.count - 1) { break; }
        WRITE_CODE(",");
        WRITE_CODE(" ");
    }
}

void generate_body(CodeGenerator *this, Body body) {
    for(size_t i = 0; i < body.count; ++i) {
        Node *n = body.items[i];
        generate_node(this, n, true);
        WRITE_CODE("\n");
    }
}

void generate_funcdecl_code(CodeGenerator *this, FunctionDeclaration funcdecl) {
    generate_indent(this);

    generate_type(this, funcdecl.rettype);

    WRITE_CODE(" ");
    
    WRITE_CODE(SV_FMT, SV_UNWRAP(funcdecl.name));
    
    WRITE_CODE("(");
    generate_funcdecl_params_code(this, funcdecl.params);
    WRITE_CODE(")");
    WRITE_CODE(" ");
    
    WRITE_CODE("{\n");
    
    this->state.indent += 1;
    generate_body(this, funcdecl.body);
    this->state.indent -= 1;
    
    WRITE_CODE("}\n");
}

void generate_vardec_code_inline(CodeGenerator *this, VariableDeclaration vardec) {
    generate_type(this, vardec.type);
    
    WRITE_CODE(" ");

    WRITE_CODE(SV_FMT, SV_UNWRAP(vardec.name));

    if(vardec.expr) {
        WRITE_CODE(" ");
        WRITE_CODE("=");
        WRITE_CODE(" ");
        generate_expression_code(this, vardec.expr);
    }
}

void generate_vardec_code(CodeGenerator *this, VariableDeclaration vardec) {
    generate_indent(this);
    generate_vardec_code_inline(this, vardec);
    WRITE_CODE(";");
}


void generate_varvalue(CodeGenerator *this, SV varname) {
    ContextVariable *var = gcontext_findvar(this->gcontext, varname);
    if(!var) {
        //FIXME: error 
        // VARIABLE NOT DEFINED IN THE CONTEXT ERROR 
        abort();    
    }

    WRITE_CODE(SV_FMT, SV_UNWRAP(var->name));
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

void crash_on_varfound(CodeGenerator *this, SV name) {
    ContextVariable *var = gcontext_findvar(this->gcontext, name);
    if(var) {
        //FIXME: error
        // VARIABLE ALREADY EXISTS ERROR
        abort();    
    }
}

void generate_vardec(CodeGenerator *this, VariableDeclaration vardec) {
    crash_on_varfound(this, vardec.name);
    gcontext_pushvar(this->gcontext, contextvar(vardec));
    generate_vardec_code(this, vardec);
}

void generate_vardec_inline(CodeGenerator *this, VariableDeclaration vardec) {
    crash_on_varfound(this, vardec.name);
    gcontext_pushvar(this->gcontext, contextvar(vardec));
    generate_vardec_code_inline(this, vardec);
}

void generate_type(CodeGenerator *this, Type type) {
    if(type.kind == TYPE_KIND_PRE_DEFINED) {
        return generate_predef_type_code(this, type.as.predef);
    }

    //FIXME: error 
    // INVALID TYPE
    abort();
}

void generate_return(CodeGenerator *this, Return ret) {
    generate_indent(this);
    WRITE_CODE("return");
    WRITE_CODE(" ");
    generate_expression_code(this, ret.expr);
    WRITE_CODE(";");
}

void generate_if(CodeGenerator *this, If iff) {
    generate_indent(this);
    
    WRITE_CODE("if");
    
    WRITE_CODE("(");
    generate_expression_code(this, iff.e);
    WRITE_CODE(")");

    WRITE_CODE(" ");

    WRITE_CODE("{\n");

    // push a new context
    gcontext_push(this->gcontext);

    this->state.indent += 1;
    generate_body(this, iff.body);
    this->state.indent -= 1;

    // pop the context
    gcontext_pop(this->gcontext);

    generate_indent(this);
    WRITE_CODE("}");
}

void generate_for(CodeGenerator *this, For forr) {
    generate_indent(this);
    
    WRITE_CODE("for");

    // push a new context
    gcontext_push(this->gcontext);
    
    WRITE_CODE("(");
    
    if(forr.v) {
        generate_vardec_inline(this, *forr.v);
    }

    WRITE_CODE(";");
    WRITE_CODE(" ");

    if(forr.e) {
        generate_expression_code(this, forr.e);
    }

    WRITE_CODE(";");
    WRITE_CODE(" ");

    WRITE_CODE(")");

    WRITE_CODE(" ");

    WRITE_CODE("{\n");

    this->state.indent += 1;
    generate_body(this, forr.body);
    this->state.indent -= 1;

    // pop the context
    gcontext_pop(this->gcontext);

    generate_indent(this);
    WRITE_CODE("}");
}

void generate_varres(CodeGenerator *this, VariableReassignement varres) {
    generate_indent(this);
    WRITE_CODE(SV_FMT, SV_UNWRAP(varres.name));
    WRITE_CODE(" ");
    WRITE_CODE("=");
    WRITE_CODE(" ");
    generate_expression_code(this, varres.expr);
    WRITE_CODE(";");
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

    if(n->kind == NODE_KIND_RETURN_STATEMENT) {
        return generate_return(this, n->as.ret);
    }

    if(n->kind == NODE_KIND_IF) {
        return generate_if(this, n->as.iff);
    }

    if(n->kind == NODE_KIND_FOR) {
        return generate_for(this, n->as.forr);
    }

    if(n->kind == NODE_KIND_VARIABLE_REASSIGNEMENT) {
        return generate_varres(this, n->as.varres);
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
        WRITE_CODE("\n");
    }
}


