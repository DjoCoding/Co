#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

char *fcontent(const char *filepath);
bool fcopy(const char *filename, const char *to);
bool createdir(const char *dirname);

#endif