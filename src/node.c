#include "node.h"
#include "malloc.h"

Node *node(NodeKind kind, NodeAs as) {
    Node *n = alloc(sizeof(Node));
    n->kind = kind;
    n->as = as;
    return n;
}  

Node *nodeas_funcdecl(FunctionDeclaration funcdecl) {
    Node *n = node(
        NODE_KIND_FUNCTION_DECLARATION,
        (NodeAs) {
            .funcdecl = funcdecl
        }
    );
    return n;
}


Node *nodeas_funccall(FunctionCall funccall) {
    Node *n = node(
        NODE_KIND_FUNCTION_CALL,
        (NodeAs) {
            .funccall = funccall
        }
    );
    return n;
}

Node *nodeas_vardec(VariableDeclaration vardec) {
    Node *n = node(
        NODE_KIND_VARIABLE_DECLARATION,
        (NodeAs) {
            .vardec = vardec
        }
    );
    return n;
}

Node *nodeas_return(Expression *ret) {
    Node *n = node(
        NODE_KIND_RETURN_STATEMENT,
        (NodeAs) {
            .ret = (Return) { 
                ret
            }
        }
    );
    return n;
}

Expression *expr(ExpressionKind kind, ExpressionAs as) {
    Expression *e = alloc(sizeof(Expression));
    e->kind = kind;
    e->as = as;
    return e;
}

Expression *expras_binop(BinOpExpression binop) {
    return expr(
        EXPRESSION_KIND_BINARY_OPERATION,
        (ExpressionAs) {
            .binop = binop
        }
    );
} 

Expression *expras_string(SV string) {
    return expr(
        EXPRESSION_KIND_STRING,
        (ExpressionAs) {
            .string = string
        }
    );
}

Expression *expras_integer(int integer) {
    return expr(
        EXPRESSION_KIND_INTEGER,
        (ExpressionAs) {
            .integer = integer
        }
    );
}

Expression *expras_funccall(FunctionCall funccall) {
    return expr(
        EXPRESSION_KIND_FUNCTION_CALL,
        (ExpressionAs) { 
            .funccall = funccall
        }
    );
}

Expression *expras_var(Variable var) {
    return expr(
        EXPRESSION_KIND_VARIABLE,
        (ExpressionAs) {
            .var = var
        }
    );
}
