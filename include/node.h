#ifndef NODE_H
#define NODE_H

#include "sv.h"
#include "da.h"

typedef enum {
    NODE_KIND_NONE = 0,
    NODE_KIND_FUNCTION_DECLARATION,
    NODE_KIND_FUNCTION_CALL,
    NODE_KIND_VARIABLE_DECLARATION,
    NODE_KIND_RETURN_STATEMENT
} NodeKind;

typedef struct Node Node;
typedef struct Expression Expression;

typedef struct {
    Node **items;
    size_t count;
    size_t size;
} Body;

typedef enum {
    TYPE_INT = 0,
    PRE_DEFINED_TYPE_COUNT
} PreDefinedType;  

typedef enum {
    TYPE_KIND_PRE_DEFINED
} TypeKind;

typedef union {
    PreDefinedType predef;
} TypeAs;

typedef struct {
    TypeKind kind;
    TypeAs as;
} Type; 

typedef struct {
    Type type;
    SV name;
} Parameter;

DEF_ARRAY(Parameter);

typedef struct {
    Expression *e;
} Argument; 

DEF_ARRAY(Argument);

typedef struct {
    SV name;
    ARRAY_OF(Parameter) params;
    Body body;
} FunctionDeclaration;

typedef struct {
    SV name;
    ARRAY_OF(Argument) args;
} FunctionCall;

typedef enum {
    EXPRESSION_KIND_NONE = 0,
    EXPRESSION_KIND_BINARY_OPERATION, 
    EXPRESSION_KIND_INTEGER,
    EXPRESSION_KIND_STRING,
    EXPRESSION_KIND_FUNCTION_CALL,
    EXPRESSION_KIND_VARIABLE,
} ExpressionKind;

typedef enum {
    OPERATION_NONE = 0,
    OPERATION_ADD,
    OPERATION_SUB,
    OPERATION_MUL,
    OPERATION_DIV
} Operation;

typedef struct {
    SV name;
} Variable;

typedef struct {
    Expression *lhs;
    Expression *rhs;
    Operation op;
} BinOpExpression;

typedef union {
    BinOpExpression binop;
    SV string;
    int integer;
    FunctionCall funccall;
    Variable var;
} ExpressionAs;

struct Expression {
    ExpressionKind kind;
    ExpressionAs as;
};

typedef struct {
    SV name;
    Type type;
    Expression *expr;
} VariableDeclaration;

typedef struct {
    Expression *expr;
} Return;

typedef union {
    FunctionDeclaration funcdecl;
    FunctionCall funccall;
    VariableDeclaration vardec;
    Return ret;
} NodeAs;

struct Node {
    NodeKind kind;
    NodeAs as;
};

Node *node(NodeKind kind, NodeAs as);
Node *nodeas_funcdecl(FunctionDeclaration funcdecl);
Node *nodeas_funccall(FunctionCall funccall);
Node *nodeas_vardec(VariableDeclaration vardec);
Node *nodeas_return(Expression *ret);


Expression *expr(ExpressionKind kind, ExpressionAs as);
Expression *expras_string(SV string);
Expression *expras_binop(BinOpExpression binop);
Expression *expras_integer(int integer);
Expression *expras_funccall(FunctionCall funccall);
Expression *expras_var(Variable var);

#endif