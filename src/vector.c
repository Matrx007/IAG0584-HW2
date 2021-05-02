#include <features.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Vector {
    int __actualSize;
    int size;
    int __unitSize;
    float __growthRatio;
    void *__data;
};

struct Vector vectorCreate(int unitSize) {
    struct Vector vector;

    vector.__unitSize = unitSize;
    vector.__actualSize = 10;
    vector.size = 0;
    vector.__growthRatio = 1.5f;

    vector.__data = malloc(vector.__unitSize * vector.__actualSize);

    return vector;
}

struct Vector vectorCreateAdv(int unitSize, int initialSize, float growthRate) {
    struct Vector vector;

    vector.__unitSize = unitSize;
    vector.__actualSize = initialSize;
    vector.size = 0;
    vector.__growthRatio = growthRate;

    vector.__data = malloc(vector.__unitSize * vector.__actualSize);

    return vector;
}

void __vectorGrow(struct Vector *vector) {
    if(vector->__actualSize == 0) {
        printf("Array size is zero\n");
        abort();
    }

    vector->__actualSize *= vector->__growthRatio;
    
    vector->__data = realloc(vector->__data, vector->__actualSize * vector->__unitSize);

    if(vector->__data == 0) {
        printf("Couldn't resize vector\n");
        abort();
    }
}

void vectorPack(struct Vector *vector) {
    vector->__actualSize = vector->size;
    
    vector->__data = realloc(vector->__data, vector->__actualSize * vector->__unitSize);

    if(vector->__data == 0) {
        printf("Couldn't resize vector\n");
        abort();
    }
}

int vectorAdd(struct Vector *vector, void *data) {
    if(vector->size >= vector->__actualSize) {
        __vectorGrow(vector);
    }

    int index = vector->size;
    void *dest = vector->__data + index * vector->__unitSize;

    memcpy(dest, data, vector->__unitSize);

    vector->size++;

    return index;
}

void vectorRemove(struct Vector *vector, int index) {
    if(index < 0 || index >= vector->size) {
        printf("Array index out of bounds\n");
        abort();
    }

    void *loc = vector->__data + index * vector->__unitSize;

    memcpy(loc, loc + vector->__unitSize, (vector->size - index - 1) * vector->__unitSize);

    vector->size--;
}

void vectorChange(struct Vector *vector, int index, void *data) {
    if(index < 0 || index >= vector->size) {
        printf("Array index out of bounds\n");
        abort();
    }

    void *dest = vector->__data + index * vector->__unitSize;

    memcpy(dest, data, vector->__unitSize);
}

void* vectorGet(struct Vector *vector, int index) {
    if(index < 0 || index >= vector->size) {
        printf("Array index out of bounds\n");
        abort();
    }

    return vector->__data + index * vector->__unitSize;
}

// matcher(..) has to return either 1 or 0
uint8_t vectorHasMatch(struct Vector *vector, uint8_t (*matcher)(void* data, void* args), void* args) {

    int result;
    for(int i = 0; i < vector->size; i++) {
        result = matcher(vectorGet(vector, i), args);
        if(result) return 1;
    }

    return 0;
}

// matcher(..) has to return either 1 or 0
int vectorCountMatches(struct Vector *vector, uint8_t (*matcher)(void* data, void* args), void* args) {

    int matches = 0;
    for(int i = 0; i < vector->size; i++) {
        matches += matcher(vectorGet(vector, i), args);
    }

    return matches;
}

void vectorForEach(struct Vector *vector, void (*action)(void* data, void* args), void* args) {

    for(int i = 0; i < vector->size; i++) {
        action(vectorGet(vector, i), args);
    }
}

int vector_test(int argc, char **args) {
    struct Vector vector = vectorCreate(sizeof(int));

    int tmp;
    tmp = 5;
    vectorAdd(&vector, &tmp);
    tmp = 8;
    vectorAdd(&vector, &tmp);
    tmp = 7;
    vectorAdd(&vector, &tmp);
    tmp = 3;
    vectorAdd(&vector, &tmp);

    printf("%d\n", vector.size);

    vectorRemove(&vector, 0);
    tmp = 13;
    vectorAdd(&vector, &tmp);

    printf("%d\n", vector.size);
    printf("%d\n", *(int*)vectorGet(&vector, 2));

    tmp = 10;
    vectorChange(&vector, 2, &tmp);

    printf("%d\n", vector.size);

    printf("actual size: %d\n", vector.__actualSize);

    tmp = 69;
    vectorAdd(&vector, &tmp);
    tmp = 42;
    vectorAdd(&vector, &tmp);

    vectorRemove(&vector, vector.size-1);
    vectorRemove(&vector, vector.size-1);
    
    printf("actual size before: %d\n", vector.__actualSize);

    vectorPack(&vector);

    printf("actual size after: %d\n", vector.__actualSize);

    return 0;
}