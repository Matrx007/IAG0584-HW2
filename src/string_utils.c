#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

uint8_t contains(const char* haystack, const char* needle) {
    int str1_len = strlen(haystack)+1;
    int str2_len = strlen(needle)+1;

    char* str1_lowercase = malloc(str1_len * sizeof(char));
    char* str2_lowercase = malloc(str2_len * sizeof(char));
    
    char* str1_lowercase_pointer = str1_lowercase;
    char* str2_lowercase_pointer = str2_lowercase;
    
    const char* str1_pointer = haystack;
    const char* str2_pointer = needle;

    while(*str1_pointer != 0) {
        *str1_lowercase_pointer = tolower(*str1_pointer);
        str1_lowercase_pointer++;
        str1_pointer++;
    }
    *str1_lowercase_pointer = 0;

    while(*str2_pointer != 0) {
        *str2_lowercase_pointer = tolower(*str2_pointer);
        str2_lowercase_pointer++;
        str2_pointer++;
    }
    *str2_lowercase_pointer = 0;

    char* result = strstr(str1_lowercase, str2_lowercase);

    free(str1_lowercase);
    free(str2_lowercase);

    return result != 0;
}