#include "parser.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "malloc.h"
#include "shared.h"

// this for the parser instance
#define expect(kind) throw(error(PARSER, errfromparser(parserror(kind, EXPECTED_TOKEN_KIND_BUT_FOUND_ANOTHER, this))))

Parser *parser(const char *filename) {
    Parser *this = alloc(sizeof(Parser));
    this->tokens = ARRAY(Token);
    this->current = 0;
    this->filename = svc((char *)filename);
    return this;
}

void parser_set_tokens(Parser *this, ARRAY_OF(Token) tokens) {
    this->tokens = tokens;
}

ParserError parserror(TokenKind kind, ErrorCode code, Parser *this) {
    ParserError err = {0};
    err.code = code;
    err.filename = this->filename;
    err.expectedkind = kind;
    err.currtoken = this->tokens.items[this->current];
    return err;
}

Token trypeakahead(Parser *this, size_t ahead) {
    if(this->current + ahead >= this->tokens.count) { return TOKEN_NONE; }
    return this->tokens.items[this->current + ahead];
}

Token ppeek(Parser *this) {
    return trypeakahead(this, 0);
}

void padvance(Parser *this) {
    ++this->current;
}

bool pend(Parser *this) {
    Token t = ppeek(this);
    return t.kind == TOKEN_KIND_END;
}

bool pexpect(Parser *this, TokenKind kind) {
    Token t = ppeek(this);
    return t.kind == kind;
}

Node *parse_node(Parser *this, bool infuncbody);
Expression *parse_addition(Parser *this);
FunctionCall parse_function_call(Parser *this);
Expression *parse_expression(Parser *this);

Type parse_type(Parser *this) {
    Token t = ppeek(this);

    for(size_t i = 0; i < LENGTH(predeftypes); ++i) {
        PreDefinedTypeMap current = predeftypes[i];
        if(svcmp(svc((char* )current.type_as_cstr), t.value)) { 
            padvance(this);
            return typeas_predef(current.type);
        }
    }

    throw(
        error(
            PARSER,
            errfromparser(
                parserror(
                    TOKEN_KIND_NONE,
                    UNKNOWN_TYPE_NAME,
                    this
                )
            )
        )
    );

    return TYPE_NONE;
}

int parse_integer(SV value) {
    int result = 0;
    for(size_t i = 0; i < value.count; ++i) {
        result *= 10;
        result += (int) value.content[i] - (int)'0';
    }
    return result;
}

