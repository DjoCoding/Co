#include "code.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "utils.h"
#include "malloc.h"
#include "coerror.h"
#include "colib.h"

#define BUFFER_MAX_SIZE FILENAME_MAX

// this macro will get the this object in the current context as not declared dependency
#define gencode(...) fprintf(this->state.f, __VA_ARGS__)

CodeGenerator *code(const char *input, const char *output) {
    CodeGenerator *c = alloc(sizeof(CodeGenerator));
    c->tree = NULL;
    c->gcontext = gcontext();
    c->state.f = stdout;
    c->state = (CodeGeneratorState) {0};
    c->state.output = output;
    c->state.input = input;
    gcontextfile(c->gcontext, output);
    return c;
}

void code_set_tree(CodeGenerator *this, AST *tree) {
    this->tree = tree;
}

void generate_indent(CodeGenerator *this) {
    for(size_t i = 0; i < this->state.indent; ++i) {
        gencode("\t");
    }
}

void generate_funccall(CodeGenerator *this, FunctionCall funccall, bool inexpr);
void generate_varvalue(CodeGenerator *this, SV varname);
void generate_funcdecl(CodeGenerator *this, FunctionDeclaration funcdecl);
void generate_node(CodeGenerator *this, Node *n, bool infuncbody);
void generate_expression_code(CodeGenerator *this, Expression *e);
void generate_type(CodeGenerator *this, Type type);

void generate_binop_expression_code(CodeGenerator *this, BinOpExpression binop) {
    Type lhstype = typeOf(this->gcontext, binop.lhs);
    Type rhstype = typeOf(this->gcontext, binop.rhs);
    bool result = typecmpop(lhstype, rhstype, binop.op);

    if(!result) {
        throw(
            error(
                TYPE,
                errfromtype(
                    typerror(
                        INVALID_OPERATION_BETWEEN_TYPES,
                        svc((char *)strtype(lhstype)),
                        svc((char *)strtype(rhstype))
                    )
                ), 
                svc((char *)this->state.input)
            )
        );
    }

    if(iscostring(lhstype) && iscostring(rhstype)) {
        // this should raise an error, but im sure this will be handled by the typechecker
        if(binop.op != OPERATION_ADD) { return; }

        // predefined function name
        gencode("costring_concat");
        gencode("(");
        generate_expression_code(this, binop.lhs);
        gencode(",");
        gencode(" ");
        generate_expression_code(this, binop.rhs);
        gencode(")");
        return;
    }

    gencode("(");
    generate_expression_code(this, binop.lhs);
    gencode(" ");
    gencode("%s", mapoptostr(binop.op));
    gencode(" ");
    generate_expression_code(this, binop.rhs);
    gencode(")");
}

void generate_expression_code(CodeGenerator *this, Expression *e) {
    if(e->kind == EXPRESSION_KIND_INTEGER) { 
        gencode("%d", e->as.integer);
        return;
    }

    if(e->kind == EXPRESSION_KIND_STRING) {
        gencode("costring");
        gencode("(");
        gencode("\"" SV_FMT "\"", SV_UNWRAP(e->as.string));
        gencode(")");
        return;
    }

    if(e->kind == EXPRESSION_KIND_BINARY_OPERATION) {
        return generate_binop_expression_code(this, e->as.binop);
    }

    if(e->kind == EXPRESSION_KIND_FUNCTION_CALL) {
        return generate_funccall(this, e->as.funccall, true);
    }

    if(e->kind == EXPRESSION_KIND_VARIABLE) {
        return generate_varvalue(this, e->as.var.name);
    }

    assert(false && "expression kind logging not implemented yet");
}

void generate_funccall_code(CodeGenerator *this, FunctionCall funccall, bool inexpr) {
    if(!inexpr) { 
        generate_indent(this);
    }

    gencode(SV_FMT, SV_UNWRAP(funccall.name));
    gencode("(");
    for(size_t i = 0; i < funccall.args.count; ++i) {
        Argument arg = funccall.args.items[i];
        generate_expression_code(this, arg.e);
        if(i == funccall.args.count - 1)  { break; }
        gencode(",");
        gencode(" ");
    }
    gencode(")");
}

void generate_predef_type_code(CodeGenerator *this, PreDefinedType predef) {
    switch(predef) {
        case PRE_DEFINED_TYPE_INT:
            gencode("int");
            break;
        case PRE_DEFINED_TYPE_VOID:
            gencode("void");
            break;
        case PRE_DEFINED_TYPE_STRING:
            gcontext_pushinclude(this->gcontext, include(svc("costring"), false));
            gencode("CoString");
            break;
        case PRE_DEFINED_TYPE_BOOL:
            gcontext_pushinclude(this->gcontext, include(svc("stdbool.h"), true));
            gencode("bool");
            break;
        default:
            UNREACHABLE();
    }
}

