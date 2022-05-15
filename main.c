#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <malloc.h>
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
    //TODO length should not be fixed but calculated
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
    char * buffer = strtok_r(procStat, "\n", &tmp);  //buffer_origin to plik wejsciowy z danymi
    while(strcmp(cpu_name =strtok(buffer, " "),"intr")!=0)
        {
        int values[10];
        int i = 0;

        char * value = strtok(NULL, " ");

        while( value != NULL )
            {
                values[i] = (int)value;
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

        //TODO sleep should not be used
        sleep(FREQUENCY);
    }
}

void *parser(int cpu_tab[CORE][10]) {
    char *cpuStat;
    printf("++++++++++++++++++++PARSER++++++++++++++\n");
    while ((cpuStat = StsQueue.pop(rawCpuInfoQueue)) != NULL) {
        if (DEBUG_ENABLED) {
            printf("Starting analysis on data: %.20s\n", cpuStat);
        }
        char *tmp;
        char *cpu_name;
        char *buffer = strtok_r(cpuStat, "\n", &tmp);  //buffer_origin to plik wejsciowy z danymi

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

            //TODO here we should analyse data and calculate usage
            //free(cpuStat);
        }
    }
}

void *analyzer() {
    printf("Starting analyzer thread...\n");
    int cpu_value1[CORE][10];
    int cpu_value2[CORE][10];
    int i = 0;
    int j = 0;
    while (true) {
        i=0;
        j=0;
        parser(cpu_value1);
        sleep(FREQUENCY);
        parser(cpu_value2);


        /*
        for(i;i<9;i++)
        {
            for(j;j<10;j++)
            {
                printf("CPU%d:%d\n",i,cpu_value1[i][j]);
                printf("CPU%d:%d\n",i,cpu_value
            }

        }
         */
        //TODO here we should analyse data and calculate usage



    }

    StsQueue.push(transformedProcInfoQueue, "TODO: usage to be calculated");

}


/*
void *analyzer() {
    printf("Starting analyzer thread...\n");
    int *cpu_value1[CORE][10];
    int *cpu_value2[CORE][10];
    parser(cpu_value1);
    sleep(FREQUENCY);
    parser(cpu_value2);

    while (true) {
        while ((cpuStat = StsQueue.pop(rawCpuInfoQueue)) != NULL) {
            if (DEBUG_ENABLED) {
                printf("Starting analysis on data: %.20s\n", cpuStat);
            }
            char *tmp;
            char *cpu_name ;
            char * buffer = strtok_r(cpuStat, "\n", &tmp);  //buffer_origin to plik wejsciowy z danymi
            int i = 0;

            while(strcmp(cpu_name =strtok(buffer, " "),"intr")!=0)
            {
                printf( "Nazwa CPU: %s\n", cpu_name );

                long j = 0;

                char * value = strtok(NULL, " ");

                while( value != NULL ) {
                    values[i][j] = (int)value;
                    printf("i: %d, j: %d\t",i,j);
                    printf( "Value: %s\n", value ); //printing each token

                    value = strtok(NULL, " ");
                    j += 1;
                }
                i += 1;
                buffer = strtok_r(NULL, "\n", &tmp);

                //TODO here we should analyse data and calculate usage



            }



            StsQueue.push(transformedProcInfoQueue, "TODO: usage to be calculated");

            free(cpuStat);
        }
        //TODO sleep should not be used
        sleep(FREQUENCY);
    }
}
*/
void *printer() {
    printf("Starting printer thread...\n");

    char *procStat;

    while (true) {
        while ((procStat = StsQueue.pop(transformedProcInfoQueue)) != NULL) {
            printf("CPU usage %s\n", procStat);
        }

        //TODO sleep should not be used
        sleep(FREQUENCY);
    }
}

int main() {
    printf("Starting CPU analysis\n");
    CORE = init_tab();
    printf("Amount of cores: %i\n",CORE);

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
