#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "sts_queue/sts_queue.h"
#include <string.h>
#include <stdlib.h>

//TODO reader, analyzer, printer should be in separate files
int CORE = 0;
const bool DEBUG_ENABLED = false;
const int FREQUENCY = 1;
StsHeader *rawCpuInfoQueue;
StsHeader *transformedProcInfoQueue;

char *readCurrentProcStat() {
    char *buffer;
    long length = 1024;
    FILE *file = fopen("/proc/stat", "r");
    buffer = malloc(length);
    fread(buffer, sizeof(char), length, file);
    fclose(file);

    return buffer;
}

int *init_tab(){
    char *procStat = readCurrentProcStat();
    printf("INIT TAB...\n");
    char *tmp;
    char *cpu_name ;
    int j = 0;
    char * buffer = strtok_r(procStat, "\n", &tmp);  //buffer_origin input file
    while(strcmp(cpu_name =strtok(buffer, " "),"intr")!=0)
        {
        int values[10];
        int i = 0;

        char * value = strtok(NULL, " ");

        while( value != NULL )
            {
                values[i] = atoi(value);
                value = strtok(NULL, " ");
                i += 1;
            }
        j += 1;
        buffer = strtok_r(NULL, "\n", &tmp);

        }
    return j;
    }

void *reader() {
    printf("Starting reader thread...\n");

    while (true) {
        printf("Reading values from proc\n");

        char *procStat = readCurrentProcStat();
        if (DEBUG_ENABLED) {
            printf("Raw cpu data: %s\n", procStat);
        }
        StsQueue.push(rawCpuInfoQueue, procStat);
        sleep(FREQUENCY);
    }
}

void *parser(int cpu_tab[CORE][10]) {
    char *cpuStat;

    while ((cpuStat = StsQueue.pop(rawCpuInfoQueue)) != NULL) {
        if (DEBUG_ENABLED) {
            printf("Starting analysis on data: %.20s\n", cpuStat);
        }
        char *tmp;
        char *cpu_name;
        char *buffer = strtok_r(cpuStat, "\n", &tmp);  //buffer_origin input file

        int i = 0;
        while (strcmp(cpu_name = strtok(buffer, " "), "intr") != 0) {
            if (DEBUG_ENABLED) {
                printf("Nazwa CPU: %s\n", cpu_name);
            }
            int j = 0;

            char *value = strtok(NULL, " ");

            while (value != NULL) {
                cpu_tab[i][j] = atoi(value);
                if (DEBUG_ENABLED) {
                    printf("i: %d, j: %d\t", i, j);
                    printf("Value: %d\n", atoi(value)); //printing each token
                }
                value = strtok(NULL, " ");
                j += 1;
            }
            i += 1;
            buffer = strtok_r(NULL, "\n", &tmp);

            //free(cpuStat);
        }
    }
    //free(cpuStat);
}

void *analyzer() {
    printf("Starting analyzer thread...\n");
    int cpu_value1[CORE][10];
    int cpu_value2[CORE][10];
    int i;

    while (true) {
        parser(cpu_value1);
        sleep(FREQUENCY);
        parser(cpu_value2);

        for(i=0;i<CORE;i++)
        {
            unsigned long long now_workingtime = 0;
            unsigned long long prev_workingtime = 0;
            unsigned long long workingtime = 0;
            unsigned long long now_idletime = 0;
            unsigned long long prev_idletime = 0;
            unsigned long long all_time = 0;
            long double result[CORE] ;
            char buffer[32];
            now_workingtime = cpu_value2[i][0] + cpu_value2[i][1] + cpu_value2[i][2] + cpu_value2[i][5] + cpu_value2[i][6];
            prev_workingtime = cpu_value1[i][0] + cpu_value1[i][1] + cpu_value1[i][2] + cpu_value1[i][5] + cpu_value1[i][6];
            workingtime =  now_workingtime - prev_workingtime;
            now_idletime = cpu_value2[i][3] +  cpu_value2[i][4];
            prev_idletime = cpu_value1[i][3] +  cpu_value2[i][4];
            all_time = (now_workingtime - prev_workingtime) + (now_idletime - prev_idletime);
            result[i] = (long double)workingtime / all_time * 100.0L;
            //printf("CPU%d Usage: %.02Lf%%\n",i,(long double)workingtime / all_time * 100.0L);
            snprintf(buffer, sizeof(buffer), "CPU%d %.02Lf", i,result[i]);
            StsQueue.push(transformedProcInfoQueue, buffer);
            sleep(0.1);
        }

    }
}


void *printer() {
    printf("Starting printer thread...\n");

    char *procStat;

    while (true)
    {
        while ((procStat = StsQueue.pop(transformedProcInfoQueue)) != NULL)
        {
            //printf("CPU usage %s %\n",procStat);
            printf("Usage %s %\n",procStat);
        }
        //sleep(FREQUENCY);
    }
}

int main() {
    printf("Starting CPU analysis\n");
    CORE = init_tab();
    printf("Amount of cores: %i\n",CORE-2);

    rawCpuInfoQueue = StsQueue.create();
    transformedProcInfoQueue = StsQueue.create();
    pthread_t readerThread;
    pthread_create(&readerThread, NULL, reader, NULL);

    pthread_t analyzerThread;
    pthread_create(&analyzerThread, NULL, analyzer, NULL);

    pthread_t printerThread;
    pthread_create(&printerThread, NULL, printer, NULL);

    pthread_join(readerThread, NULL);
    StsQueue.destroy(rawCpuInfoQueue);
    StsQueue.destroy(transformedProcInfoQueue);
    return 0;
}