void generate_array_type_code(CodeGenerator *this, ArrayType arraytype) {
    gcontext_pushinclude(this->gcontext, include(svc("coarray"), false));
    gencode("CoArray");
    gencode("(");
    generate_type(this, *arraytype.of);
    gencode(")");
}

void generate_funcdecl_param_code(CodeGenerator *this, Parameter p) {
    generate_type(this, p.type);
    gencode(" ");
    gencode(SV_FMT, SV_UNWRAP(p.name));
}

void generate_funcdecl_params_code(CodeGenerator *this, ARRAY_OF(Parameter) params) {
    if(params.count == 0) { 
        gencode("void");
        return;
    }

    for(size_t i = 0; i < params.count; ++i) {
        Parameter p = params.items[i];
        generate_funcdecl_param_code(this, p);
        if(i == params.count - 1) { break; }
        gencode(",");
        gencode(" ");
    }
}

void generate_body(CodeGenerator *this, Body body) {
    for(size_t i = 0; i < body.count; ++i) {
        Node *n = body.items[i];
        generate_node(this, n, true);
        gencode("\n");
    }
}

void generate_funcdecl_code(CodeGenerator *this, FunctionDeclaration funcdecl) {
    generate_indent(this);

    generate_type(this, funcdecl.rettype);

    gencode(" ");
    
    gencode(SV_FMT, SV_UNWRAP(funcdecl.name));
    
    gencode("(");
    generate_funcdecl_params_code(this, funcdecl.params);
    gencode(")");
    gencode(" ");
    
    gencode("{\n");
    
    this->state.indent += 1;
    generate_body(this, funcdecl.body);
    this->state.indent -= 1;
    
    gencode("}\n");
}

void generate_vardec_code_inline(CodeGenerator *this, VariableDeclaration vardec) {
    generate_type(this, vardec.type);
    
    gencode(" ");

    gencode(SV_FMT, SV_UNWRAP(vardec.name));

    if(vardec.expr) {
        gencode(" ");
        gencode("=");
        gencode(" ");
        generate_expression_code(this, vardec.expr);
    }
}

void generate_vardec_code(CodeGenerator *this, VariableDeclaration vardec) {
    if(vardec.expr) {
        Type exprtype = typeOf(this->gcontext, vardec.expr);
        if(!typecmp(this->gcontext, vardec.type, exprtype)) {
            throw(
                error(
                    TYPE,
                    errfromtype(
                        typerror(
                            INVALID_EXPRESSION_TYPE_ON_ASSIGNEMENT, 
                            svc((char *)strtype(vardec.type)),
                            svc((char *)strtype(exprtype))
                        )
                    ),
                    svc((char *)this->state.input)
                )
            );

            return;
        }
    }
    
    generate_indent(this);
    generate_vardec_code_inline(this, vardec);
    gencode(";");
}


void generate_varvalue(CodeGenerator *this, SV varname) {
    ContextVariable *var = gcontext_findvar(this->gcontext, varname);
    if(!var) {
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        VARIABLE_NOT_DECLARED,
                        varname,
                        this->gcontext
                    )
                ),
                svc((char *)this->state.input)
            )
        );
        return;
    }

    gencode(SV_FMT, SV_UNWRAP(var->name));
} 

void generate_funccall(CodeGenerator *this, FunctionCall funccall, bool inexpr) {
    ContextFunction *func = gcontext_findfunc(this->gcontext, funccall.name);
    if(!func) {
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        FUNCTION_NOT_DECLARED,
                        funccall.name,
                        this->gcontext
                    )
                ), 
                svc((char *)this->state.input)
            )
        );
        return;
    }

    if(func->params.count != funccall.args.count) { 
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        INVALID_NUMBER_OF_PARAMS,
                        funccall.name,
                        this->gcontext
                    )
                ), 
                svc((char *)this->state.input)
            )
        );
        return;
    }

    for(size_t i = 0; i < func->params.count; ++i) {
        Parameter p = func->params.items[i];
        Argument arg = funccall.args.items[i];
        if(!typecheckexpr(this->gcontext, p.type, arg.e)) { 
            throw(
                error(
                    TYPE,
                    errfromtype(
                        typerror(
                            TYPE_ERROR,
                            svc((char *)strtype(p.type)),
                            svc((char *)strtype(typeOf(this->gcontext, arg.e)))
                        )
                    ),
                    svc((char *)this->state.input)
                )
            );
            return;
        }
    }

    generate_funccall_code(this, funccall, inexpr);
}


