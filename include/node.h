#ifndef NODE_H
#define NODE_H

#include "sv.h"
#include "da.h"

#define TYPE_NONE (Type) { .kind = TYPE_KIND_NONE }
#define PARAM_NONE (Parameter) { .type = TYPE_NONE, .name = SV_NULL }

typedef enum {
    NODE_KIND_NONE = 0,
    NODE_KIND_FUNCTION_DECLARATION,
    NODE_KIND_FUNCTION_CALL,
    NODE_KIND_VARIABLE_DECLARATION,
    NODE_KIND_RETURN_STATEMENT, 
    NODE_KIND_IF,
    NODE_KIND_FOR,
    NODE_KIND_VARIABLE_REASSIGNEMENT,
} NodeKind;

typedef struct Node Node;
typedef struct Expression Expression;

typedef struct {
    Node **items;
    size_t count;
    size_t size;
} Body;

typedef struct Type Type;

typedef enum {
    PRE_DEFINED_TYPE_VOID = 0,
    PRE_DEFINED_TYPE_INT,
    PRE_DEFINED_TYPE_STRING,
    PRE_DEFINED_TYPE_BOOL,
    PRE_DEFINED_TYPE_COUNT
} PreDefinedType;  

typedef struct {
    Type *of;
} ArrayType;

typedef enum {
    TYPE_KIND_NONE = 0,
    TYPE_KIND_PRE_DEFINED, 
    TYPE_KIND_ARRAY,
} TypeKind;

typedef union {
    PreDefinedType predef;
    ArrayType array;
} TypeAs;

struct Type {
    TypeKind kind;
    TypeAs as;
}; 

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
    Type rettype;
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
    OPERATION_DIV,
    OPERATION_LESS,
    OPERATION_LESS_OR_EQ,
    OPERATION_GREATER,
    OPERATION_GREATER_OR_EQ,
    OPERATION_EQ,
    OPERATIONS_COUNT,
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
    SV name;
    Expression *expr;
} VariableReassignement;

typedef struct {
    Expression *expr;
} Return;

typedef struct {
    Expression *e;
    Body body;
} If;

typedef struct {
    VariableDeclaration *v; // for the custom initialization
    Expression *e;          // for the custom end of loop condition
    Body body;
} For;

typedef union {
    FunctionDeclaration funcdecl;
    FunctionCall funccall;
    VariableDeclaration vardec;
    Return ret;
    If iff;
    For forr;
    VariableReassignement varres;
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
Node *nodeas_if(If iff);
Node *nodeas_for(For forr);
Node *nodeas_varres(VariableReassignement varres);

Expression *expr(ExpressionKind kind, ExpressionAs as);
Expression *expras_string(SV string);
Expression *expras_binop(BinOpExpression binop);
Expression *expras_integer(int integer);
Expression *expras_funccall(FunctionCall funccall);
Expression *expras_var(Variable var);


// construct type: array of <of> 
Type arrayof(Type of);
Type typeas_predef(PreDefinedType predef);

VariableDeclaration *vardec(Type type, SV name, Expression *value);


#endif