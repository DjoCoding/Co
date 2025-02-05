#ifndef SV_H
#define SV_H

#include <stdio.h>
#include <stdbool.h>

#define SV_NULL (SV) { .content = NULL, .count = 0 }
#define SV_FMT       "%.*s"
#define SV_UNWRAP(s) (int)s.count, s.content

typedef struct {
    char *content;
    size_t count;
} SV;


// returns a string view
SV sv(char *content, size_t count);

// returns a string view from a c string (c string: ends with NULL terminator)
SV svc(char *content);

// returns a sub string view starting from a value
SV svsub(SV s, size_t from, size_t count);

// compares two string views
bool svcmp(SV a, SV b);


#endif