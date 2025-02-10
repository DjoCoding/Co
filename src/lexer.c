#include "lexer.h"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "malloc.h"
#include "coerror.h"

typedef struct {
    TokenKind kind;
    const char *value;
} TokenKindValueObject;

TokenKindValueObject pre_defined_tokens[] = {
    { TOKEN_KIND_SEMI_COLON, ";" },
    { TOKEN_KIND_CLOSE_PAREN, ")" },
    { TOKEN_KIND_OPEN_PAREN, "(" },
    { TOKEN_KIND_OPEN_CURLY, "{" },
    { TOKEN_KIND_CLOSE_CURLY, "}" },
    { TOKEN_KIND_COMMA, "," },
    { TOKEN_KIND_PLUS, "+" },
    { TOKEN_KIND_MINUS,  "-" },
    { TOKEN_KIND_STAR, "*" },
    { TOKEN_KIND_SLASH, "/" },
    { TOKEN_KIND_EQ, "==" },
    { TOKEN_KIND_EQUAL, "=" },
    { TOKEN_KIND_COLON_EQ, ":=" },
    { TOKEN_KIND_COLON, ":" },
    { TOKEN_KIND_LESS_OR_EQ, "<=" },
    { TOKEN_KIND_LESS, "<" },
    { TOKEN_KIND_GREATER_OR_EQ, ">=" },
    { TOKEN_KIND_GREATER, ">" },
    { TOKEN_KIND_IF, "if" },
    { TOKEN_KIND_FN, "fn" }, 
    { TOKEN_KIND_FOR, "for" }, 
    { TOKEN_KIND_RETURN, "return" },
};

Lexer *lexer(const char *filename) {
    Lexer *this = alloc(sizeof(Lexer));
    this->source = SV_NULL;
    this->current = 0;
    this->filename = svc((char *)filename);
    this->loc = location(1, 1);
    this->currentok = TOKEN_NONE;
    return this;
}

void lexer_set_source(Lexer *this, SV source) {
    this->source = source;
}

LexerError lexerror(ErrorCode code, Lexer *this) {
    return (LexerError) {
        .code = code,
        .current = this->current,
        .loc = this->loc,
        .lasttok = this->currentok,
    };
}

char *lpeekp(Lexer *this) {
    return &this->source.content[this->current];
}

char lpeek(Lexer *this) {
    return *lpeekp(this);
}

void ladvance(Lexer *this) {
    char c = lpeek(this);

    switch(c) {
        case '\n': 
            this->loc.line += 1;
            this->loc.offset = 1;
            break;
        case '\t':
            this->loc.offset += 4;
            break;
        default:
            this->loc.offset += 1;
    }

    ++this->current;
}

void ladvance_ahead(Lexer *this, size_t ahead) {
    for(size_t i = 0; i < ahead; ++i) {
        ladvance(this);
    }
}


bool lend(Lexer *this) {
    return this->current >= this->source.count;
}

Token trylexpredef(Lexer *this) {
    Location loc = this->loc;

    for(size_t i = 0; i < LENGTH(pre_defined_tokens); ++i) {
        // get the string view from the current pre defined token
        SV current = svc((char *)pre_defined_tokens[i].value);
        
        // compare the string view value with the sub string view from the source code of the same length
        if(svcmp(current, svsub(this->source, this->current, current.count))) {
            ladvance_ahead(this, current.count);
            this->currentok = token(current, pre_defined_tokens[i].kind, loc);
            return this->currentok;
        }
    }

    return TOKEN_NONE;
}

Token trylexnumber(Lexer *this) {
    Location loc = this->loc;

    if(isdigit(lpeek(this))) {
        //FIXME: fix this by making this an independant function that lexes and checks for errors
        size_t size = 0;
        char *start = lpeekp(this);
        while(!lend(this)) {
            if(!isdigit(lpeek(this))) { break; }
            ++size;
            ladvance(this);
        }
        this->currentok = token(sv(start, size), TOKEN_KIND_INTEGER, loc);
        return this->currentok;
    }

    return TOKEN_NONE;
}

Token trylexstring(Lexer *this) {
    Location loc = this->loc;

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
            this->currentok = token(sv(start, size), TOKEN_KIND_INVALID, this->loc);
            throw(
                error(
                    LEXER, 
                    errfromlexer(lexerror(INVALID_STRING, this)),
                    this->filename
                )
            );
            return TOKEN_NONE;
        }

        ladvance(this);
        this->currentok = token(size == 0 ? SV_NULL : sv(start, size), TOKEN_KIND_STRING, loc);
        return this->currentok;
    }

    return TOKEN_NONE;
}

Token trylexidentifier(Lexer *this) {
    Location loc = this->loc;

    if(isalpha(lpeek(this))) {
        size_t size = 0;
        char *start = lpeekp(this);
        while(!lend(this)) {
            if(!isalnum(lpeek(this))) { break; }
            ++size; 
            ladvance(this); 
        }
        this->currentok = token(sv(start, size), TOKEN_KIND_IDENTIFIER, loc);
        return this->currentok;
    }

    return TOKEN_NONE;
}

Token ltoken(Lexer *this) {
    Token tok = TOKEN_NONE;

    tok = trylexpredef(this);
    if(tok.kind != TOKEN_KIND_NONE) { return tok; }

    tok = trylexnumber(this);
    if(tok.kind != TOKEN_KIND_NONE) { return tok; }
    
    tok = trylexstring(this);
    if(tok.kind != TOKEN_KIND_NONE) { return tok; }
    
    tok = trylexidentifier(this);
    if(tok.kind != TOKEN_KIND_NONE) { return tok; }
    
    this->currentok = token(sv(lpeekp(this), 1), TOKEN_KIND_INVALID, this->loc);
    throw(
        error(
            LEXER, 
            errfromlexer(lexerror(INVALID_TOKEN, this)),
            this->filename
        )
    );

    // to by-pass the gcc warning
    return TOKEN_NONE;
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
