#include <stdio.h>

/**
 * Converts a UTF-8 `in` file into a UTF-32 `out` file
 */
int convUtf8to32(FILE* in, FILE* out);

/**
 * Converts a UTF-32 (Little or Big Endian) `in` file into a UTF-8 `out` file
 */
int convUtf32to8(FILE* in, FILE* out);
