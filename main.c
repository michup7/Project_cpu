#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <malloc.h>
#include "sts_queue/sts_queue.h"

//TODO reader, analyzer, printer should be in separate files

const bool DEBUG_ENABLED = true;
const int FREQUENCY = 1;
StsHeader *rawCpuInfoQueue;
StsHeader *transformedProcInfoQueue;

char *readCurrentProcStat() {
    char *buffer;
    //TODO length should not be fixed but calculated
    long length = 350;
    FILE *file = fopen("/proc/stat", "r");
    buffer = malloc(length);
    fread(buffer, 1, length, file);
    fclose(file);

    return buffer;
}

void *reader() {
    printf("Starting reader thread...\n");

    while (true) {
        printf("Reading values from proc\n");

        char *procStat = readCurrentProcStat();
        if (DEBUG_ENABLED) {
            printf("Raw cpu data: %.20s\n", procStat);
        }
        StsQueue.push(rawCpuInfoQueue, procStat);

        //TODO sleep should not be used
        sleep(FREQUENCY);
    }
}

void *analyzer() {
    printf("Starting analyzer thread...\n");

    char *cpuStat;

    while (true) {
        while ((cpuStat = StsQueue.pop(rawCpuInfoQueue)) != NULL) {
            if (DEBUG_ENABLED) {
                printf("Starting analysis on data: %.20s\n", cpuStat);
            }
            //TODO here we should analyse data and calculate usage
            StsQueue.push(transformedProcInfoQueue, "TODO: usage to be calculated");

            free(cpuStat);
        }
        //TODO sleep should not be used
        sleep(FREQUENCY);
    }
}

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