void generate_funcdecl(CodeGenerator *this, FunctionDeclaration funcdecl) {
    ContextFunction *found = gcontext_findfunc(this->gcontext, funcdecl.name);
    if(found) {
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        FUNCTION_ALREADY_DECLARED,
                        funcdecl.name,
                        this->gcontext
                    )
                ),
                svc((char *)this->state.input)
            )
        );
        return;
    }

    ContextFunction func = contextfunc(funcdecl);

    // push the function in the current context 
    gcontext_pushfunc(this->gcontext, func);

    gcontext_set_current_function(this->gcontext, func);

    // push a new context for the function
    gcontext_push(this->gcontext);

    for(size_t i = 0; i < funcdecl.params.count; ++i) {
        Parameter p = funcdecl.params.items[i];
        gcontext_pushvar(this->gcontext, (ContextVariable) {
            .name = p.name, 
            .type = p.type
        });
    }

    // generate the function declaration source code
    generate_funcdecl_code(this, funcdecl);

    // pop the function context
    gcontext_pop(this->gcontext);

    // escape the current function
    gcontext_escape_function(this->gcontext);
}

void crash_on_varfound(CodeGenerator *this, SV name) {
    ContextVariable *var = gcontext_findvar(this->gcontext, name);
    if(var) {
        throw(
            error(
                CONTEXT,
                errfromcontext(
                    contexterror(
                        VARIABLE_ALREADY_DECLARED,
                        name,
                        this->gcontext
                    )
                ),
                svc((char *)this->state.input)
            )
        );
        return;
    }
}

void generate_vardec(CodeGenerator *this, VariableDeclaration vardec) {
    crash_on_varfound(this, vardec.name);
    gcontext_pushvar(this->gcontext, contextvar(vardec));
    generate_vardec_code(this, vardec);
}

void generate_vardec_inline(CodeGenerator *this, VariableDeclaration vardec) {
    crash_on_varfound(this, vardec.name);
    gcontext_pushvar(this->gcontext, contextvar(vardec));
    generate_vardec_code_inline(this, vardec);
}

void generate_type(CodeGenerator *this, Type type) {
    if(type.kind == TYPE_KIND_PRE_DEFINED) {
        return generate_predef_type_code(this, type.as.predef);
    }

    if(type.kind == TYPE_KIND_ARRAY) {
        return generate_array_type_code(this, type.as.array);
    }

    TODO("implement custom types later on");
}

void generate_return(CodeGenerator *this, Return ret) {
    if(!this->gcontext->inside_func) {
        assert(false && "not inside function??? check the implementation of the function declaration, make sure to set this flag to true");
    }

    Type type = typeOf(this->gcontext, ret.expr);
    Type rettype = this->gcontext->currfunc.rettype;

    if(!typecmp(this->gcontext, rettype, type)) {
        throw(
            error(
                TYPE,
                errfromtype(
                    typerror(
                        INVALID_RETURN_TYPE_OF_FUNCTION,
                        svc((char *)strtype(rettype)),
                        svc((char *)strtype(type))
                    )
                ),
                svc((char *)this->state.input)
            )
        );

        return;
    }

    generate_indent(this);
    gencode("return");
    gencode(" ");
    generate_expression_code(this, ret.expr);
    gencode(";");
}

void generate_if(CodeGenerator *this, If iff) {
    generate_indent(this);
    
    gencode("if");
    
    gencode("(");
    generate_expression_code(this, iff.e);
    gencode(")");

    gencode(" ");

    gencode("{\n");

    // push a new context
    gcontext_push(this->gcontext);

    this->state.indent += 1;
    generate_body(this, iff.body);
    this->state.indent -= 1;

    // pop the context
    gcontext_pop(this->gcontext);

    generate_indent(this);
    gencode("}");
}

void generate_for(CodeGenerator *this, For forr) {
    generate_indent(this);
    
    gencode("for");

    // push a new context
    gcontext_push(this->gcontext);
    
    gencode("(");
    
    if(forr.v) {
        generate_vardec_inline(this, *forr.v);
    }

    gencode(";");
    gencode(" ");

    if(forr.e) {
        generate_expression_code(this, forr.e);
    }

    gencode(";");
    gencode(" ");

    gencode(")");

    gencode(" ");

    gencode("{\n");

    this->state.indent += 1;
    generate_body(this, forr.body);
    this->state.indent -= 1;

    // pop the context
    gcontext_pop(this->gcontext);

    generate_indent(this);
    gencode("}");
}

