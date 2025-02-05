#include "typechecker.h"


bool typecheck(Type type, Expression *e) {
    return true;
}

Type typeOf(Expression *e) {
    return (Type) {
        .as = {
            .predef = PRE_DEFINED_TYPE_INT
        },
        .kind = TYPE_KIND_PRE_DEFINED
    };
}
