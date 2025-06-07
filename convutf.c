#include "convutf.h"
#include <stdio.h>

int convUtf8to32(FILE* in, FILE* out) {
    unsigned char c = 0;
    unsigned int utf32Code = 0;
    unsigned char mask = 0x00;

    // Little Endian BOM: FF FE 00 00
    unsigned char bom[4] = {255, 254, 0, 0};
    if (fwrite(bom, sizeof(char), 4, out) != 4) {
        fprintf(stderr, "%s", "I/O error: failed to write BOM!\n");
        return -1;
    }

    while (fread(&c, 1, 1, in) > 0) {
        if ((c & 0x80) == 0) {
            // 0x80 = 1000 0000
            // 1 byte (0xxxxxxx)
            // ASCII compatible
            // U+0000 to U+007F

            // extract 7 LSBs of the byte
            mask = 0x7F;
            c &= mask;

            utf32Code |= c;

        } else if ((c & 0xE0) == 0xC0) {
            // 0xE0 = 1110 0000
            // 2 bytes (110xxxxx)
            // U+0080 to U+07FF
            
            // extract 5 LSBs from the first byte
            mask = 0x1F;
            c &= mask;
            utf32Code |= (c << 6);
            
            // extract 6 LSBs from the second byte
            mask = 0x3F;
            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= c;
            
        } else if ((c & 0xF0) == 0xE0) {
            // 0xF0 = 1111 0000
            // 3 bytes (1110xxxx)
            // U+0800 to U+FFFF

            // extract 4 LSBs from the first byte
            mask = 0x0F;
            c &= mask;
            utf32Code |= (c << 12);

            // extract 6 LSBs from second and third byte
            mask = 0x3F;
            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= (c << 6);

            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= c;
       
        } else if ((c & 0xF8) == 0xF0) {
            // 0xF8 = 1111 1000
            // 4 bytes (11110xxx)
            // U+10000 to U+10FFFF

            // extract 3 LSBs from first byte
            mask = 0x07;
            c &= mask;
            utf32Code |= (c << 18);

            // extract 6 LSBs from second, third, and fourth bytes
            mask = 0x3F;
            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= (c << 12);

            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= (c << 6);

            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= c;

        } else {
            fprintf(stderr, "%s", "I/O error: invalid UTF-8 character!\n");
            return -1;
        }

        if (fwrite(&utf32Code, sizeof(utf32Code), 1, out) != 1) {
            fprintf(stderr, "%s", "I/O error: could not write equivalent integer of UTF-8 character!\n");
            return -1;
        }
        utf32Code = 0;
    }

    return 0;
}

int convUtf32to8(FILE* in, FILE* out)  {
    unsigned char bom[4] = {};
    if (fread(bom, sizeof(unsigned char), 4, in) != 4) {
        fprintf(stderr, "%s", "I/O error: failed to read BOM!\n");
        return -1;
    }

    int isBigEndian = 0;
    if (*bom == 255) {   
        // BOM starts with FF: Little Endian file
    } else if (*bom == 0) {  
        // BOM starts with 00: Big Endian file
        isBigEndian = 1;
    } else {
        fprintf(stderr, "%s","Invalid BOM!\n");
        return -1;
    }

    unsigned int value = 0;
    unsigned char utf8value = 0;
    while (fread(&value, sizeof(value), 1, in) > 0) {
        if (isBigEndian) {
            // if file is BE, convert the current value to LE so we work with a single bit pattern
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
            // 4 bytes em UTF-8

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
            fprintf(stderr, "%s", "I/O error: invalid UTF-8 character!\n");
            return -1;
        }
    }
    
    return 0;
}
