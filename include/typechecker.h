#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <stdbool.h>
#include "node.h"
#include "coerror.h"
#include "context.h"

// generate a custom type error
TypeError typerror(ErrorCode code, SV expectedtype, SV foundtype);

// type check expression type
bool typecheckexpr(CodeGeneratorContext *gcontext, Type type, Expression *e);

// get the type of an expression
Type typeOf(CodeGeneratorContext *gcontext, Expression *e);

// check whether the operation on the two types is valid or not
bool typecmpop(Type a, Type b, Operation op);

// get the c string version of a type
const char *strtype(Type t);

// check if a type is CoString (helper)
bool iscostring(Type t);

// exact comparaison between two types
bool typecmp(CodeGeneratorContext *gcontext, Type a, Type b);

#endif