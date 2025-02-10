#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <stdbool.h>
#include "node.h"
#include "coerror.h"

bool typecheck(Type type, Expression *e);
TypeError typerror(ErrorCode code, SV expectedtype, SV foundtype, SV filename);
const char *strtype(Type t);
Type typeOf(Expression *e);

#endif