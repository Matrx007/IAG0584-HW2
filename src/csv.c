#include <inttypes.h>
#include "parsing.c"

uint8_t __readDatabaseString(const char **c, char buffer[], char **o) {
    // If field doesn't start with double quotes
    if(**c != '"') {
        return 0;
    }

    nextChar(c);

    *o = buffer;
    while(*o-buffer < 255 && **c != 0 && **c != '"') {
        if(**c == '\\') {
            (*c)++;
            if(**c == '"') {
                **o = **c;
                (*o)++;
            } else {
                (*c)--;
                **o = **c;
            }
        } else {
            **o = **c;
            (*o)++;
        }
        (*c)++;
    }

    // Add null-termiantor
    **o = 0;

    // If no closing double quotes
    if(!eat(c, '"')) {
        return 0;
    }

    return 1;
}

uint8_t __readDatabaseInteger(const char **c, char buffer[], char **o) {

    // If field doesn't start with a number
    if(**c < '0' || **c > '9') {
        return 0;
    }

    *o = buffer;
    while(*o-buffer < 255 && **c != 0 && (**c >= '0' && **c <= '9')) {
        **o = **c;
        (*o)++;
        (*c)++;
    }

    // Add null-termiantor
    **o = 0;

    return 1;
}

uint8_t __readDatabaseFloat(const char **c, char buffer[], char **o) {

    // If field doesn't start with a number
    if(**c < '0' || **c > '9') {
        return 0;
    }

    *o = buffer;
    while(*o-buffer < 255 && **c != 0 && (**c >= '0' && **c <= '9')) {
        **o = **c;
        (*o)++;
        (*c)++;
    }

    // Read decimal places
    if(**c == '.') {
        **o = '.';
        (*o)++;
        (*c)++;
        while(*o-buffer < 255 && **c != 0 && (**c >= '0' && **c <= '9')) {
            **o = **c;
            (*o)++;
            (*c)++;
        }
    }

    // Add null-termiantor
    **o = 0;

    return 1;
}