#ifndef STD_COSTRING_H
#define STD_COSTRING_H

#include <stdio.h>

typedef struct {
    char *content;
    size_t count;
} CoString;

CoString costring(const char *cstr);
CoString costring_concat(CoString a, CoString b);

#ifdef STD_COSTRING_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

CoString costring(const char *cstr) {
    size_t count = strlen(cstr);
    char *content = malloc(sizeof(char) * count);
    memcpy(content, cstr, count);
    return (CoString) {
        .content = content, 
        .count = count
    };
} 


CoString costring_concat(CoString a, CoString b) {
    size_t count = a.count + b.count;
    char *content = malloc(sizeof(char) * count);
    memcpy(content, a.content, a.count);
    memcpy(content + a.count, b.content, b.count);
    return (CoString) {
        .content = content, 
        .count = count
    };
}


#endif

#endif