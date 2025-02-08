#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "ast.h"
#include "shared.h"

void log_ast(AST *a);
void log_tokens(ARRAY_OF(Token) tokens);

#endif