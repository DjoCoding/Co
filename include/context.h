#ifndef CONTEXT_H
#define CONTEXT_H

#include "da.h"
#include "sv.h"
#include "node.h"
#include "coerror.h"

typedef struct {
    SV name;
    ARRAY_OF(Parameter) params;
} ContextFunction;
DEF_ARRAY(ContextFunction);

typedef struct {
    SV name;
    Type type;
} ContextVariable;
DEF_ARRAY(ContextVariable);

typedef struct {
    ARRAY_OF(ContextFunction) functions;
    ARRAY_OF(ContextVariable) variables; 
} Context;
DEF_ARRAY(Context);

typedef struct {
    SV name;
    bool isstd;
} Include;
DEF_ARRAY(Include);

typedef struct { 
    SV filename;
    ARRAY_OF(Context) ctxs;
    ARRAY_OF(Include) includes;
} CodeGeneratorContext;

Context context();
void context_pushfunc(Context *this, ContextFunction ctxfunc);
void context_pushvar(Context *this, ContextVariable ctxvar);

ContextFunction contextfunc(FunctionDeclaration func);
ContextVariable contextvar(VariableDeclaration var);

ContextFunction *context_findfunc(Context *this, SV funcname);
ContextVariable *context_findvar(Context *this, SV varname);
Parameter *context_findparam(Context *this, SV paramname);

Include include(SV name, bool isstd);

CodeGeneratorContext *gcontext();
ContextError contexterror(ErrorCode code, SV name, CodeGeneratorContext *gcontext);
void gcontextfile(CodeGeneratorContext *this, const char *filename);
void gcontext_push(CodeGeneratorContext *this);
ContextFunction *gcontext_findfunc(CodeGeneratorContext *this, SV funcname);
ContextVariable *gcontext_findvar(CodeGeneratorContext *this, SV varname);
Include *gcontext_findinclude(CodeGeneratorContext *this, SV name);
Parameter *gcontext_findparam(CodeGeneratorContext *this, SV paramname);
void gcontext_push(CodeGeneratorContext *this);
void gcontext_pushfunc(CodeGeneratorContext *this, ContextFunction func);
void gcontext_pushvar(CodeGeneratorContext *this, ContextVariable var);
void gcontext_pushinclude(CodeGeneratorContext *this, Include inc);
void gcontext_pop(CodeGeneratorContext *this);

#endif