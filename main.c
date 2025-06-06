#include "convutf.h"
#include <stdio.h>

int main() {
    FILE* utf8In, *utf32LeIn, *utf32BeIn;
    utf8In = fopen("utf8_peq.txt", "rb");
    utf32LeIn = fopen("utf32_peq.txt", "rb");
    utf32BeIn = fopen("utf32_peq_be.txt", "rb");

    if (!utf8In || !utf32LeIn || !utf32BeIn) {
        printf("Couldn't open in files!\n");
        return -1;
    }

    FILE* utf8Out, *utf32LeOut, *utf32BeOut;
    utf8Out = fopen("utf8_peq_convertido_utf32.txt", "wb");
    utf32LeOut = fopen("utf32_peq_convertido_utf8.txt", "wb");
    utf32BeOut = fopen("utf32_peq_be_convertido_utf8.txt", "wb");

    if (!utf8Out || !utf32LeOut || !utf32BeOut) {
        printf("Could not create out files!\n");
        return -1;
    }

    // UTF-8 to UTF-32
    convUtf8to32(utf8In, utf8Out);

    // UTF-32 BE and LE to UTF-8
    convUtf32to8(utf32LeIn, utf32LeOut);
    convUtf32to8(utf32BeIn, utf32BeOut);

    fclose(utf8In);
    fclose(utf32LeIn);
    fclose(utf32BeIn);
    fclose(utf8Out);
    fclose(utf32LeOut);
    fclose(utf32BeOut);

    return 0;
}
