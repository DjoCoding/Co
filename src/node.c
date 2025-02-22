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

Node *nodeas_if(If iff) {
    Node *n = node(
        NODE_KIND_IF,
        (NodeAs) {
            .iff = iff
        }
    );
    return n;
}

Node *nodeas_for(For forr) {
    Node *n = node(
        NODE_KIND_FOR,
        (NodeAs) {
            .forr = forr
        }
    );
    return n;
}

Node *nodeas_varres(VariableReassignement varres) {
    Node *n = node(
        NODE_KIND_VARIABLE_REASSIGNEMENT,
        (NodeAs) {
            .varres = varres
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

Expression *expras_array(Array array) {
    return expr(
      EXPRESSION_KIND_ARRAY,
      (ExpressionAs) {
        .array = array
      }
    );
}

Type typeas_predef(PreDefinedType predef) {
    return (Type) {
        .kind = TYPE_KIND_PRE_DEFINED,
        .as = {
            .predef = predef
        }
    };
}

VariableDeclaration *vardec(Type type, SV name, Expression *value) {
    VariableDeclaration *var = alloc(sizeof(VariableDeclaration));
    var->type = type;
    var->name = name;
    var->expr = value;
    return var;
}

Type arrayof(Type of) {
    Type *t = alloc(sizeof(*t));
    *t = of;
    return (Type) {
        .kind = TYPE_KIND_ARRAY,
        .as = (TypeAs) {
            .array = (ArrayType) {
                .of = t
            }
        }
    };
}
