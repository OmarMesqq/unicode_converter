#include "convutf.h"
#include "unicode_types.h"
#include "libs/libcutilities/cutilities.h"
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_LEN 128
#define OUTPUT_FOLDER "generated"
#define CONVERTED_FILE_TAG "CONV_"

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

    basename = get_basename(filename, MAX_LEN);
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

    // detect if input file is UTF-32 BE, UTF-32 LE, or UTF-8
    unsigned char bom[BOM_SIZE] = {0};
    if (fread(bom, sizeof(unsigned char), BOM_SIZE, fin) != BOM_SIZE) {
        fprintf(stderr, "failed to read initial %d bytes of file %s to detect eventual BOM.", BOM_SIZE, filename);
        fprintf(stderr, "File may be too small\n");
        ret = -1;
        goto cleanup;
    }

    char isUtf8 = 0;
    if (memcmp(bom, BOM_UTF32_BE, 4) == 0) {
        printf("File is UTF-32 Big Endian\n");
    } 
    else if (memcmp(bom, BOM_UTF32_LE, 4) == 0) {
        printf("File is UTF-32 Little Endian\n");
    } 
    else {
        printf("BOM sequence not identified. Treating file as UTF-8\n");
        isUtf8 = 1; 
    }
    rewind(fin);    // restore bytes read to detect BOM

    // Do the actual conversion
    if (isUtf8) {
        status = convUtf8to32(fin, fout);
    } else {
        status = convUtf32to8(fin, fout);
    }

    if (status != 0) {
        fprintf(stderr, "failed to convert file!\n");
        ret = -1;
        goto cleanup;
    }

    ret = 0;
    cleanup:
        free(basename);
        if(fin) fclose(fin);
        if(fout) fclose(fout);
        return ret;
}
