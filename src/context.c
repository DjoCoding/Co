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

ContextError contexterror(ErrorCode code, SV name, CodeGeneratorContext *gcontext) {
    return (ContextError) {
        .code = code,
        .name = name,
    };
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
        .params = func.params,
        .rettype = func.rettype,
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

CodeGeneratorContext *gcontext() {
    CodeGeneratorContext *gctx = alloc(sizeof(CodeGeneratorContext));
    gctx->ctxs = ARRAY(Context);
    gctx->filename = SV_NULL;
    gctx->includes = ARRAY(Include);
    return gctx;
}

void gcontextfile(CodeGeneratorContext *this, const char *filename) {
    this->filename = svc((char *)filename);
}

void gcontext_set_current_function(CodeGeneratorContext *this, ContextFunction func) {
    this->inside_func = true;
    this->currfunc = func;
}

// i liked the name of this function
void gcontext_escape_function(CodeGeneratorContext *this) {
    this->inside_func = false;
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

    ContextFunction *f = context_findfunc(gcontext_tos(this), func.name);
    if(f) {
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        FUNCTION_ALREADY_DECLARED,
                        func.name,
                        this
                    )
                ), 
                this->filename
            )
        );

        return;
    }

    return context_pushfunc(gcontext_tos(this), func);
}

void gcontext_pushvar(CodeGeneratorContext *this, ContextVariable var) {
    if(gcontext_empty(this)) { 
        gcontext_push(this);
    }

    ContextVariable *v = context_findvar(gcontext_tos(this), var.name);
    if(v) {
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        VARIABLE_ALREADY_DECLARED,
                        var.name,
                        this
                    )
                ), 
                this->filename
            )
        );

        return;
    }

    context_pushvar(gcontext_tos(this), var);
}

void gcontext_pop(CodeGeneratorContext *this) {
    assert(!gcontext_empty(this) && "cannot pop from an empty context stack");
    free(gcontext_tos(this)->functions.items);
    free(gcontext_tos(this)->variables.items);
    this->ctxs.count -= 1;
}

Include include(SV name, bool isstd) {
    return (Include) {
        .name = name,
        .isstd = isstd
    };
} 

Include *gcontext_findinclude(CodeGeneratorContext *this, SV name) {
    for(size_t i = 0; i < this->includes.count; ++i) {
        Include *inc = &this->includes.items[i];
        if(svcmp(name, inc->name)) { return inc; }
    }
    return NULL;
}

void gcontext_pushinclude(CodeGeneratorContext *this, Include inc) {
    Include *isfound = gcontext_findinclude(this, inc.name);
    if(isfound) { return; }
    APPEND(&this->includes, inc);
}

