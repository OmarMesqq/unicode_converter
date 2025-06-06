#include "convutf.h"
#include <stdio.h>

int main() {
    FILE* entradaUtf8, *entradaUtf32Le, *entradaUtf32Be;
    entradaUtf8 = fopen("utf8_peq.txt", "rb");
    entradaUtf32Le = fopen("utf32_peq.txt", "rb");
    entradaUtf32Be = fopen("utf32_peq_be.txt", "rb");

    if (!entradaUtf8 || !entradaUtf32Le || !entradaUtf32Be) {
        printf("Falha ao abrir arquivos de entrada! Abortando...\n");
        return -1;
    }

    FILE* saidaUtf8, *saidaUtf32Le, *saidaUtf32Be;
    saidaUtf8 = fopen("utf8_peq_convertido_utf32.txt", "wb");
    saidaUtf32Le = fopen("utf32_peq_convertido_utf8.txt", "wb");
    saidaUtf32Be = fopen("utf32_peq_be_convertido_utf8.txt", "wb");

    if (!saidaUtf8 || !saidaUtf32Le || !saidaUtf32Be) {
        printf("Falha ao criar os arquivos de saída (convertidos)! Abortando...\n");
        return -1;
    }

    // Convertendo arquivo UTF-8 para UTF-32
    convUtf8p32(entradaUtf8, saidaUtf8);

    // Convertendo arquivos UTF-32 para UTF-8
    convUtf32p8(entradaUtf32Le, saidaUtf32Le);
    convUtf32p8(entradaUtf32Be, saidaUtf32Be);

    fclose(entradaUtf8);
    fclose(entradaUtf32Le);
    fclose(entradaUtf32Be);
    fclose(saidaUtf8);
    fclose(saidaUtf32Le);
    fclose(saidaUtf32Be);

    return 0;
}

