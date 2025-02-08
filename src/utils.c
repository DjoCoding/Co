#include "utils.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "malloc.h"

bool isdir(const char *path) {
    struct stat st;
    if(stat(path, &st) == 0) { 
        return S_ISDIR(st.st_mode);
    }
    return false;
}

// remove the dir content
bool removedirc(const char *dirname) {
    DIR *dir = opendir(dirname);
    if(!dir) {
        return false;
    }

    struct dirent *entry;
    char path[FILENAME_MAX] = {0};

    while(true) {
        entry = readdir(dir);
        if(!entry) { break; }
    
        if(strcmp(entry->d_name, ".") == 0) {
            continue;
        }

        if(strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        if(isdir(path)) {
            removedirc(path);
            rmdir(path);
        } else {
            remove(path);
        }
    }

    return closedir(dir) == 0;
}

char *fcontent(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if(!f) { return NULL; }

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = alloc(sizeof(char) * (fsize + 1));
    fread(content, sizeof(char), fsize, f);
    content[fsize] = 0;

    fclose(f);
    return content;
}

// buffer should be a c string
bool fwritec(const char *filename, char *buffer) {
    FILE *f = fopen(filename, "w");
    if(!f) { return false; }
    fwrite(buffer, sizeof(char), strlen(buffer), f);
    fclose(f);
    return true;
}

bool createdir(const char *dirname) {
    int result = mkdir(dirname, 0777);
    if(result == 0) { return true; }

    if(errno == EEXIST) {
        return removedirc(dirname);
    }

    return true;
}

bool fcopy(const char *filename, const char *to) {
    char *content = fcontent(filename);
    if(!content) { return false; }
    bool success = fwritec(to, content);
    free(content);
    return success;
}