#ifndef PARSER_H
#define PARSER_H

#include "coerror.h"
#include "da.h"
#include "token.h"
#include "node.h"
#include "ast.h"

typedef struct {
    PreDefinedType type;
    const char *type_as_cstr;
} PreDefinedTypeMap;

extern const PreDefinedTypeMap predeftypes[PRE_DEFINED_TYPE_COUNT];
typedef struct {
    SV filename;
    ARRAY_OF(Token) tokens;
    size_t current;
} Parser;


Parser *parser(ARRAY_OF(Token) tokens);
AST *parse(Parser *this);

// specify which file to be parsed
void parserofile(Parser *this, const char *filename);


#endif