#include <stdio.h>
#include <stdlib.h>

char* readEntireFile(const char* name) {

    FILE *file = fopen(name, "rb");

    // Find file size
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate big enough buffer (+ null-termiator)
    char *buffer = malloc(fsize + 1);
    fread(buffer, 1, fsize, file);
    buffer[fsize] = 0;

    fclose(file);

    return buffer;
}

