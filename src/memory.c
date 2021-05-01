#include <memory.h>
#include <stdlib.h>

const char* copyBufferToHeap(char buffer[], size_t buffer_size) {
    char *c = malloc(sizeof(char) * buffer_size);

    memcpy(c, buffer, sizeof(char) * buffer_size);

    return (const char *) c;
}

const char* copyStringToHeap(char buffer[]) {
    unsigned long bytes = strlen(buffer) + 1;
    char *c = malloc(sizeof(char) * bytes);

    memcpy(c, buffer, sizeof(char) * bytes);

    return (const char *) c;
}