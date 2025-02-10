#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <stdbool.h>
#include "node.h"
#include "coerror.h"
#include "context.h"

TypeError typerror(ErrorCode code, SV expectedtype, SV foundtype);
bool typecheckexpr(CodeGeneratorContext *gcontext, Type type, Expression *e);
Type typeOf(CodeGeneratorContext *gcontext, Expression *e);
bool typecmpop(Type a, Type b, Operation op);
const char *strtype(Type t);

#endif