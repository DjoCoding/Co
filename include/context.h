#ifndef CONTEXT_H
#define CONTEXT_H

#include "da.h"
#include "sv.h"
#include "node.h"
#include "coerror.h"
#include "colib.h"

typedef struct {
    SV name;
    ARRAY_OF(Parameter) params;
    Type rettype;
} ContextFunction;
DEF_ARRAY(ContextFunction);

typedef struct {
    SV name;
    Type type;
    bool isinitialized;
} ContextVariable;
DEF_ARRAY(ContextVariable);

typedef struct {
    ARRAY_OF(ContextFunction) functions;
    ARRAY_OF(ContextVariable) variables; 
} Context;
DEF_ARRAY(Context);

typedef SV StdLib;

typedef union {
    CoLib lib;
    StdLib std;
} IncludeAs;

typedef struct {
    bool isstd;
    IncludeAs as;
} Include;
DEF_ARRAY(Include);

typedef struct { 
    SV filename;                     // track the name of the file which this context is for
    ARRAY_OF(Context) ctxs;          // track the program stacks of context
    ARRAY_OF(Include) includes;      // track all the includes necessary when generating the code
    ContextFunction currfunc;        // track the current function we're currently generating the code for
    bool inside_func;                // track whether we're inside a function declaration or not
} CodeGeneratorContext;

// generate a context error
ContextError contexterror(ErrorCode code, SV name, CodeGeneratorContext *gcontext);

// constructor for the context object
Context context();

// push a function inside the context object
void context_pushfunc(Context *this, ContextFunction ctxfunc);

// push a variable inside the context object
void context_pushvar(Context *this, ContextVariable ctxvar);

// construct the context function object
ContextFunction contextfunc(FunctionDeclaration func);

// construct the context function object
ContextVariable contextvar(VariableDeclaration var);

// return a function inside a context
ContextFunction *context_findfunc(Context *this, SV funcname);

// return a variable inside a context
ContextVariable *context_findvar(Context *this, SV varname);

// return a param inside a context
Parameter *context_findparam(Context *this, SV paramname);

// constructor for the include object
Include include(SV name, bool isstd);

// constructor for the generator context
CodeGeneratorContext *gcontext();

// set the filename of the generator context
void gcontextfile(CodeGeneratorContext *this, const char *filename);

// push a new context inside the contexts stack
void gcontext_push(CodeGeneratorContext *this);

// set the current function we're generating the code for
void gcontext_set_current_function(CodeGeneratorContext *this, ContextFunction func);

// reset the function we're generating the code for to None
void gcontext_escape_function(CodeGeneratorContext *this);

// find a function in all the generator context
ContextFunction *gcontext_findfunc(CodeGeneratorContext *this, SV funcname);

// find a variable in all the generator context
ContextVariable *gcontext_findvar(CodeGeneratorContext *this, SV varname);

// find the include object in all the generator context
Include *gcontext_findinclude(CodeGeneratorContext *this, SV name);

// find a param in all the generator context
Parameter *gcontext_findparam(CodeGeneratorContext *this, SV paramname);

// push a new function inside the generator context
void gcontext_pushfunc(CodeGeneratorContext *this, ContextFunction func);

// push a variable inside the generator context
void gcontext_pushvar(CodeGeneratorContext *this, ContextVariable var);

// push a new include file inside the generator context
void gcontext_pushinclude(CodeGeneratorContext *this, Include inc);

// pop the current context
void gcontext_pop(CodeGeneratorContext *this);

#endif