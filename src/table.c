#include <features.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct Table {
    int __actualSize;
    int size;
    int __unitSize;
    float __overheadRatio;
    void *__data;
};

struct Table tableCreate(int unitSize) {

    struct Table table;

    table.__unitSize = unitSize;
    table.__actualSize = 0;
    table.size = 0;
    table.__overheadRatio = 1.5f;

    table.__data = malloc(table.__unitSize * table.__actualSize);

    return table;
}

struct Table tableCreateAdv(int unitSize, int initialSize, float growthRate) {

    struct Table table;

    table.__unitSize = unitSize;
    table.__actualSize = initialSize;
    table.size = 0;
    table.__overheadRatio = growthRate;

    table.__data = malloc(table.__unitSize * table.__actualSize);

    return table;
}

void __tableGrow(struct Table *table, int target) {
    target++;
    
    if(target < table->__actualSize) {
        printf("internal error: attempting to shrink table\n");
        abort();
    }

    // Store old values
    int oldEnd = table->size * table->__unitSize;
    int oldSize = table->__actualSize;

    // Update information
    table->__actualSize = target * table->__overheadRatio;
    
    // Reallocate new memory space
    table->__data = realloc(table->__data, table->__actualSize * table->__unitSize);

    if(table->__data == 0) {
        printf("Couldn't resize table\n");
        abort();
    }

    // Initialize the expanded area
    memset(table->__data + oldEnd, 0, (table->__actualSize - oldSize) * table->__unitSize);
}

void tablePack(struct Table *table) {

    table->__actualSize = table->size;
    
    table->__data = realloc(table->__data, table->__actualSize * table->__unitSize);

    if(table->__data == 0) {
        printf("Couldn't resize table\n");
        abort();
    }
}

int tableInsert(struct Table *table, int index, void *data) {
    
    if(index < 0) {
        printf("Table index out of bounds\n");
        abort();
    }

    if(index >= table->size) {
        __tableGrow(table, index);
    }

    void *dest = table->__data + index * table->__unitSize;

    memcpy(dest, data, table->__unitSize);

    if(++index > table->size)
        table->size = index;

    return index;
}

void tableRemove(struct Table *table, int index) {

    if(index < 0 || index >= table->size) {
        printf("Table index out of bounds\n");
        abort();
    }

    void *loc = table->__data + index * table->__unitSize;

    memset(loc, 0, table->__unitSize);
}

void tableChange(struct Table *table, int index, void *data) {
    if(index < 0 || index >= table->size) {
        printf("Table index out of bounds\n");
        abort();
    }

    void *dest = table->__data + index * table->__unitSize;

    memcpy(dest, data, table->__unitSize);
}

void* tableGet(struct Table *table, int index) {
    if(index < 0 || index >= table->size) {
        printf("Table index out of bounds\n");
        abort();
    }

    return table->__data + index * table->__unitSize;
}

// matcher(..) has to return either 1 or 0
uint8_t tableHasMatch(struct Table *table, uint8_t (*matcher)(void* data, void* args), void* args) {

    int result;
    for(int i = 0; i < table->size; i++) {
        result = matcher(tableGet(table, i), args);
        if(result) return 1;
    }

    return 0;
}

// matcher(..) has to return either 1 or 0
int tableCountMatches(struct Table *table, uint8_t (*matcher)(void* data, void* args), void* args) {

    int matches = 0;
    for(int i = 0; i < table->size; i++) {
        matches += matcher(tableGet(table, i), args);
    }

    return matches;
}

// matcher(..) has to return 0 if that element doesn't exist / == NULL
// Iterates from back to front and shrinks the table until the first found non-uninitialized element
void tableShrink(struct Table *table, uint8_t (*tester)(void* data)) {
    if(table->size == 0) return;

    int last;
    for(last = table->size-1; last >= 0; last--) {
        if(tester(tableGet(table, last))) {
            break;
        }
    }
    last++;

    table->__actualSize = last;
    table->size = last;

    table->__data = realloc(table->__data, last * table->__unitSize);
}

void tableForEach(struct Table *table, void (*action)(void* data, void* args), void* args) {

    for(int i = 0; i < table->size; i++) {
        action(tableGet(table, i), args);
    }
}

void actionForEachElement(void* data, void* args) {
    int num = *(int*) data;
    if(num != 0) {
        printf("%d\n", num);
    }
}

uint8_t isNumber5(void* data, void* args) {
    int num = *(int*) data;
    return num == 5;
}

uint8_t numberTester(void* data) {
    int num = *(int*) data;
    return num != 0;
}

int table_test(int argc, char **args) {
    struct Table table = tableCreate(sizeof(int));

    int tmp;

    tmp = 5;
    printf("inserting '5' at 0\n");
    tableInsert(&table, 0, &tmp);

    printf("1: size: %d\n", table.size);
    printf("1: actual size: %d\n", table.__actualSize);

    tmp = 5;
    printf("inserting '5' at 10\n");
    tableInsert(&table, 10, &tmp);

    printf("2: size: %d\n", table.size);
    printf("2: actual size: %d\n", table.__actualSize);

    tmp = 0;
    printf("inserting '0' at 100\n");
    tableInsert(&table, 100, &tmp);

    printf("3: size: %d\n", table.size);
    printf("3: actual size: %d\n", table.__actualSize);

    printf("printing all non-zero elements: \n");

    tableForEach(&table, actionForEachElement, 0);
    
    printf("number of 5-s: %d\n", tableCountMatches(&table, isNumber5, 0));

    printf("packing...\n");
    tablePack(&table);
    printf("actual size after packing: %d\n", table.__actualSize);

    printf("shrinking...\n");
    tableShrink(&table, numberTester);
    printf("actual size after shrinking: %d\n", table.__actualSize);

    printf("reading element at 0: %d\n", *(int*) tableGet(&table, 0));
    printf("reading element at 10: %d\n", *(int*) tableGet(&table, 0));
    printf("reading element at 11 (not set, should be 0): %d\n", *(int*) tableGet(&table, 11));
    printf("reading element at 101 (should abort): \n");
    printf("%d\n", *(int*) tableGet(&table, 101));

    return 0;
}