#include "convutf.h"
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <stdlib.h>
#include <errno.h>

#define MAX_LEN 128
#define OUTPUT_FOLDER "generated"
#define CONVERTED_FILE_TAG "CONV_"

static char* get_basename(const char* filepath);

int main(int argc, char* argv[]) {
    int ret = -1;
    int read = 0;
    int status = 0;
    char* basename = NULL;
    FILE* fin = NULL;
    FILE* fout = NULL;

    if (argc != 2) {
        fprintf(stderr, "usage: ./utfConverter <path/to/file>\n");
        ret = -1;
        goto cleanup;
    }
    
    const char* filename = argv[1];
    if (!filename) {
        fprintf(stderr, "no filename provided!\n");
        ret = -1;
        goto cleanup;
    }

    fin = fopen(filename, "rb");
    if (!fin) {
        fprintf(stderr, "failed to open file %s\n", filename);
        ret = -1;
        goto cleanup;
    }

    basename = get_basename(filename);
    if (!basename) {
        fprintf(stderr, "failed get basename for file %s\n", filename);
        ret = -1;
        goto cleanup;
    }

    // converted filename creation
    char convertedFileName[MAX_LEN] = {0};
    read = snprintf(convertedFileName, MAX_LEN, "%s%s", CONVERTED_FILE_TAG, basename);
    if (read >= MAX_LEN) {
        printf("output filename buffer exceeded, truncating string\n");
    }

    // output folder creation
    mode_t dirPermissionsFlag = S_IRWXU | S_IRWXG | S_IRWXO; // 0777
    status = mkdir(OUTPUT_FOLDER, dirPermissionsFlag);
    if ((status != 0) && (errno != EEXIST)) {
        fprintf(stderr, "failed to create folder %s\n", OUTPUT_FOLDER);
        ret = -1;
        goto cleanup;
    }

    // creation of file in output folder
    char filenameInOutputFolder[MAX_LEN] = {0};
    read = snprintf(filenameInOutputFolder, MAX_LEN, "./%s/%s", OUTPUT_FOLDER, convertedFileName);
    if (read >= MAX_LEN) {
        printf("output filename buffer exceeded, truncating string\n");
    }

    // converted file creation
    fout = fopen(filenameInOutputFolder, "wb");
    if (!fout) {
        fprintf(stderr, "failed to create converted %s\n", convertedFileName);
        ret = -1;
        goto cleanup;
    }

    // detect if fin is UTF-8 or 32
    // status = 0;
    // if 8 -> status = convUtf8to32(fin, fout);
    // elif 32 -> status = convUtf32to8(fin, fout);
    // else -> panic

    cleanup:
        free(basename);
        if(fin) fclose(fin);
        if(fout) fclose(fout);
        return ret;
}

static char* get_basename(const char* filepath) {
    if (!filepath) {
        fprintf(stderr, "no filepath provided to extract a file basename!\n");
        return NULL;
    }
    char* basename = malloc(MAX_LEN * sizeof(char));
    if (!basename) {
        fprintf(stderr, "failed to create array for basename\n");
        return NULL;
    }

    int i = 0;
    char* ptr = filepath;
    while (*ptr != '\0') {
        if (*ptr == '/') {
            i = 0;
            ptr++;
            continue;
        }
        if (i >= MAX_LEN) {
            printf("basename buffer exceeded, truncating string\n");
            break;
        }
        basename[i] = *ptr;
        ++i;
        ptr++;
    }
    basename[i] = '\0';
    // TODO: edge cases
    return basename;
}
