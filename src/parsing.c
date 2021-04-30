#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "parsing.h"

/*  Skip leading whitespaces and new lines */
void trim(const char **c) {
    while(**c == ' ' || **c == '\n') nextChar(c);
}

/*  Skip leading whitespaces */
void space(const char **c) {
    while(**c == ' ') nextChar(c);
}

/*  Increment the pointer (char *c) */
void nextChar(const char **c) {
    (*c)++;
    if(**c == 0) {
        printf("Input end reached\n");
    }
}

/*  Increment the pointer (char *c) and return the new character */
char next(const char **c) {
    char next = **c;
    nextChar(c);
    return next;
}

/*  Increments pointer and returns true if next character equals T
    Skips leading whitespaces and new lines. */
u_int8_t eat(const char **c, char t) {
    trim(c);
    if(**c == t) {
        nextChar(c);
        return 1;
    }
    return 0;
}

/*  Trims the input and returns the following character. */
char get(const char **c) {
    trim(c);
    return **c;
}

/*  Trims the input and returns the current character and then steps forward. */
char read(const char **c) {
    trim(c);
    return *(*c++);
}


