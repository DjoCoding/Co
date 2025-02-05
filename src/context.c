#include "context.h"

#include <assert.h>
#include <stdlib.h>
#include "malloc.h"

Context context() {
    Context ctx = {0};
    ctx.functions = ARRAY(ContextFunction);
    ctx.variables = ARRAY(ContextVariable);
    return ctx;
}

void context_pushfunc(Context *this, ContextFunction ctxfunc) {
    APPEND(&this->functions, ctxfunc);
}

void context_pushvar(Context *this, ContextVariable ctxvar) {
    APPEND(&this->variables, ctxvar);
}

ContextFunction contextfunc(FunctionDeclaration func) {
    return (ContextFunction) {
        .name = func.name, 
        .params = func.params
    };
}

ContextVariable contextvar(VariableDeclaration var) {
    return (ContextVariable) {
        .name = var.name, 
        .type = var.type,
    };
}

ContextFunction *context_findfunc(Context *this, SV funcname) {
    for(size_t i = 0; i < this->functions.count; ++i) {
        ContextFunction func = this->functions.items[i];
        if(svcmp(func.name, funcname)) { 
            return &this->functions.items[i];
        }
    }
    return NULL;
}

ContextVariable *context_findvar(Context *this, SV varname) {
    for(size_t i = 0; i < this->variables.count; ++i) {
        ContextVariable var = this->variables.items[i];
        if(svcmp(var.name, varname)) { 
            return &this->variables.items[i];
        }
    }
    return NULL;
}

Parameter *context_findparam(Context *this, SV paramname) {
    for(size_t i = 0; i < this->functions.count; ++i) {
        ContextFunction func = this->functions.items[i];
        ARRAY_OF(Parameter) params = func.params;
        for(size_t j = 0; j < params.count; ++j) {
            Parameter param = params.items[j];
            if(svcmp(param.name, paramname)) { 
                return &params.items[j];
            }
        }
    }
    return NULL;
}

// ContextFunction *getfuncfromcall(Context *this, FunctionCall funccall) {
//     ContextFunction *func = context_findfunc(this, funccall.name);
//     if(!func) {
//         //FIXME: error 
//         abort();
//     }

//     if(func->params.count != funccall.args.count) {
//         //FIXME: error
//         abort();
//     }

//     for(size_t i = 0; i < func->params.count; ++i) {
//         Parameter p = func->params.items[i];
//         context_pushvar(this, (ContextVariable) {
//             .name = p.name, 
//             .type = p.type,
//         })        
//     }
// }

CodeGeneratorContext *gcontext() {
    CodeGeneratorContext *gctx = alloc(sizeof(CodeGeneratorContext));
    gctx->ctxs = ARRAY(Context);
    return gctx;
}

ContextFunction *gcontext_findfunc(CodeGeneratorContext *this, SV funcname) {
    for(size_t i = this->ctxs.count; i > 0; --i) {
        Context *ctx = &this->ctxs.items[i - 1];
        ContextFunction *func = context_findfunc(ctx, funcname);
        if(!func) { continue; }
        return func;
    }
    return NULL;
}


ContextVariable *gcontext_findvar(CodeGeneratorContext *this, SV varname) {
    for(size_t i = this->ctxs.count; i > 0; --i) {
        Context *ctx = &this->ctxs.items[i - 1];
        ContextVariable *var = context_findvar(ctx, varname);
        if(!var) { continue; }
        return var;
    }
    return NULL;
}

Parameter *gcontext_findparam(CodeGeneratorContext *this, SV paramname) {
    for(size_t i = this->ctxs.count; i > 0; --i) {
        Context *ctx = &this->ctxs.items[i - 1];
        Parameter *p = context_findparam(ctx, paramname);
        if(!p) { continue; }
        return p;
    }
    return NULL;
}


void gcontext_push(CodeGeneratorContext *this) {
    APPEND(&this->ctxs, context());
}

// get the top of the stack of the contexts
Context *gcontext_tos(CodeGeneratorContext *this) {
    return &this->ctxs.items[this->ctxs.count - 1];
}

bool gcontext_empty(CodeGeneratorContext *this) {
    return this->ctxs.count == 0;
}


void gcontext_pushfunc(CodeGeneratorContext *this, ContextFunction func) {
    if(gcontext_empty(this)) { 
        gcontext_push(this);
    }

    context_pushfunc(gcontext_tos(this), func);
}

void gcontext_pushvar(CodeGeneratorContext *this, ContextVariable var) {
    if(gcontext_empty(this)) { 
        gcontext_push(this);
    }

    ContextVariable *v = context_findvar(gcontext_tos(this), var.name);
    if(v) {
        //FIXME: raise error
        // variable already declared
        abort();
    }

    context_pushvar(gcontext_tos(this), var);
}

void gcontext_pop(CodeGeneratorContext *this) {
    assert(!gcontext_empty(this) && "cannot pop from an empty context stack");
    free(gcontext_tos(this)->functions.items);
    free(gcontext_tos(this)->variables.items);
    this->ctxs.count -= 1;
}




