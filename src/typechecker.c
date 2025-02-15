#include "typechecker.h"

#include "shared.h"

Type predeftype(PreDefinedType predef) {
    return (Type) {
        .kind = TYPE_KIND_PRE_DEFINED,
        .as.predef = predef
    };
}

bool typecheckexpr(CodeGeneratorContext *gcontext, Type type, Expression *e) {
    return true;
}

bool typecmpop_on_predef(PreDefinedType a, PreDefinedType b, Operation op) {
    switch(a) {
        case PRE_DEFINED_TYPE_INT:
            if(b == PRE_DEFINED_TYPE_INT) return true;
            return false;
        case PRE_DEFINED_TYPE_BOOL:
            return false;
        case PRE_DEFINED_TYPE_STRING:
            if(b == PRE_DEFINED_TYPE_STRING && op == OPERATION_ADD) return true;   
            return false;
        default:
            return false;
    }

    return false;
}

bool iscostring(Type t) {
    return (t.kind == TYPE_KIND_PRE_DEFINED && t.as.predef == PRE_DEFINED_TYPE_STRING); 
}

bool typecmp(CodeGeneratorContext *gcontext, Type a, Type b) {
    if(a.kind != b.kind) { return false; }

    if(a.kind == TYPE_KIND_PRE_DEFINED) {
        return a.as.predef == b.as.predef;
    }

    if(a.kind == TYPE_KIND_ARRAY) {
        return typecmp(gcontext, *a.as.array.of, *b.as.array.of);
    }

    UNREACHABLE();
}

// check if the operation is valid on the types
bool typecmpop(Type a, Type b, Operation op) {
    if(a.kind != b.kind) { return false; }
    switch(a.kind) {
        case TYPE_KIND_PRE_DEFINED:
            return typecmpop_on_predef(a.as.predef, b.as.predef, op);
        default:
            return false;
    }

    return false;
}

Type typeOf_function_call(CodeGeneratorContext *gcontext, FunctionCall funccall) {
    ContextFunction *func = gcontext_findfunc(gcontext, funccall.name);
    if(!func) {
       UNREACHABLE(); 
    }
    return func->rettype;
}

Type typeOf_variable(CodeGeneratorContext *gcontext, Variable var) {
    ContextVariable *v = gcontext_findvar(gcontext, var.name);
    if(!v) {
       UNREACHABLE(); 
    }
    return v->type;
}

Type typeOf(CodeGeneratorContext *gcontext, Expression *e) {
    switch(e->kind) {
        case EXPRESSION_KIND_FUNCTION_CALL: 
            return typeOf_function_call(gcontext, e->as.funccall);
        case EXPRESSION_KIND_INTEGER:
            return predeftype(PRE_DEFINED_TYPE_INT);
        case EXPRESSION_KIND_STRING:
            return predeftype(PRE_DEFINED_TYPE_STRING);
        case EXPRESSION_KIND_VARIABLE:
            return typeOf_variable(gcontext, e->as.var);
        case EXPRESSION_KIND_BINARY_OPERATION:
            Type lhs = typeOf(gcontext, e->as.binop.lhs);
            Type rhs = typeOf(gcontext, e->as.binop.rhs);
            if(typecmpop(lhs, rhs, e->as.binop.op)) { return lhs; }
            
            // expected type will be interpreted as the lhs type, the other one for the rhs type
            throw(
                error(
                    TYPE,
                    errfromtype(
                        typerror(
                            INVALID_OPERATION_BETWEEN_TYPES,
                            svc((char *)strtype(lhs)),
                            svc((char *)strtype(rhs))
                        )
                    ),
                    gcontext->filename
                )
            );
            break;
        default:    
            UNREACHABLE();
    }

    return TYPE_NONE;
}

TypeError typerror(ErrorCode code, SV expectedtype, SV foundtype) {
    return (TypeError) {
        .code = code,
        .expectedtype = expectedtype,
        .foundtype = foundtype,
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