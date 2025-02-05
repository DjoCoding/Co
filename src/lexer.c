#include "lexer.h"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "malloc.h"

typedef struct {
    TokenKind kind;
    const char *value;
} TokenKindValueObject;

TokenKindValueObject pre_defined_tokens[] = {
    { TOKEN_KIND_CLOSE_PAREN, ")" },
    { TOKEN_KIND_OPEN_PAREN, "(" },
    { TOKEN_KIND_OPEN_CURLY, "{" },
    { TOKEN_KIND_CLOSE_CURLY, "}" },
    { TOKEN_KIND_COMMA, "," },
    { TOKEN_KIND_PLUS, "+" },
    { TOKEN_KIND_MINUS,  "-" },
    { TOKEN_KIND_STAR, "*" },
    { TOKEN_KIND_SLASH, "/" },
    { TOKEN_KIND_EQUAL, "=" },
    { TOKEN_KIND_COLON, ":" },
    { TOKEN_KIND_FN, "fn" }, 
    { TOKEN_KIND_RETURN, "return" },
};

Lexer *lexer(SV source) {
    Lexer *this = alloc(sizeof(Lexer));
    this->source = source;
    this->current = 0;
    return this;
}

char *lpeekp(Lexer *this) {
    return &this->source.content[this->current];
}

char lpeek(Lexer *this) {
    return *lpeekp(this);
}

void ladvance_ahead(Lexer *this, size_t ahead) {
    this->current += ahead;
}

void ladvance(Lexer *this) {
    ++this->current;
}

bool lend(Lexer *this) {
    return this->current >= this->source.count;
}

Token ltoken(Lexer *this) {
    for(size_t i = 0; i < LENGTH(pre_defined_tokens); ++i) {
        // get the string view from the current pre defined token
        SV current = svc((char *)pre_defined_tokens[i].value);
        
        // compare the string view value with the sub string view from the source code of the same length
        if(svcmp(current, svsub(this->source, this->current, current.count))) {
            ladvance_ahead(this, current.count);
            return token(current, pre_defined_tokens[i].kind);
        }
    }

    if(isdigit(lpeek(this))) {
        //FIXME: fix this by making this an independant function that lexes and checks for errors
        size_t size = 0;
        char *start = lpeekp(this);
        while(!lend(this)) {
            if(!isdigit(lpeek(this))) { break; }
            ++size;
            ladvance(this);
        }
        return token(sv(start, size), TOKEN_KIND_INTEGER);
    }

    if(lpeek(this) == '\"') {
        ladvance(this);
        size_t size = 0;
        char *start = lpeekp(this);
        while(!lend(this)) {
            if(lpeek(this) == '\"') { break; }
            ++size;
            ladvance(this);
        }
        
        if(lpeek(this) != '\"') { 
            //FIXME: raise error 
            abort();
        }

        ladvance(this);
        return token(size == 0 ? SV_NULL : sv(start, size), TOKEN_KIND_STRING);
    }

    if(isalpha(lpeek(this))) {
        size_t size = 0;
        char *start = lpeekp(this);
        while(!lend(this)) {
            if(!isalnum(lpeek(this))) { break; }
            ++size; 
            ladvance(this); 
        }
        return token(sv(start, size), TOKEN_KIND_IDENTIFIER);
    }

    assert(false && "undefined token");
}   

ARRAY_OF(Token) lex(Lexer *this) {
    ARRAY_OF(Token) tokens = ARRAY(Token);

    while(!lend(this)) {
        if(isspace(lpeek(this))) { 
            ladvance(this);
            continue;
        }

        Token token = ltoken(this);
        APPEND(&tokens, token);
    }

    APPEND(&tokens, END_TOKEN);
    return tokens;
}
