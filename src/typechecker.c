#include "typechecker.h"

#include "shared.h"

bool typecheck(Type type, Expression *e) {
    return true;
}

Type typeOf(Expression *e) {
    TODO("typeof function not implemented yet");
}

TypeError typerror(ErrorCode code, SV expectedtype, SV foundtype, SV filename) {
    return (TypeError) {
        .code = code,
        .expectedtype = expectedtype,
        .foundtype = foundtype,
        .filename = filename
    };
}

const char *strtype(Type t) {
    switch(t.kind) {
        case TYPE_KIND_PRE_DEFINED:
            for(size_t i = 0; i < LENGTH(predeftypes); ++i) {
                if(t.as.predef == predeftypes[i].type) {
                    return predeftypes[i].type_as_cstr;
                }
            };
            UNREACHABLE();
        default:
            UNREACHABLE();
    }
    return NULL;
}