#include <features.h>
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

struct Vector createVector(int unitSize) {
    struct Vector vector;

    vector.__unitSize = unitSize;
    vector.__actualSize = 10;
    vector.size = 0;
    vector.__growthRatio = 1.5f;

    vector.__data = malloc(vector.__unitSize * vector.__actualSize);

    return vector;
}

struct Vector createVectorAdv(int unitSize, int initialSize, float growthRate) {
    struct Vector vector;

    vector.__unitSize = unitSize;
    vector.__actualSize = initialSize;
    vector.size = 0;
    vector.__growthRatio = growthRate;

    vector.__data = malloc(vector.__unitSize * vector.__actualSize);

    return vector;
}

void __growVector(struct Vector *vector) {
    printf("expanding array\n");
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

void pack(struct Vector *vector) {
    vector->__actualSize = vector->size;
    
    vector->__data = realloc(vector->__data, vector->__actualSize * vector->__unitSize);

    if(vector->__data == 0) {
        printf("Couldn't resize vector\n");
        abort();
    }
}

int addElement(struct Vector *vector, void *data) {
    if(vector->size >= vector->__actualSize) {
        __growVector(vector);
    }

    int index = vector->size;
    void *dest = vector->__data + index * vector->__unitSize;

    memcpy(dest, data, vector->__unitSize);

    vector->size++;

    return index;
}

void removeElement(struct Vector *vector, int index) {
    if(index < 0 || index >= vector->size) {
        printf("Array index out of bounds\n");
        abort();
    }

    void *loc = vector->__data + index * vector->__unitSize;

    memcpy(loc, loc + vector->__unitSize, (vector->size - index - 1) * vector->__unitSize);

    vector->size--;
}

void changeElement(struct Vector *vector, int index, void *data) {
    if(index < 0 || index >= vector->size) {
        printf("Array index out of bounds\n");
        abort();
    }

    void *dest = vector->__data + index * vector->__unitSize;

    memcpy(dest, data, vector->__unitSize);
}

void* getElement(struct Vector *vector, int index) {
    if(index < 0 || index >= vector->size) {
        printf("Array index out of bounds\n");
        abort();
    }

    return vector->__data + index * vector->__unitSize;
}

int __main(int argc, char **args) {
    struct Vector vector = createVector(sizeof(int));

    int tmp;
    tmp = 5;
    addElement(&vector, &tmp);
    tmp = 8;
    addElement(&vector, &tmp);
    tmp = 7;
    addElement(&vector, &tmp);
    tmp = 3;
    addElement(&vector, &tmp);

    printf("%d\n", vector.size);

    removeElement(&vector, 0);
    tmp = 13;
    addElement(&vector, &tmp);

    printf("%d\n", vector.size);
    printf("%d\n", *(int*)getElement(&vector, 2));

    tmp = 10;
    changeElement(&vector, 2, &tmp);

    printf("%d\n", vector.size);
    printf("%s\n", (const char*) getElement(&vector, 2));

    printf("actual size: %d\n", vector.__actualSize);

    tmp = 69;
    addElement(&vector, &tmp);
    tmp = 42;
    addElement(&vector, &tmp);

    removeElement(&vector, vector.size-1);
    removeElement(&vector, vector.size-1);
    
    printf("actual size before: %d\n", vector.__actualSize);

    pack(&vector);

    printf("actual size after: %d\n", vector.__actualSize);
}