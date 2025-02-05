#include "sv.h"
#include <string.h>

SV sv(char *content, size_t count) {
    return (SV) {
        .content = content, 
        .count = count
    };
}

SV svc(char *content) {
    return sv(content, strlen(content));
}

SV svsub(SV s, size_t from, size_t count) {
    if(count > s.count - from) { return s; }
    return sv(s.content + (sizeof(char) * from), count);
}

bool svcmp(SV a, SV b) {
    if(a.count != b.count) { return false; }
    return memcmp(a.content, b.content, a.count) == 0; 
}