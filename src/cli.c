#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <sys/types.h>

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

enum INSTRUCTION { SELECT, DESELECT };
enum FIELD {    PLANT_PK, PLANT_NAME, PLANT_TYPE, PLANT_CAPACITY, PLANT_PRODUCTION_COST, 
                LOG_PK, LOG_FK, LOG_PRODUCTION, LOG_SELL_PRICE, LOG_DATE };
enum DATA_TYPE { STRING, INT, FLOAT };
struct __MATCHER_DATA {
    enum INSTRUCTION instruction; 
    enum FIELD field; 
    char* matchString; 
    void (*action)(void*, void*);
    struct Vector *dest;
};

void __selectionMatcherAction_select(void* data, void* args);
void __selectionMatcherAction_deselect(void* data, void* args);
void __selectionMatcher_contains(void* data, void* args);
void __selectionMatcher_equals(void* data, void* args);
void __selectionMatcher_passthrough(void* data, void* args);

void flushAndExit();


// Used for debugging
void printWords(void* data, void* args) {
    printf("word1: %s\n", *(char **) data);
}

void printVector(struct Vector *vector) {
    vectorForEach(vector, printWords, 0);
}

void printTable(struct Table *table) {
    tableForEach(table, printWords, 0);
}


void executeCommand(struct State *state) {

    // ### READ USER INPUT ###

    printf(" > ");

    struct Vector words = readWords();

    if(words.size == 0) return;

    char** firstWord = vectorGet(&words, 0);
    if(!strcmp("EXIT", *firstWord)) {
        flushAndExit();
    } else if (!strcmp("IMPORT", *firstWord)) {
        command_import(state, &words);
    } else if (!strcmp("EXPORT", *firstWord)) {
        command_export(state, &words);
    } else if (!strcmp("SELECT", *firstWord)) {
        command_select(state, &words);
    } else if (!strcmp("DESELECT", *firstWord)) {
        command_deselect(state, &words);
    } else if (!strcmp("INSERT", *firstWord)) {
        command_insert(state, &words);
    } else if (!strcmp("DELETE", *firstWord)) {
        command_delete(state, &words);
    } else if (!strcmp("STATS", *firstWord)) {
        command_stats(state, &words);
    } else if (!strcmp("PLANTS", *firstWord)) {
        command_plants(state, &words);
    } else if (!strcmp("LIST", *firstWord)) {
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

const char* syntax_select = "syntax: SELECT (power-plants / logs) WHERE <column> (CONTAINS <string> / EQUALS <value>)";
void command_select(struct State *state, struct Vector *words) {
    if(words->size < 2) {
        printf("%s\n", syntax_select);
        return;
    }



    // "power-plants" or "logs"
    char* sourceName = *(char**)vectorGet(words, 1);
    struct Table* source;
    struct Vector* dest;

    if(!strcmp(sourceName, "power-plants")) {
        source = state->powerPlants;
        dest = state->selectedPowerPlants;
    } else if(!strcmp(sourceName, "logs")) {
        source = state->dailyStats;
        dest = state->selectedDailyStats;
    } else {
        printf("err: unknown table \"%s\"\n", sourceName);
        return;
    }

    char* matcherName;
    char* matcherString;
    void (*matcher)(void*, void*);
    struct __MATCHER_DATA data;
    enum FIELD field;

    // if "CONTAINS" or "EQUALS" not present
    if(words->size == 2) {

        matcher = __selectionMatcher_passthrough;
        
        data = (struct __MATCHER_DATA){ SELECT, field, 0, __selectionMatcherAction_select, dest };

        goto findMatches;
    } else if(words->size < 6) {
        printf("%s\n", syntax_select);
        return;
    }

    if(strcmp(*(char**)vectorGet(words, 2), "WHERE")) {
        printf("%s\n", syntax_select);
        return;
    }

    char* columnName = *(char**)vectorGet(words, 3);

    if(source == state->powerPlants) {
        if(!strcmp(columnName, "id")) field = PLANT_PK; else
        if(!strcmp(columnName, "name")) field = PLANT_NAME; else
        if(!strcmp(columnName, "type")) field = PLANT_TYPE; else
        if(!strcmp(columnName, "capacity")) field = PLANT_CAPACITY; else
        if(!strcmp(columnName, "cost")) field = PLANT_PRODUCTION_COST;
    } else {
        if(!strcmp(columnName, "id")) field = LOG_PK; else
        if(!strcmp(columnName, "plant")) field = LOG_FK; else
        if(!strcmp(columnName, "production")) field = LOG_PRODUCTION; else
        if(!strcmp(columnName, "sales")) field = LOG_SELL_PRICE; else
        if(!strcmp(columnName, "date")) field = LOG_DATE;
    }

    // if "CONTAINS" or "EQUALS" present
    matcherName = *(char**)vectorGet(words, 4);
    matcherString = *(char**)vectorGet(words, 5);

    if(!strcmp(matcherName, "CONTAINS")) {
        matcher = __selectionMatcher_contains;
    } else if(!strcmp(matcherName, "EQUALS")) {
        matcher = __selectionMatcher_equals;
    } else {
        printf("%s\n", syntax_select);
        return;
    }

    findMatches:

    data = (struct __MATCHER_DATA){ SELECT, field, matcherString, __selectionMatcherAction_select, dest };

    tableForEach(source, matcher, &data);

    printf("\n%d power plants and %d daily reports in selection.\n\n", 
            state->selectedPowerPlants->size, state->selectedDailyStats->size);
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

void printPowerPlant(void* data, void* args) {
    struct PowerPlantsRow *row = data;

    printf("Power plant ID: %d\n", row->plantID);
    printf("Power plant name: %s\n", row->plantName);
    printf("Power plant fuel: %s\n", row->plantType);
    printf("Max. rated capacity: %f\n", row->maxRatedCapacity);
    printf("Avg. production cost: %f\n", row->avgProductionCost);
    printf("\n");
}

void printDailyStats(void* data, void* args) {
    struct DailyStatisticsRow *row = data;

    printf("Report ID: %d\n", row->reportID);
    printf("Power plant ID: %d\n", row->plantID);
    printf("Daily production: %f\n", row->dailyProduction);
    printf("Avg. sales price: %f\n", row->avgSalesPrice);
    printf("Report Date: %d\n", row->dateEpoch);
}

void command_list(struct State *state, struct Vector *words) {
    printf("\n%d power plants and %d daily reports in selection.\n\n", 
            state->selectedPowerPlants->size, state->selectedDailyStats->size);

    printf("Plant ID, plant name, plant type, rated capacity, production cost\n\n");

    vectorForEach(state->selectedPowerPlants, printPowerPlant, 0);
}

void __powerPlantSelected(void* data, void* args) {
    struct {
        uint8_t* found;
        uint32_t id;
    }* info = args;

    struct PowerPlantsRow* row = data;

    if(row->plantID == info->id) {
        *info->found = 1;
        return;
    }
}

void __selectionMatcherAction_select(void* data, void* args) {
    struct __MATCHER_DATA* mdata = args;

    // check if power plant isn't already selected
    uint8_t selected = 0;
    struct {
        uint8_t* found;
        uint32_t id;
    } info;

    info.found = &selected;
    info.id = *(uint32_t*)data;
    
    vectorForEach(mdata->dest, __powerPlantSelected, &info);

    // don't select if already selected
    if(!selected) vectorAdd(mdata->dest, data);
}

uint8_t __deselector(void* data, void* args) {
    struct PowerPlantsRow* plant = data;

    // works on both, power plants and logs, as the PK is the first elemnt in struct
    return plant->plantID == 0;
}

void __selectionMatcherAction_deselect(void* data, void* args) {
    struct __MATCHER_DATA* mdata = args;
    vectorRemoveIf(mdata->dest, __deselector, args);
}

void __selectionMatcher_contains(void* data, void* args) {
    struct __MATCHER_DATA* mdata = args;
    struct PowerPlantsRow* plant = data;
    struct DailyStatisticsRow* log = data;
    int callAction = 0;

    // works on both, power plants and logs, as the PK is the first elemnt in struct
    if(plant->plantID == 0) return; 

    switch (mdata->field) {
        case PLANT_PK:
        case PLANT_CAPACITY:
        case PLANT_PRODUCTION_COST:
        case LOG_PK:
        case LOG_FK:
        case LOG_PRODUCTION:
        case LOG_SELL_PRICE:
            printf("err: 'CONTAINS' cannot be used on this column\n");
            return;
        
        case PLANT_NAME:
            callAction = strstr(plant->plantName, mdata->matchString) != 0;
            break;
        case PLANT_TYPE:
            callAction = strstr(plant->plantType, mdata->matchString) != 0;
            break;
        
        default:
            printf("fatal err: unknown column\n");
            exit(EXIT_FAILURE);
            return;
    }

    if(callAction) mdata->action(data, args);
}

void __selectionMatcher_equals(void* data, void* args) {
    struct __MATCHER_DATA* mdata = args;
    struct PowerPlantsRow* plant = data;
    struct DailyStatisticsRow* log = data;
    int callAction = 0;

    // works on both, power plants and logs, as the PK is the first element in both structs
    if(plant->plantID == 0) return; 

    enum DATA_TYPE type = STRING;
    enum DATA_TYPE requiredType = STRING;
    float valFloat;
    uint64_t valInt;
    char* valString;


    // ### READ DATA TYPE ###
    {
        valString = mdata->matchString;

        errno = 0;
        char* endptr = valString;
        valFloat = strtof(valString, &endptr);
        if(errno == 0) {
            type = FLOAT;
        }

        errno = 0;
        endptr = valString;
        valInt = strtoul(valString, &endptr, 10);
        if(errno == 0) {
            type = INT;
        }
    }

    switch (mdata->field) {

        case PLANT_PK:              requiredType = INT;     break;
        case PLANT_NAME:            requiredType = STRING;  break;
        case PLANT_TYPE:            requiredType = STRING;  break;
        case PLANT_CAPACITY:        requiredType = FLOAT;   break;
        case PLANT_PRODUCTION_COST: requiredType = FLOAT;   break;
        case LOG_PK:                requiredType = INT;     break;
        case LOG_FK:                requiredType = INT;     break;
        case LOG_PRODUCTION:        requiredType = FLOAT;   break;
        case LOG_SELL_PRICE:        requiredType = FLOAT;   break;
        case LOG_DATE:              requiredType = INT;     break;
        
        default:
            printf("fatal err: unknown column\n");
            exit(EXIT_FAILURE);
            return;
    }

    switch (mdata->field) {

        case PLANT_PK:
            if(type == INT) {
                if(valInt == plant->plantID) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case PLANT_NAME:
            if(type == STRING) {
                if(!strcmp(valString, plant->plantName)) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case PLANT_TYPE:
            if(type == STRING) {
                if(!strcmp(valString, plant->plantType)) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case PLANT_CAPACITY:
            if(type == FLOAT) {
                if(valFloat == plant->maxRatedCapacity) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case PLANT_PRODUCTION_COST:
            if(type == FLOAT) {
                if(valFloat == plant->avgProductionCost) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case LOG_PK:
            if(type == INT) {
                if(valInt == log->reportID) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case LOG_FK:
            if(type == INT) {
                if(valInt == log->plantID) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case LOG_PRODUCTION:
            if(type == FLOAT) {
                if(valFloat == log->dailyProduction) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case LOG_SELL_PRICE:
            if(type == FLOAT) {
                if(valFloat == log->avgSalesPrice) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        case LOG_DATE:
            if(type == INT) {
                if(valInt == log->dateEpoch) {
                    mdata->action(data, args);
                }
            } else printf("err: incompatible data types\n");
            break;
        
        default:
            printf("fatal err: unknown column\n");
            exit(EXIT_FAILURE);
            return;
    }
}

void __selectionMatcher_passthrough(void* data, void* args) {
    struct __MATCHER_DATA* mdata = args;
    struct PowerPlantsRow* plant = data;
    if(plant->plantID == 0) return;

    mdata->action(data, args);
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

int main(int argc, char **args) {

    struct Table plants = tableCreate(sizeof(struct PowerPlantsRow));
    struct Table logs = tableCreate(sizeof(struct DailyStatisticsRow));
    struct Vector selectedPlants = vectorCreate(sizeof(struct PowerPlantsRow));
    struct Vector selectedLogs = vectorCreate(sizeof(struct DailyStatisticsRow));
    
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


    int r = loadPowerPlantDatabaseFile(&plants, file);

    free((char*)fileName);
    free(file);

    printf("DATA: ==============\n");

    for (int i = 0; i < plants.size; i++) {
        struct PowerPlantsRow *row = tableGet(&plants, i);
        
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

    struct State state;
    state.powerPlants = &plants;
    state.dailyStats = &logs;
    state.selectedPowerPlants = &selectedPlants;
    state.selectedDailyStats = &selectedLogs;
    
    while(1) {
        executeCommand(&state);
    }
}