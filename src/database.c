#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <memory.h>

#include "parsing.h"
#include "memory.h"
#include "memory.c"
#include "vector.c"
#include "table.c"
#include "fs.c"
#include "csv.c"

uint8_t __readDatabaseString(const char **c, char buffer[], char **o);
uint8_t __readDatabaseInteger(const char **c, char buffer[], char **o);
uint8_t __readDatabaseFloat(const char **c, char buffer[], char **o);
struct PowerPlantsRow* __loadPowerPlantDatabaseFileLine(const char **c);
uint8_t __powerPlantIDMatcher(void* data, void* args);
uint8_t loadPowerPlantDatabaseFile(struct Table *powerPlants, const char *raw);

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

void __freePowerPlantReference(void* data, void* args) {
    printf("free-ing references of %p\n", data);
    struct PowerPlantsRow* row = data;

    free((void*)row->plantName);
    free((void*)row->plantType);
}

void __freeDailyStatsReference(void* data, void* args) {
    struct DailyStatisticsRow* row = data;
}

// ################################
// ### POWER PLANT DATA PARSING ###
// ################################


struct PowerPlantsRow* __loadPowerPlantDatabaseFileLine(const char **c) {
    struct PowerPlantsRow *row = malloc(sizeof(struct PowerPlantsRow));

    trim(c);

    char buffer[256];
    // aka output: points to current character in buffer
    char *o;

    // ### Plant ID ###
    space(c);

    if(!__readDatabaseInteger(c, buffer, &o)) return 0;