void generate_varres(CodeGenerator *this, VariableReassignement varres) {
    generate_indent(this);
    gencode(SV_FMT, SV_UNWRAP(varres.name));
    gencode(" ");
    gencode("=");
    gencode(" ");
    generate_expression_code(this, varres.expr);
    gencode(";");
}

void generate_node(CodeGenerator *this, Node *n, bool infuncbody) {
    if(n->kind == NODE_KIND_FUNCTION_CALL) {
        return generate_funccall(this, n->as.funccall, false);
    }

    if(n->kind == NODE_KIND_FUNCTION_DECLARATION) {
        return generate_funcdecl(this, n->as.funcdecl);
    }

    if(n->kind == NODE_KIND_VARIABLE_DECLARATION) {
        return generate_vardec(this, n->as.vardec);
    }

    if(n->kind == NODE_KIND_RETURN_STATEMENT) {
        return generate_return(this, n->as.ret);
    }

    if(n->kind == NODE_KIND_IF) {
        return generate_if(this, n->as.iff);
    }

    if(n->kind == NODE_KIND_FOR) {
        return generate_for(this, n->as.forr);
    }

    if(n->kind == NODE_KIND_VARIABLE_REASSIGNEMENT) {
        return generate_varres(this, n->as.varres);
    }

    UNREACHABLE();
}

void generate_stdlib_include(CodeGenerator *this, FILE *root, StdLib std) {
    char buffer[BUFFER_MAX_SIZE] = {0};
    snprintf(buffer, sizeof(buffer), "#include \"" SV_FMT "\"\n", SV_UNWRAP((SV) std));
    fwrite(buffer, sizeof(char), strlen(buffer), root);
    return;
}

void generate_colib_include(CodeGenerator *this, FILE *root, CoLib lib) {
    const char *filename = headerof(lib);
    const char *tag = tagof(lib);

    char orgpath[BUFFER_MAX_SIZE] = {0};
    char outpath[BUFFER_MAX_SIZE] = {0};

    snprintf(orgpath, sizeof(orgpath), "./libs/%s.h", filename);
    snprintf(outpath, sizeof(outpath), "./dist/include/%s.h", filename);

    bool result = fcopy((const char *)orgpath, (const char *)outpath);
    if(!result) {
        perror("file copying failed");
        exit(EXIT_FAILURE);
    }

    // update the root file
    char buffer[BUFFER_MAX_SIZE] = {0};
    snprintf(buffer, sizeof(buffer), "#define %s\n", tag);
    fwrite(buffer, sizeof(char), strlen(buffer), root);

            
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "#include \"./include/%s.h\"\n", filename); 
    fwrite(buffer, sizeof(char), strlen(buffer), root);

    fwrite("\n", sizeof(char), strlen("\n"), root);
}

void generate_include(CodeGenerator *this, FILE *root, Include inc) {
    if(inc.isstd) {
        return generate_stdlib_include(this, root, inc.as.std);
    }

    return generate_colib_include(this, root, inc.as.lib);
}


void generate_includes(CodeGenerator *this) {
    // open the includes file
    FILE *root = fopen("./dist/root.h", "w");
    if(!root) { 
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    // no includes found (create the file only)
    if(this->gcontext->includes.count == 0) { 
        fclose(root);
        return;
    }

    // create the includes directory
    if(!createdir("./dist/include")) {
        perror("creating dir failed");
        exit(EXIT_FAILURE);
    }


    // for every file included, copy the file from the pre-build includes lib to the desired lib
    for(size_t i = 0; i < this->gcontext->includes.count; ++i) {
        Include inc = this->gcontext->includes.items[i];
        generate_include(this, root, inc);
    }


    fclose(root);
}

void generate(CodeGenerator *this) {
    // create the dist dir
    if(!createdir("./dist")) {
        perror("creating dir failed");
        exit(EXIT_FAILURE);
    }

    // open the main file to write to
    FILE *f = fopen(this->state.output, "w");
    if(!f) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    // set it in the generator global context
    this->state.f = f;
    
    // push the global transpiler context
    gcontext_push(this->gcontext);

    // include the includes root file
    gencode("#include \"./root.h\"\n\n");

    // generate all the nodes inside the main file
    for(size_t i = 0; i < this->tree->count; ++i) {
        Node *n = this->tree->items[i];
        generate_node(this, n, false);
        gencode("\n");
    }
    
    // generate the includes
    generate_includes(this);

    // close the main file
    if(this->state.f != stdout) {
        fclose(this->state.f);
    }
}


