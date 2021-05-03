#include <stdio.h>
#include <stdlib.h>

// #include "memory.h"
// #include "memory.c"
#include "database.c"


const char* prompt(const char *question);
const char* promptFile(const char *question);



const char* prompt(const char *question) {
    printf("%s: \n> ", question);

    char buffer[256];
    char *o = buffer;

    char c;
    while((c = fgetc(stdin)) != '\n' && o-buffer < 255 && c != 0) {
        *o = c;
        o++;
    }
    *o = 0;

    return copyStringToHeap(buffer);
}


const char* promptFile(const char *question) {
    const char* input = prompt(question);

    const char *c = input;
    while(*c != 0) {
        switch (*c) {
            case '\\':
            case '/':
            case ':':
            case '*':
            case '?':
            case '"':
            case '<':
            case '>':
            case '|':
                printf("Invalid file name, file name cannot contain ASCII's control characters nor one of those: \\/:*?\"<>|\n");
                free((char*)input);
                return promptFile(question);
            default:
                c++;
        }
    }

    return input;
}

int main(int argc, char **args) {
    struct Table table = tableCreate(sizeof(struct PowerPlantsRow));
    
    const char* fileName = promptFile("Enter power plant data file");
    if(fileName == 0) {
        printf("invalid file name\n");
        return 1;
    }


    char* file = readEntireFile(fileName);
    if(file == 0) {
        printf("failed to read file\n");
        return 1;
    }


    int r = loadPowerPlantDatabaseFile(&table, file);

    free((char*)fileName);
    free(file);

    printf("DATA: ==============\n");

    for (int i = 0; i < table.size; i++) {
        struct PowerPlantsRow *row = tableGet(&table, i);
        
        // break instead of continue because once there's a null 
        // pointer, everything after that will be too
        if(row == 0) continue;

        printf("power plant no %d:\n", i);

        printf("plant ID: %d\n", row->plantID);
        printf("plant name: %s\n", row->plantName);
        printf("plant type: %s\n", row->plantType);
        printf("max rated capacity: %f\n", row->maxRatedCapacity);
        printf("avg production cost: %f\n", row->avgProductionCost);
    }
}