Expression *parse_primary(Parser *this) {
    Token t = ppeek(this);

    if(t.kind == TOKEN_KIND_INTEGER) {
        padvance(this);
        return expras_integer(
            parse_integer(t.value)
        );
    }

    if(t.kind == TOKEN_KIND_STRING) {
        padvance(this);
        return expras_string(t.value);
    }

    if(t.kind == TOKEN_KIND_OPEN_PAREN) {
        padvance(this);
        Expression *e = parse_addition(this);
        
        if(pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { 
            padvance(this);
            return e;
        }

        expect(TOKEN_KIND_CLOSE_PAREN);
        return NULL;
    }

    if(t.kind == TOKEN_KIND_IDENTIFIER) {
        if(trypeakahead(this, 1).kind == TOKEN_KIND_OPEN_PAREN) {
            FunctionCall funccall = parse_function_call(this);
            return expras_funccall(funccall);
        }

        padvance(this);
        return expras_var((Variable) {
            .name = t.value
        });
    }


    throw(
        error(
            PARSER,
            errfromparser(
                parserror(
                    TOKEN_KIND_NONE,
                    EXPECTED_EXPRESSION,
                    this
                )
            )
        )
    );
    
    return NULL;
}

Expression *parse_multiplication(Parser *this) {
    Expression *lhs = parse_primary(this);

    while(!pend(this)) {
        if(!ismultiplication(ppeek(this).kind)) {
            break;
        }

        Operation op = ppeek(this).kind == TOKEN_KIND_STAR ? OPERATION_MUL : OPERATION_DIV;
        padvance(this);

        Expression *rhs = parse_primary(this);
        BinOpExpression binop = {
            .lhs = lhs,
            .rhs = rhs,
            .op = op
        };
        lhs = expras_binop(binop);
    }

    return lhs;
}

Expression *parse_addition(Parser *this) {
    Expression *lhs = parse_multiplication(this);

    while(!pend(this)) {
        if(!isaddition(ppeek(this).kind)) {
            break;
        }

        Operation op = ppeek(this).kind == TOKEN_KIND_PLUS ? OPERATION_ADD : OPERATION_SUB;
        padvance(this);

        Expression *rhs = parse_multiplication(this);
        BinOpExpression binop = {
            .lhs = lhs,
            .rhs = rhs,
            .op = op
        };
        lhs = expras_binop(binop);
    }

    return lhs;
}

Expression *parse_comparaison(Parser *this) {
    Expression *lhs = parse_addition(this);

    while(!pend(this)) {
        if(!iscomparaison(ppeek(this).kind)) {
            break;
        }

        Operation op = 0;
        switch(ppeek(this).kind) {
            case TOKEN_KIND_LESS:
                op = OPERATION_LESS;
                break;
            case TOKEN_KIND_LESS_OR_EQ:
                op = OPERATION_LESS_OR_EQ;
                break;
            case TOKEN_KIND_GREATER:
                op = OPERATION_GREATER;
                break;
            case TOKEN_KIND_GREATER_OR_EQ:
                op = OPERATION_GREATER_OR_EQ;
                break;
            case TOKEN_KIND_EQ:
                op = OPERATION_EQ;
                break;
            default: 
                UNREACHABLE();
        }

        padvance(this);

        Expression *rhs = parse_addition(this);
        BinOpExpression binop = {
            .lhs = lhs,
            .rhs = rhs,
            .op = op
        };
        lhs = expras_binop(binop);
    }

    return lhs;
}


Expression *parse_expression(Parser *this) {
    return parse_comparaison(this);
}

Parameter parse_parameter(Parser *this) {
    Parameter p = {0};

    Type type = parse_type(this);
    p.type = type;

    if(!pexpect(this, TOKEN_KIND_IDENTIFIER)) { 
        expect(TOKEN_KIND_IDENTIFIER);
        return PARAM_NONE;
    }

    p.name = ppeek(this).value;
    padvance(this);

    return p;
}

ARRAY_OF(Parameter) parse_parameters(Parser *this) {
    ARRAY_OF(Parameter) params = ARRAY(Parameter);

    if(!pexpect(this, TOKEN_KIND_OPEN_PAREN)) { return params; }

    padvance(this);

    if(pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { 
        padvance(this);
        return params;
    }

    while(!pend(this)) {
        Parameter param = parse_parameter(this);
        APPEND(&params, param);

        if(pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { break; }             
        if(pexpect(this, TOKEN_KIND_COMMA)) {
            padvance(this);
            continue;
        }

        expect(TOKEN_KIND_COMMA);
        return ARRAY(Parameter);
    }

    if(!pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { 
        expect(TOKEN_KIND_CLOSE_PAREN);
        return ARRAY(Parameter);
    }

    // consuming the ')' token
    padvance(this);

    return params;
}

Argument parse_argument(Parser *this) {
    Expression *e = parse_expression(this);
    return (Argument) {
        .e = e
    };
}

ARRAY_OF(Argument) parse_arguments(Parser *this) {
    ARRAY_OF(Argument) args = ARRAY(Argument);

    if(!pexpect(this, TOKEN_KIND_OPEN_PAREN)) { 
        expect(TOKEN_KIND_OPEN_PAREN);
        return ARRAY(Argument);
    }

    padvance(this);

    if(pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { 
        padvance(this);
        return args;
    }

    while(!pend(this)) {
        Argument arg = parse_argument(this);
        APPEND(&args, arg);

        if(pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { break; }             
        if(pexpect(this, TOKEN_KIND_COMMA)) {
            padvance(this);
            continue;
        }

    }

    if(!pexpect(this, TOKEN_KIND_CLOSE_PAREN)) { 
        expect(TOKEN_KIND_CLOSE_PAREN);
        return ARRAY(Argument);
    }

    // consuming the ')' token
    padvance(this);

    return args;
}

Body parse_body(Parser *this) {
    Body body = {0};

    while(!pend(this)) {
        if(pexpect(this, TOKEN_KIND_CLOSE_CURLY)) { break; }
        Node *n = parse_node(this, true);
        APPEND(&body, n);
    }

    return body;
}

FunctionDeclaration parse_function_declaration(Parser *this) {
    if(!pexpect(this, TOKEN_KIND_FN)) {
        expect(TOKEN_KIND_FN);
        return (FunctionDeclaration) {0};
    }

    // consuming the fn token
    padvance(this);

    FunctionDeclaration funcdecl = {0};

    if(!pexpect(this, TOKEN_KIND_IDENTIFIER)) {
        expect(TOKEN_KIND_IDENTIFIER);
        return (FunctionDeclaration) {0};   
    }

    funcdecl.name = ppeek(this).value;
    padvance(this);

    ARRAY_OF(Parameter) params = parse_parameters(this);
    funcdecl.params = params;

    if(!pexpect(this, TOKEN_KIND_COLON)) {
        funcdecl.rettype = typeas_predef(PRE_DEFINED_TYPE_VOID);
    } else {
        padvance(this);
        Type rettype = parse_type(this);
        funcdecl.rettype = rettype;
    }

    if(!pexpect(this, TOKEN_KIND_OPEN_CURLY)) {
        // for now just make it return error, but later fix this to make it return an expression directly
        // example
        // fn add(int a, int b) => a + b;
        //FIXME: error
        abort();
        TODO("inline functions not implemented yet\n example: fn add(int a, int b) => a + b");
    }

    padvance(this);
    
    Body body = parse_body(this);

    if(!pexpect(this, TOKEN_KIND_CLOSE_CURLY)) {
        expect(TOKEN_KIND_CLOSE_CURLY);
        return (FunctionDeclaration) {0};   
    }

    funcdecl.body = body;

    padvance(this);

    return funcdecl;
}

FunctionCall parse_function_call(Parser *this) {
    if(!pexpect(this, TOKEN_KIND_IDENTIFIER)) {
        expect(TOKEN_KIND_IDENTIFIER);
        return (FunctionCall) {0};   
    }

    FunctionCall funccall = {0};
    funccall.name = ppeek(this).value;

    padvance(this);

    ARRAY_OF(Argument) args = parse_arguments(this);
    funccall.args = args;

    return funccall;
}

VariableDeclaration parse_variable_declaration(Parser *this) {
    VariableDeclaration vardec = {0};

    Type type = parse_type(this);
    vardec.type = type;

    if(!pexpect(this, TOKEN_KIND_IDENTIFIER)) {
        expect(TOKEN_KIND_IDENTIFIER);
        return (VariableDeclaration) {0};   
    }

    vardec.name = ppeek(this).value;
    padvance(this);

    if(!pexpect(this, TOKEN_KIND_EQUAL)) { 
        vardec.expr = NULL;
        return vardec;
    }

    padvance(this);
    
    Expression *expr = parse_expression(this);
    vardec.expr = expr;

    return vardec;
}

If parse_if(Parser *this) {
    if(!pexpect(this, TOKEN_KIND_IF)) {
        expect(TOKEN_KIND_IF);
        return (If) {0};   
    }

    padvance(this);

    If iff = {0};

    if(!pexpect(this, TOKEN_KIND_OPEN_PAREN)) {
        expect(TOKEN_KIND_OPEN_PAREN);
        return (If) {0};   
    }

    padvance(this);
    
    Expression *e = parse_expression(this);
    iff.e = e;

    if(!pexpect(this, TOKEN_KIND_CLOSE_PAREN)) {
        expect(TOKEN_KIND_CLOSE_PAREN);
        return (If) {0};       
    }

    padvance(this);

    if(!pexpect(this, TOKEN_KIND_OPEN_CURLY)) {
        Node *n = parse_node(this, true);
        APPEND(&iff.body, n);
        return iff;
    }

    padvance(this);

    Body body = parse_body(this);
    iff.body = body;

    if(!pexpect(this, TOKEN_KIND_CLOSE_CURLY)) {
        expect(TOKEN_KIND_CLOSE_CURLY);
        return (If) {0};   
    }

    padvance(this);

    return iff;
}

For parse_for(Parser *this) {
    if(!pexpect(this, TOKEN_KIND_FOR)) {
        expect(TOKEN_KIND_FOR);
        return (For) {0};   
    }

    padvance(this);

    For forr = {0};

    if(!pexpect(this, TOKEN_KIND_OPEN_CURLY)) {
        if(trypeakahead(this, 2).kind != TOKEN_KIND_EQUAL) {
            Expression *e = parse_expression(this);
            forr.v = NULL;
            forr.e = e;
        } else {
            VariableDeclaration var = parse_variable_declaration(this);
            forr.v = vardec(var.type, var.name, var.expr);
            
            if(!(pexpect(this, TOKEN_KIND_SEMI_COLON) || pexpect(this, TOKEN_KIND_OPEN_CURLY))) {
                expect(TOKEN_KIND_OPEN_CURLY);
                return (For) {0};
            }

            if(pexpect(this, TOKEN_KIND_SEMI_COLON)) {
                padvance(this);
                Expression *e = parse_expression(this);
                forr.e = e;
            }
        }
    }

    if(!pexpect(this, TOKEN_KIND_OPEN_CURLY)) {
        expect(TOKEN_KIND_OPEN_CURLY);
        return (For) {0};
    }

    padvance(this);

    Body body = parse_body(this);
    forr.body = body;

    if(!pexpect(this, TOKEN_KIND_CLOSE_CURLY)) {
        expect(TOKEN_KIND_CLOSE_CURLY);
        return (For) {0};
    }

    padvance(this);

    return forr;
}

VariableReassignement parse_variable_reassignement(Parser *this) {
    if(!pexpect(this, TOKEN_KIND_IDENTIFIER)) {
        expect(TOKEN_KIND_IDENTIFIER);
        return (VariableReassignement) {0};
    }

    VariableReassignement varres = {0};
    varres.name = ppeek(this).value;

    padvance(this);

    if(!pexpect(this, TOKEN_KIND_EQUAL)) {
        expect(TOKEN_KIND_EQUAL);
        return (VariableReassignement) {0};
    }

    padvance(this);

    Expression *e = parse_expression(this);
    varres.expr = e;

    return varres;
}

Node *parse_function_declaration_node(Parser *this) {
    FunctionDeclaration funcdecl = parse_function_declaration(this);
    return nodeas_funcdecl(funcdecl);
}

Node *parse_function_call_node(Parser *this) {
    FunctionCall funccall = parse_function_call(this);
    return nodeas_funccall(funccall);
}

Node *parse_variable_declaration_node(Parser *this) {
    VariableDeclaration vardec = parse_variable_declaration(this);
    return nodeas_vardec(vardec);
}

Node *parse_if_node(Parser *this) {
    If iff = parse_if(this);
    return nodeas_if(iff);
}

Node *parse_for_node(Parser *this) {
    For forr = parse_for(this);
    return nodeas_for(forr);
} 

Node *parse_variable_reassignement_node(Parser *this) {
    VariableReassignement varres = parse_variable_reassignement(this);
    return nodeas_varres(varres);
}


Node *parse_node(Parser *this, bool infuncbody) {
    Token t = ppeek(this);
    
    if(!infuncbody) {
        if(t.kind == TOKEN_KIND_FN) {
            return parse_function_declaration_node(this);
        }
    }

    if(t.kind == TOKEN_KIND_RETURN) {
        padvance(this);
        Expression *e = parse_expression(this);
        return nodeas_return(e);
    }

    if(t.kind == TOKEN_KIND_IF) {
        return parse_if_node(this);
    }

    if(t.kind == TOKEN_KIND_FOR) {
        return parse_for_node(this);
    }

    if(t.kind == TOKEN_KIND_IDENTIFIER) {
        if(trypeakahead(this, 1).kind == TOKEN_KIND_OPEN_PAREN) {
            return parse_function_call_node(this);
        }

        if(trypeakahead(this, 1).kind == TOKEN_KIND_EQUAL) {
            return parse_variable_reassignement_node(this);
        }

        if(trypeakahead(this, 2).kind == TOKEN_KIND_EQUAL) {
            return parse_variable_declaration_node(this);
        }
    }

    throw(
        error(
            PARSER,
            errfromparser(
                parserror(
                    TOKEN_KIND_NONE,
                    INVALID_START_OF_STATEMENT,
                    this
                )
            )
        )
    );

    return NULL;
}

Node *parse_root_node(Parser *this) {
    return parse_node(this, false);
}

AST *parse_root(Parser *this) {
    AST *out = ast();

    while(!pend(this)) {
        Node *n = parse_root_node(this);
        APPEND(out, n);
    }

    return out;
}

AST *parse(Parser *this) {
    AST *root = parse_root(this);
    return root;
}

