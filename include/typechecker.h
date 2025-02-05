#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <stdbool.h>
#include "node.h"

bool typecheck(Type type, Expression *e);
Type typeOf(Expression *e);

#endif