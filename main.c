#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Ārēji mainīgie getopt izmantošanai
extern char *optarg;
extern int optind;

// Programmas izmantošanas instrukcija
const char usageFormat[] = "Usage: %s [-c chunks-file] [-s sizes-file]\n";

// Maksimālais alokācijas testējamās atmiņas izmērs
const int maxMemorySize = 1024;

// ### Alokāciju algoritmi
void *mallocBestFitInit(int *chunks) {
    // int i = 0;
    // while (chunks[i] != -1) {
    //     printf("%d\n", chunks[i++]);
    // }
}

void *mallocBestFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ## Worst fit (Krišjānis)
void *mallocWorstFitInit(int *chunks) {
    // Todo: Replace with actual memory initialisation
}

void *mallocWorstFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ## First fit (Ģirts)
void *mallocFirstFitInit(int *chunks) {
    // Todo: Replace with actual memory initialisation
}

void *mallocFirstFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ## Next fit (Andris)
void *mallocNextFitInit(int *chunks) {
    // Todo: Replace with actual memory initialisation
}

void *mallocNextFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ### Galvenā programmas funkcionalitāte
int main(int argc, char *argv[]) {
    // ### Noparsējam programmai padotos parametrus
    char chunksPath[PATH_MAX];
    char sizesPath[PATH_MAX];
    int option;
    while ((option = getopt(argc, argv, "c:s:")) != -1) {
        switch (option) {
            case 'c':
                strcpy(chunksPath, optarg);
                break;
            case 's':
                strcpy(sizesPath, optarg);
                break;
            default:
                fprintf(stderr, "Incorrect option provided\n");
                fprintf(stderr, usageFormat, argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Pārbaudam, ka chunks fails tika padots
    if (strcmp(chunksPath, "") == 0) {
        fprintf(stderr, "Chunks file not provided\n");
        fprintf(stderr, usageFormat, argv[0]);
        return EXIT_FAILURE;
    }

    // Pārbaudam, ka sizes fails tika padots
    if (strcmp(sizesPath, "") == 0) {
        fprintf(stderr, "Sizes file not provided\n");
        fprintf(stderr, usageFormat, argv[0]);
        return EXIT_FAILURE;
    }

    // ### Noparsējam testējamos chunks, sizes failus
    // Atveram chunks failu
    FILE *chunksFile;
    chunksFile = fopen(chunksPath, "r");
    if (chunksFile == NULL) {
        fprintf(stderr, "Chunks file '%s' couldn\'t be read\n", chunksPath);
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        fprintf(stderr, usageFormat, argv[0]);
        return EXIT_FAILURE;
    }

    // Noparsējam chunks failu
    int chunks[maxMemorySize];
    int chunkCreationIterator = 0;
    while (!feof(chunksFile)) {
        if (!fscanf(chunksFile, "%d", &chunks[chunkCreationIterator])) {
            fprintf(stderr, "Chunks file is incorrectly formatted\n");
            fprintf(stderr, "Chunks file should consist of lines of single numbers\n");
            return EXIT_FAILURE;
        }
        chunkCreationIterator++;
    }
    chunks[chunkCreationIterator] = -1;

    // Atveram sizes failu
    FILE *sizesFile;
    sizesFile = fopen(sizesPath, "r");
    if (sizesFile == NULL) {
        fprintf(stderr, "Sizes file '%s' couldn\'t be read\n", sizesPath);
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        fprintf(stderr, usageFormat, argv[0]);
        return EXIT_FAILURE;
    }

    // Noparsējam sizes failu
    int sizes[maxMemorySize];
    int sizesCreationIterator = 0;
    while (!feof(sizesFile)) {
        if (!fscanf(sizesFile, "%1d", &sizes[sizesCreationIterator])) {
            fprintf(stderr, "Sizes file is incorrectly formatted\n");
            fprintf(stderr, "Sizes file should consist of lines of single numbers\n");
            return EXIT_FAILURE;
        }
        sizesCreationIterator++;
    }

    // ### Inicializējam alokācijas algoritmus
    // Šeit tiek sagatavota nepieciešamā atmiņa algoritmiem
    printf("Initialising best fit\n");
    fflush(stdout);
    mallocBestFitInit(chunks);

    printf("Initialising worst fit\n");
    fflush(stdout);
    mallocWorstFitInit(chunks);

    printf("Initialising first fit\n");
    fflush(stdout);
    mallocFirstFitInit(chunks);

    printf("Initialising next fit\n");
    fflush(stdout);
    mallocNextFitInit(chunks);

    // ### Testējam alokācijas algoritmus
    int sizesTestingIterator;

    printf("Testing best fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizes[sizesTestingIterator] != -1) {
        int size = sizes[sizesTestingIterator];
        mallocBestFit(size);
        sizesTestingIterator++;
    }

    printf("Testing worst fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizes[sizesTestingIterator] != -1) {
        int size = sizes[sizesTestingIterator];
        mallocWorstFit(size);
        sizesTestingIterator++;
    }

    printf("Testing first fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizes[sizesTestingIterator] != -1) {
        int size = sizes[sizesTestingIterator];
        mallocFirstFit(size);
        sizesTestingIterator++;
    }

    printf("Testing next fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizes[sizesTestingIterator] != -1) {
        int size = sizes[sizesTestingIterator];
        mallocNextFit(size);
        sizesTestingIterator++;
    }

    return EXIT_SUCCESS;
}