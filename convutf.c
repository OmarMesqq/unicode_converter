#include "convutf.h"
#include <stdio.h>

int convUtf8to32(FILE* in, FILE* out) {
    unsigned char c = 0;
    unsigned int utf32Code = 0;
    unsigned char mask = 0x00;

    // BOM Little Endian: FF FE 00 00
    unsigned char bom[4] = {255, 254, 0, 0};
    if (fwrite(bom, sizeof(char), 4, out) != 4) {
        fprintf(stderr, "%s: %s", "Falha do tipo E/S:", "erro ao escrever BOM no arquivo convertido para UTF-32.\n");
        return -1;
    }

    while (fread(&c, 1, 1, in) > 0) {
        if ((c & 0x80) == 0) {
            // 80 (16) = 1000 0000 (2)
            // 1 byte (0xxxxxxx)
            // retrocompatível com ASCII
            // U+0000 a U+007F

            // extrai 7 bits menos significativos do primeiro (único) byte
            mask = 0x7F;
            c &= mask;

            utf32Code |= c;

        } else if ((c & 0xE0) == 0xC0) {
            // E0 (16) = 1110 0000 (2)
            // 2 bytes (110xxxxx)
            // U+0080 a U+07FF
            
            // extrai 5 bits menos significativos do primeiro byte
            mask = 0x1F;
            c &= mask;
            utf32Code |= (c << 6);
            
            // extrai 6 bits menos significativos do segundo byte
            mask = 0x3F;
            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= c;
            
        } else if ((c & 0xF0) == 0xE0) {
            // F0 (16) = 1111 0000 (2)
            // 3 bytes (1110xxxx)
            // U+0800 a U+FFFF

            // extrai 4 bits menos significativos do primeiro byte
            mask = 0x0F;
            c &= mask;
            utf32Code |= (c << 12);

            // extrai 6 bits menos significativos do segundo byte e do terceiro byte
            mask = 0x3F;
            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= (c << 6);

            fread(&c, 1, 1, in);
            c &= mask;
            utf32Code |= c;
       
        } else if ((c & 0xF8) == 0xF0) {
            // F8 (16) = 1111 1000
            // 4 bytes (11110xxx)
            // U+10000 a U+10FFFF

            // extrai 3 bits menos significativos do primeiro byte
            mask = 0x07;
            c &= mask;
            utf32Code |= (c << 18);

            // extrai 6 bits menos significativos dos segundo, terceiro e quarto bytes
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
            fprintf(stderr, "%s: %s", "Falha do tipo E/S:", "caractere UTF-8 inválido.\n");
            return -1;
        }

        if (fwrite(&utf32Code, sizeof(utf32Code), 1, out) != 1) {
            fprintf(stderr, "%s: %s", "Falha do tipo E/S:", "erro ao escrever inteiro correspondente ao caractere UTF-8.\n");
            return -1;
        }
        utf32Code = 0;
    }

    return 0;
}

int convUtf32to8(FILE* in, FILE* out)  {
    unsigned char bom[4] = {};
    if (fread(bom, sizeof(unsigned char), 4, in) != 4) {
        fprintf(stderr, "%s","Erro ao extrair BOM do arquivo de entrada (UTF-32)!\n");
        return -1;
    }

    int isBigEndian = 0;
    if (*bom == 255) {   
        // BOM inicia com FF, arquivo é Little Endian
    } else if (*bom == 0) {  
        // BOM inicia com 00, arquivo é Big Endian
        isBigEndian = 1;
    } else {
        fprintf(stderr, "%s","BOM inválido! Saindo...\n");
        return -1;
    }

    unsigned int value = 0;
    unsigned char utf8value = 0;
    while (fread(&value, sizeof(value), 1, in) > 0) {
        if (isBigEndian) {
            // caso arquivo seja BE, convertemos o valor atual para LE
            // assim temos apenas um padrão de lidar com os bits
            unsigned int ultimoByte, segundoByte, terceiroByte, primeiroByte;

            // joga byte MAIS significativo BE para o byte MENOS significativo LE: 24 bits (3 bytes) para direita
            ultimoByte = (value & 0xFF000000) >> 24;

            // troca posição dos bytes do meio
            segundoByte = (value & 0x00FF0000) >> 8;
            terceiroByte = (value & 0x0000FF00) << 8;

            // joga byte MENOS significativo BE para o byte MAIS significativo LE: 24 bits (3 bytes) para esquerda
            primeiroByte = (value & 0x000000FF) << 24;

            // reúne os bytes trocados
            value = primeiroByte | terceiroByte | segundoByte | ultimoByte;
        }
        if (value <= 0x7F) {
            // 1 byte em UTF-8

            // pega os 7 bits inferiores
            utf8value = value & 0x7F; 
            
            fwrite(&utf8value, sizeof(utf8value), 1, out);  

        } else if (value <= 0x7FF) {
            // 2 bytes em UTF-8

            // pega 5 bits inferiores do primeiro byte
            utf8value = (value >> 6) & 0x1F;

            // inicia byte com 110xxxxx
            utf8value |= 0xC0;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // pega 6 bits inferiores do segundo byte
            utf8value = value & 0x3F;
            
            // inicia byte com 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

        } else if (value <= 0xFFFF) {
            // 3 bytes em UTF-8

            // pega 4 bits inferiores do primeiro byte
            utf8value = (value >> 12) & 0x0F;
            
            // inicia byte com 1110xxxx
            utf8value |= 0xE0;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // pega 6 bits inferiores do segundo byte
            utf8value = (value >> 6) & 0x3F;
            
            // inicia byte com 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // pega 6 bits inferiores do terceiro byte
            utf8value = value & 0x3F;
            
            // inicia byte com 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

        } else if (value <= 0x10FFFF) {
            // 4 bytes em UTF-8

            // pega 3 bits inferiores do primeiro byte
            utf8value = (value >> 18) & 0x07;

            // inicia byte com 11110xxx
            utf8value |= 0xF0;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // pega 6 bits inferiores do segundo byte
            utf8value = (value >> 12) & 0x3F;
            
            // inicia byte com 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // pega 6 bits inferiores do terceiro byte
            utf8value = (value >> 6) & 0x3F;
            
            // inicia byte com 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

            // pega 6 bits inferiores do quarto byte
            utf8value = value & 0x3F;
            
            // inicia byte com 10xxxxxx
            utf8value |= 0x80;
            fwrite(&utf8value, sizeof(utf8value), 1, out);

        } else {
            fprintf(stderr, "%s: %s", "Falha do tipo E/S:", "caractere UTF-8 inválido.\n");
            return -1;
        }
    }
    
    return 0;
}
