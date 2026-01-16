#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

char* get_basename(const char* filepath, unsigned maxBaseNameLength) {
    if (!filepath) {
        fprintf(stderr, "get_basename: no filepath provided to extract a file basename!\n");
        return NULL;
    }
    char* basename = malloc(maxBaseNameLength * sizeof(char));
    if (!basename) {
        fprintf(stderr, "get_basename: failed to create array for basename\n");
        return NULL;
    }

    unsigned int i = 0;
    char* ptr = (char*) filepath;
    while (*ptr != '\0') {
        if (*ptr == '/') {
            i = 0;
            ptr++;
            continue;
        }
        if (i >= maxBaseNameLength) {
            printf("get_basename: basename buffer exceeded, truncating string\n");
            break;
        }
        basename[i] = *ptr;
        ++i;
        ptr++;
    }
    basename[i] = '\0';
    return basename;
}

long get_filesize(FILE* f) {
    fseek(f, 0 , SEEK_END);
    long length = ftell(f);
    rewind(f);

    if (length == -1L) {
        fprintf(stderr, "get_filesize: failed to read file size using 'ftell'\n");
        return -1;
    }
    return length;
}
