#include "logger.h"

#include <assert.h>
#include "parser.h"

#define INDENT "\t"

void log_node(Node *n, size_t indent);
void log_funccall(FunctionCall funccall, size_t indent);

void log_indent(size_t indent_count) {
    for(size_t i = 0; i < indent_count; ++i) {
        printf(INDENT);
    }
}

void log_type(Type type) {
    if(type.kind == TYPE_KIND_PRE_DEFINED) {
        PreDefinedType predeftype = type.as.predef;
        for(size_t i = 0; i < LENGTH(predeftypes); ++i) {
            if(predeftypes[i].type == predeftype) {
                printf("%s", predeftypes[i].type_as_cstr);
                return;
            }
        }
    }

    if(type.kind == TYPE_KIND_ARRAY) {
        ArrayType arraytype = type.as.array;
        printf("[]");
        return log_type(*arraytype.of);
    }

    assert(false && "not implemented yet");
}

void log_expr(Expression *expr) {
    if(expr->kind == EXPRESSION_KIND_INTEGER) { 
        printf("%d", expr->as.integer);
        return;
    }

    if(expr->kind == EXPRESSION_KIND_STRING) {
        printf("\"" SV_FMT "\"", SV_UNWRAP(expr->as.string));
        return;
    }

    if(expr->kind == EXPRESSION_KIND_BINARY_OPERATION) {
        printf("(");
        log_expr(expr->as.binop.lhs);
        printf(" ");
        printf("%s", mapoptostr(expr->as.binop.op));
        printf(" ");
        log_expr(expr->as.binop.rhs);
        printf(")");
        return;
    }

    if(expr->kind == EXPRESSION_KIND_FUNCTION_CALL) {
        return log_funccall(expr->as.funccall, 0);
    }

    if(expr->kind == EXPRESSION_KIND_VARIABLE) {
        printf(SV_FMT, SV_UNWRAP(expr->as.var.name));
        return;
    }

    if(expr->kind == EXPRESSION_KIND_ARRAY) {
        printf("[");
        for(size_t i = 0; i < expr->as.array.count; ++i) {
            log_expr(expr->as.array.items[i]);
            if(i == expr->as.array.count - 1) { break; }
            printf(",");
            printf(" ");
        }
        printf("]");
        return;
    }

    assert(false && "expression kind logging not implemented yet");
}

void log_params(ARRAY_OF(Parameter) params) {
    if(params.count == 0) { return; }
    
    printf("(");
    for(size_t i = 0; i < params.count; ++i) {
        Parameter p = params.items[i];
        log_type(p.type);
        printf(" ");
        printf(SV_FMT, SV_UNWRAP(p.name));
        if(i == params.count - 1) { break; }
        printf(", "); 
    }
    printf(")");
}

void log_body(Body body, size_t indent) {
    if(body.count == 0) { return; }
    printf("{\n");
    for(size_t i = 0; i < body.count; ++i) {
        Node *n = body.items[i];
        log_indent(indent);
        log_node(n, indent);
        printf("\n");
    }
    log_indent(indent - 1);
    printf("}");
}

void log_args(ARRAY_OF(Argument) args) {
    printf("(");
    for(size_t i = 0; i < args.count; ++i) {
        Argument arg = args.items[i];
        log_expr(arg.e);
        if(i == args.count - 1) { break; }
        printf(",");
        printf(" ");
    }
    printf(")");
}

void log_funcdecl(FunctionDeclaration funcddecl, size_t indent) {
    printf("fn");
    printf(" ");
    printf(SV_FMT, SV_UNWRAP(funcddecl.name));
    log_params(funcddecl.params);
    printf(":");
    printf(" ");
    log_type(funcddecl.rettype);
    printf(" ");
    log_body(funcddecl.body, indent + 1);
}

void log_funccall(FunctionCall funccall, size_t indent) {
    printf(SV_FMT, SV_UNWRAP(funccall.name));
    log_args(funccall.args);
}

void log_vardec(VariableDeclaration vardec) {
    log_type(vardec.type);
    printf(" ");
    printf(SV_FMT, SV_UNWRAP(vardec.name));
    if(!vardec.expr) { return; }
    printf(" ");
    printf("=");
    printf(" ");
    log_expr(vardec.expr);
}

void log_return(Return ret) {
    printf("return");
    printf(" ");
    log_expr(ret.expr);
}

void log_if(If iff, size_t indent) {    
    printf("if");
    
    printf(" ");
    
    printf("(");
    log_expr(iff.e);
    printf(")");
    
    printf(" ");

    if(iff.body.count == 1) {
        return log_node(iff.body.items[0], 0);
    }

    printf(" ");
    
    log_body(iff.body, indent + 1);
}

void log_for(For forr, size_t indent) {
    printf("for");

    printf(" ");

    if(forr.v) { 
        log_vardec(*forr.v);
    }

    if(forr.e) {
        printf(";");
        printf(" ");
        log_expr(forr.e);
    }

    printf(" ");

    log_body(forr.body, indent + 1);    
}

void log_varres(VariableReassignement varres) {
    printf(SV_FMT, SV_UNWRAP(varres.name));
    printf(" ");
    printf("=");
    printf(" ");
    log_expr(varres.expr);
}

void log_node(Node *n, size_t indent) {
    if(n->kind == NODE_KIND_FUNCTION_DECLARATION) {
        return log_funcdecl(n->as.funcdecl, indent);
    }
    
    if(n->kind == NODE_KIND_FUNCTION_CALL) {
        return log_funccall(n->as.funccall, indent);
    }

    if(n->kind == NODE_KIND_VARIABLE_DECLARATION) {
        return log_vardec(n->as.vardec);
    }       

    if(n->kind == NODE_KIND_RETURN_STATEMENT) {
        return log_return(n->as.ret);
    }

    if(n->kind == NODE_KIND_IF) {
        return log_if(n->as.iff, indent);
    }

    if(n->kind == NODE_KIND_FOR) {
        return log_for(n->as.forr, indent);
    }

    if(n->kind == NODE_KIND_VARIABLE_REASSIGNEMENT) {
        return log_varres(n->as.varres);
    }

    assert(false && "node kind logging not implemented yet");
}

void log_ast(AST *a) {
    for(size_t i = 0; i < a->count; ++i) {
        Node *n = a->items[i];
        log_node(n, 0);
        if(i == a->count - 1) { break; }
        printf("\n");
    }
    printf("\n");
}

void log_token(Token t) {
    printf("kind: %s\n", tokenkind_cstr(t.kind));
}

void log_tokens(ARRAY_OF(Token) tokens) {
    for(size_t i = 0; i < tokens.count; ++i) {
        Token t = tokens.items[i];
        log_token(t);
    }
}