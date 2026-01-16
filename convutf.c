#include "convutf.h"
#include "utils.h"
#include "unicode_types.h"
#include <stdio.h>
#include <stdlib.h>

int convUtf8to32(FILE* in, FILE* out) {
    /**
     * Convert UTF-8 to **Little Endian**
     * UTF-32. I guess this is arbitrary.
     */
    if (fwrite(BOM_UTF32_LE, sizeof(unsigned char), BOM_SIZE, out) != BOM_SIZE) {
        fprintf(stderr, "convUtf8to32: failed to write LE BOM to converted UTF-32 file!\n");
        return -1;
    }
    long fSize = get_filesize(in);
    if (fSize == -1) return -1;

    unsigned int* utf32UnsigIntArr = calloc(fSize, sizeof(unsigned int));
    if (!utf32UnsigIntArr) {
        fprintf(stderr, "convUtf8to32: failed to allocate UTF-32 unsigned integer array!\n");
        return -1;
    }

    /**
     * index for the UTF-32 unsigned int array, 
     * as well as the actual number of UTF-32 characters
     * created from the UTF-8 bytes
     */
    unsigned i = 0;
    int c = 0;
    while ((c = fgetc(in)) != EOF) {
        if ((c & 0x80) == 0) {
            // 0x80 = 1000 0000
            // 1 byte (0xxxxxxx)
            // U+0000 to U+007F

            // extract 7 LSBs of the byte
            c &= 0x7F;

            utf32UnsigIntArr[i] |= c;
        } else if ((c & 0xE0) == 0xC0) {
            // 0xE0 = 1110 0000
            // 2 bytes (110xxxxx)
            // U+0080 to U+07FF
            
            // extract 5 LSBs from the first byte
            c &= 0x1F;
            utf32UnsigIntArr[i] |= (c << 6);
            
            // extract 6 LSBs from the second byte
            fread(&c, 1, 1, in);
            c &= 0x3F;
            utf32UnsigIntArr[i] |= c;
            
        } else if ((c & 0xF0) == 0xE0) {
            // 0xF0 = 1111 0000
            // 3 bytes (1110xxxx)
            // U+0800 to U+FFFF

            // extract 4 LSBs from the first byte
            c &= 0x0F;
            utf32UnsigIntArr[i] |= (c << 12);

            // extract 6 LSBs from second and third byte
            fread(&c, 1, 1, in);
            c &= 0x3F;
            utf32UnsigIntArr[i] |= (c << 6);

            fread(&c, 1, 1, in);
            c &= 0x3F;
            utf32UnsigIntArr[i] |= c;
       
        } else if ((c & 0xF8) == 0xF0) {
            // 0xF8 = 1111 1000
            // 4 bytes (11110xxx)
            // U+10000 to U+10FFFF

            // extract 3 LSBs from first byte
            c &= 0x07;
            utf32UnsigIntArr[i] |= (c << 18);

            // extract 6 LSBs from second, third, and fourth bytes
            fread(&c, 1, 1, in);
            c &= 0x3F;
            utf32UnsigIntArr[i] |= (c << 12);

            fread(&c, 1, 1, in);
            c &= 0x3F;
            utf32UnsigIntArr[i] |= (c << 6);

            fread(&c, 1, 1, in);
            c &= 0x3F;
            utf32UnsigIntArr[i] |= c;

        } else {
            fprintf(stderr, "invalid UTF-8 character in the given UTF-8 file!\n");
            return -1;
        }
        i++;
    }

    if (fwrite(utf32UnsigIntArr, sizeof(unsigned int), i, out) != i) {
        fprintf(stderr, "convUtf8to32: failed to write UTF-32 characters in converted file!\n");
        return -1;
    }

    return 0;
}

int convUtf32to8(FILE* in, FILE* out)  {
    unsigned char bom[BOM_SIZE] = {};
    if (fread(bom, sizeof(unsigned char), BOM_SIZE, in) != BOM_SIZE) {
        fprintf(stderr, "failed to read BOM of given UTF-32 file!\n");
        return -1;
    }

    int isBigEndian = 0;
    if (*bom == 255) {   
        // BOM starts with FF: Little Endian file
    } else if (*bom == 0) {  
        // BOM starts with 00: Big Endian file
        isBigEndian = 1;
    } else {
        fprintf(stderr, "detected invalid BOM in the given UTF-32 file!\n");
        return -1;
    }

    unsigned int value = 0;
    unsigned char utf8value = 0;
    while (fread(&value, sizeof(value), 1, in) > 0) {
        if (isBigEndian) {  // if file is BE, convert the current value to LE so we work with a single bit pattern
            unsigned int lastByte, secondByte, thirdByte, firstByte;

            // makes most significant BE byte the least significant byte in LE: shift right 24 bits (3 bytes)
            lastByte = (value & 0xFF000000) >> 24;

            // switches position of the middle bytes
            secondByte = (value & 0x00FF0000) >> 8;
            thirdByte = (value & 0x0000FF00) << 8;

            // makes least significant BE byte the most significant byte in LE: shift left 24 bits (3 bytes)
            firstByte = (value & 0x000000FF) << 24;

            // inclusive ORing the bytes to compose final value in LE
            value = firstByte | thirdByte | secondByte | lastByte;
        }
        if (value <= 0x7F) {
            // 1 byte in UTF-8

            // gets the 7 LSBs
            utf8value = value & 0x7F; 
            
            fwrite(&utf8value, sizeof(utf8value), 1, out);  

        } else if (value <= 0x7FF) {
            // 2 bytes in UTF-8

            // gets the 5 LSBs of the first byte
            utf8value = (value >> 6) & 0x1F;

            // starts new byte with 110xxxxx
            utf8value |= 0xC0;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // gets the 6 LSBs of the second byte
            utf8value = value & 0x3F;
            
            // starts new byte with 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

        } else if (value <= 0xFFFF) {
            // 3 bytes in UTF-8

            // gets the 4 LSBs of the first byte
            utf8value = (value >> 12) & 0x0F;
            
            // starts new byte with 1110xxxx
            utf8value |= 0xE0;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // gets the 6 LSBs of the second byte
            utf8value = (value >> 6) & 0x3F;
            
            // starts new byte with 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // gets the 6 LSBs of the third byte
            utf8value = value & 0x3F;
            
            // starts new byte with 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

        } else if (value <= 0x10FFFF) {
            // 4 bytes in UTF-8

            // gets the 3 LSBs of the first byte
            utf8value = (value >> 18) & 0x07;

            // starts new byte with 11110xxx
            utf8value |= 0xF0;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // gets the 6 LSBs of the second byte
            utf8value = (value >> 12) & 0x3F;
            
            // starts new byte with 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // gets the 6 LSBs of the third byte
            utf8value = (value >> 6) & 0x3F;
            
            // starts new byte with 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // gets the 6 LSBs of the fourth byte
            utf8value = value & 0x3F;
            
            // starts new byte with 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

        } else {
            fprintf(stderr, "found invalid UTF-8 character in the given UTF-32 file\n");
            return -1;
        }
    }
    
    return 0;
}