    row->plantID = atoi(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Plant name ###
    space(c);
    
    if(!__readDatabaseString(c, buffer, &o)) return 0;

    row->plantName = copyStringToHeap(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Plant type ###
    space(c);
    
    if(!__readDatabaseString(c, buffer, &o)) return 0;

    row->plantType = copyStringToHeap(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Maximum rated capacity ###
    space(c);

    if(!__readDatabaseFloat(c, buffer, &o)) return 0;

    row->maxRatedCapacity = atof(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }
    // ### Avg production cost ###
    space(c);

    if(!__readDatabaseFloat(c, buffer, &o)) return 0;

    row->avgProductionCost = atof(buffer);

    return row;    
}

uint8_t __powerPlantIDMatcher(void* data, void* args) {
    return ((struct PowerPlantsRow*) data)->plantID == *(u_int32_t*)args;
}

uint8_t loadPowerPlantDatabaseFile(struct Table *powerPlants, const char *raw) {
    const char *c = raw;

    int line = 0;
    do {
        line++;
        trim(&c);
        
        // If end-of-file
        if(*c == 0) break;

        // Skip comment lines
        if(*c == '#') {
            while(*c != '\n') c++;
            continue;
        }

        struct PowerPlantsRow* row = __loadPowerPlantDatabaseFileLine(&c);

        // If pointer is invalid, then that file probably had a syntax error
        // Skip this line
        if(row == 0) {
            printf("problem on line %d\n", line);

            // Skip this line
            while(*c != '\n') c++;
            continue;
        }

        // PK isn't allowed to be zero
        if(row->plantID == 0) {
            printf("power plant's PK can't be 0, skipping line %d\n", line);

            // Skip this line
            while(*c != '\n') c++;
            continue;
        }

        // Duplicate PK-s aren't allowed
        if(tableHasMatch(powerPlants, __powerPlantIDMatcher, (void*)&row->plantID)) {
            printf("duplicate PK %d on line %d\n", row->plantID, line);

            // Skip this line
            while(*c != '\n') c++;
            continue;
        }

        // Store the parsed power plant information in given table
        tableInsert(powerPlants, row->plantID, row);

        free(row);

        space(&c);
    } while(*c == '\n');

    return 1;
}

uint8_t readPowerPlantDatabaseFile(struct Table *powerPlants, char *file) {
    char* data = readEntireFile(file);
    if(data == 0) {
        return 1;
    }
    int r = loadPowerPlantDatabaseFile(powerPlants, data);
    free(data);
    return r;
}


// ##########################
// ### DAILY LOGS PARSING ###
// ##########################


struct DailyStatisticsRow* __loadDailyLogDatabaseFileLine(const char **c) {
    struct DailyStatisticsRow *log = malloc(sizeof(struct DailyStatisticsRow));

    trim(c);

    char buffer[256];
    // aka output: points to current character in buffer
    char *o;

    // ### Log ID ###
    space(c);

    if(!__readDatabaseInteger(c, buffer, &o)) return 0;

    log->reportID = atoi(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Plant ID ###
    space(c);

    if(!__readDatabaseInteger(c, buffer, &o)) return 0;

    log->plantID = atoi(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Daily production ###
    space(c);

    if(!__readDatabaseFloat(c, buffer, &o)) return 0;

    log->dailyProduction = atof(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Avg. sales price ###
    space(c);

    if(!__readDatabaseFloat(c, buffer, &o)) return 0;

    log->avgSalesPrice = atof(buffer);

    // If no comma after field
    if(!eat(c, ',')) {
        return 0;
    }

    // ### Date time epoch ###
    space(c);

    if(!__readDatabaseInteger(c, buffer, &o)) return 0;

    log->dateEpoch = atoi(buffer);

    return log;    
}

uint8_t __dailyLogIDMatcher(void* data, void* args) {
    return ((struct DailyStatisticsRow*) data)->reportID == *(u_int32_t*)args;
}

uint8_t loadDailyLogDatabaseFile(struct Table *dailyLogs, const char *raw) {
    const char *c = raw;

    int line = 0;
    do {
        line++;
        trim(&c);
        
        // If end-of-file
        if(*c == 0) break;

        // Skip comment lines
        if(*c == '#') {
            while(*c != '\n') c++;
            continue;
        }

        struct DailyStatisticsRow* log = __loadDailyLogDatabaseFileLine(&c);

        // If pointer is invalid, then that file probably had a syntax error
        // Skip this line
        if(log == 0) {
            printf("problem on line %d\n", line);

            // Skip this line
            while(*c != '\n') c++;
            continue;
        }

        // PK isn't allowed to be zero
        if(log->reportID == 0) {
            printf("report's PK can't be 0, skipping line %d\n", line);

            // Skip this line
            while(*c != '\n') c++;
            continue;
        }

        // Duplicate PK-s aren't allowed
        if(tableHasMatch(dailyLogs, __dailyLogIDMatcher, (void*)&log->reportID)) {
            printf("duplicate PK %d on line %d\n", log->reportID, line);

            // Skip this line
            while(*c != '\n') c++;
            continue;
        }

        // Store the parsed power plant information in given table
        tableInsert(dailyLogs, log->reportID, log);

        free(log);

        space(&c);
    } while(*c == '\n');

    return 1;
}

uint8_t readDailyLogDatabaseFile(struct Table *dailyLogs, char *file) {
    char* data = readEntireFile(file);
    if(data == 0) {
        return 1;
    }
    int r = loadDailyLogDatabaseFile(dailyLogs, data);
    free(data);
    return r;
}

int database_test(int argc, const char *args[]) {
    struct Table table = tableCreate(sizeof(struct PowerPlantsRow));

    char* file = readEntireFile(args[1]);
    if(file == 0) {
        return 1;
    }
    int r = loadPowerPlantDatabaseFile(&table, file);
    free(file);

    printf("================\n");
    printf("return code: %d\n", r);


    for (int i = 0; i < table.size; i++) {
        struct PowerPlantsRow *row = (struct PowerPlantsRow*)tableGet(&table, i);
        
        if(row->plantID == 0) {
            printf("\nno power plant #%d\n", i);
            continue;
        }

        printf("\npower plant #%d:\n", i);

        printf("plant ID: %d\n", row->plantID);
        printf("plant name: %s\n", row->plantName);
        printf("plant type: %s\n", row->plantType);
        printf("max rated capacity: %f\n", row->maxRatedCapacity);
        printf("avg production cost: %f\n", row->avgProductionCost);
    }

    return 0;
}