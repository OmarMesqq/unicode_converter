#ifndef UNICODE_TYPES_H
#define UNICODE_TYPES_H

/**
 * The Byte Order Mark is a sequence of four
 * bytes in UTF-32 files that indicates whether
 * the file has Big or Little Endian byte ordering
 */
#define BOM_SIZE 4

static const unsigned char BOM_UTF32_BE[BOM_SIZE] = {0x00, 0x00, 0xFE, 0xFF};
static const unsigned char BOM_UTF32_LE[BOM_SIZE] = {0xFF, 0xFE, 0x00, 0x00};

#endif