#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <memory.h>

#include "parsing.h"
#include "parsing.c"
#include "fs.c"

struct PowerPlantsRow {
    uint32_t plantID;
    const char *plantName;
    const char *plantType;
    float maxRatedCapacity;
    float avgProductionCost;
};

struct DailyStatisticsRow {
    uint32_t reportID;
    uint32_t plantID;
    float dailyProduction;
    float avgSalesPrice;
    uint32_t dateEpoch;
};

// These will be populated while parsing database files
struct PowerPlantsRow       **powerPlants;
struct DailyStatisticsRow   **dailyReports;

// These will be incremented while parsing database files
int n_powerPlants = 0;
int n_dailyStatistics = 0;

/* 

    ### File parsing ###





        
        

 */

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
// TODO: Store until null terminator
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

struct PowerPlantsRow* __loadPowerPlantDatabaseFileLine(const char **c) {
    struct PowerPlantsRow *row = malloc(sizeof(struct PowerPlantsRow));

    trim(c);

    char buffer[256];
    // aka output: points to current character in buffer
    char *o;

    // ### Plant ID ###

    if(!__readDatabaseInteger(c, buffer, &o)) return 0;

    row->plantID = atoi(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Plant name ###
    
    if(!__readDatabaseString(c, buffer, &o)) return 0;

    row->plantName = copyStringToHeap(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Plant type ###
    
    if(!__readDatabaseString(c, buffer, &o)) return 0;

    row->plantType = copyStringToHeap(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Maximum rated capacity ###

    if(!__readDatabaseFloat(c, buffer, &o)) return 0;

    row->maxRatedCapacity = atof(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }
    // ### Avg production cost ###

    if(!__readDatabaseFloat(c, buffer, &o)) return 0;

    row->avgProductionCost = atof(buffer);

    return row;    
}

uint8_t loadPowerPlantDatabaseFile(const char *raw) {
    const char *c = raw;
    
    trim(&c);

    char buffer[256];
    // aka output: points to current character in buffer
    char *o;
    if(!__readDatabaseInteger(&c, buffer, &o)) return 0;
    n_powerPlants = atoi(buffer);

    powerPlants = calloc(n_powerPlants, sizeof(struct PowerPlantsRow*));
    if(powerPlants == 0) {
        return 0;
    }

    int i = 0;
    
    do {
        space(&c);
        
        // If end-of-file
        if(*c == 0) break;

        struct PowerPlantsRow* row = __loadPowerPlantDatabaseFileLine(&c);
        if(row == 0) return 0;

        powerPlants[i++] = row;

        space(&c);
    } while(*c == '\n');

    return 1;
}

int main(int argc, const char *args[]) {
    int r = loadPowerPlantDatabaseFile(readEntireFile(args[1]));
    printf("return code: %d\n", r);

    for (int i = 0; i < n_powerPlants; i++) {
        struct PowerPlantsRow *row = powerPlants[i];
        
        // break isntead of continue because once there's a null 
        // pointer, everything after that will be too
        if(row == 0) break;

        printf("power plant no %d:\n", i);

        printf("plant ID: %d\n", row->plantID);
        printf("plant name: %s\n", row->plantName);
        printf("plant type: %s\n", row->plantType);
        printf("max rated capacity: %f\n", row->maxRatedCapacity);
        printf("avg production cost: %f\n", row->avgProductionCost);
    }
}