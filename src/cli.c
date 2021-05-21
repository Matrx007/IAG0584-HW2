#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "memory.h"
// #include "memory.c"
#include "database.c"

struct State {
    struct Table* powerPlants;
    struct Table* dailyStats; 

    struct Vector* selectedPowerPlants;
    struct Vector* selectedDailyStats;
};

const char* prompt(const char *question);
const char* promptFile(const char *question);

// Vector<char*>
struct Vector readWords();

void executeCommand(struct State *state);

void command_import(struct State *state, struct Vector *words);
void command_export(struct State *state, struct Vector *words);

void command_select(struct State *state, struct Vector *words);
void command_deselect(struct State *state, struct Vector *words);

void command_insert(struct State *state, struct Vector *words);
void command_delete(struct State *state, struct Vector *words);

void command_stats(struct State *state, struct Vector *words);
void command_plants(struct State *state, struct Vector *words);

void command_list(struct State *state, struct Vector *words);

void flushAndExit();



void executeCommand(struct State *state) {

    // ### READ USER INPUT ###

    printf(" > ");

    struct Vector words = readWords();

    if(words.size == 0) return;

    if(strcmp("exit", vectorGet(&words, 0))) {
        flushAndExit();
    } else if (strcmp("import", vectorGet(&words, 0))) {
        command_import(state, &words);
    } else if (strcmp("export", vectorGet(&words, 0))) {
        command_export(state, &words);
    } else if (strcmp("select", vectorGet(&words, 0))) {
        command_select(state, &words);
    } else if (strcmp("deselect", vectorGet(&words, 0))) {
        command_deselect(state, &words);
    } else if (strcmp("insert", vectorGet(&words, 0))) {
        command_insert(state, &words);
    } else if (strcmp("delete", vectorGet(&words, 0))) {
        command_delete(state, &words);
    } else if (strcmp("stats", vectorGet(&words, 0))) {
        command_stats(state, &words);
    } else if (strcmp("plants", vectorGet(&words, 0))) {
        command_plants(state, &words);
    } else if (strcmp("list", vectorGet(&words, 0))) {
        command_list(state, &words);
    } else {
        printf("Unknown command\n");
    }
}



void command_import(struct State *state, struct Vector *words) {
    if(words->size < 2) {
        printf("import syntax: <file name>\n");
        return;
    }

    uint8_t success = readPowerPlantDatabaseFile(state->powerPlants, vectorGet(words, 1));
    if(!success) {
        printf("failed to read database file\n");
    }
}

void command_export(struct State *state, struct Vector *words) {

}


void command_select(struct State *state, struct Vector *words) {
    if(words->size < 2) {
        printf("select syntax: power-plants / daily-stats [contains <string> / matches <pattern>]\n");
        return;
    }

    char* firstWord = vectorGet(words, 1);
    if(strcmp(firstWord, "power-plants")) {
        
    } else if(strcmp(firstWord, "daily-stats")) {
        
    }
}

void command_deselect(struct State *state, struct Vector *words) {
    if(words->size < 2) {
        printf("select syntax: \"power plants\"/\"daily stats\" [contains <string> / matches <pattern>]\n");
        return;
    }
}


void command_insert(struct State *state, struct Vector *words) {

}

void command_delete(struct State *state, struct Vector *words) {

}


void command_stats(struct State *state, struct Vector *words) {

}

void command_plants(struct State *state, struct Vector *words) {

}


void command_list(struct State *state, struct Vector *words) {

}


void flushAndExit(struct State *state) {
    tableForEach(state->dailyStats,  __freeReferences, 0);
    tableForEach(state->powerPlants, __freeReferences, 0);
    // TODO: pack vectors & tables (maybe problems if vector/table is empty)
}


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

struct Vector readWords() {

    struct Vector words = vectorCreate(sizeof(char *));

    char buffer[256];

    char c;

    do {
        char *o = buffer;

        int escaping = 0;
        int inString = 0;
        while(((c = fgetc(stdin)) != ' ' || inString) && o-buffer < 512 && c != 0 && c != '\n') {
            if(c =='"') {
                inString = !inString;
                continue;
            }

            if(inString) {
                if(escaping) {
                    switch (c) {
                        case 'n':
                            *o = '\n';
                            continue;
                            break;
                        case '"':
                            *o = '"';
                            continue;
                            break;
                        default:
                            break;
                    }

                    escaping = 0;
                }

                if(c == '\\') {
                    escaping = 1;
                    continue;
                }
            }

            *o = c;
            o++;
        }

        *o = 0;

        const char* string = copyStringToHeap(buffer);
        vectorAdd(&words, &string);
    } while(c != '\n');

    vectorPack(&words);

    return words;
}

void printWords(void* data, void* args) {
    printf("word: %s\n", *(char **) data);
}

int main(int argc, char **args) {

    struct Vector words = readWords();

    // void vectorForEach(struct Vector *vector, void (*action)(void* data, void* args), void* args)
    vectorForEach(&words, printWords, 0);





    if(1) return 0;

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