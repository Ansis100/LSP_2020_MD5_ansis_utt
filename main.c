#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define DEBUG 0

// Ārēji mainīgie getopt izmantošanai
extern char * optarg;
extern int optind;

// Programmas izmantošanas instrukcija
const char usageFormat[] = "Usage: %s [-c chunks-file] [-s sizes-file]\n";

// Maksimālais alokācijas testējamās atmiņas izmērs
#define MAX_MEMORY_SIZE 1024
const int maxMemorySize = MAX_MEMORY_SIZE;

// ### Alokāciju algoritmi
// ## Best fit (Ansis)
void * mallocBestFitInit(int * chunks) {
    // Todo: Replace with actual memory initialisation
}

void * mallocBestFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ## Worst fit (Krišjānis)

// Dienasta informācija par pieejamo atmiņu
typedef struct WorstServiceInfoStruct {
    size_t availableMemory;
    size_t initialAvailableMemory;
    void * startOfMemory;
    void * initialStartOfMemory;
} WorstServiceInfo;

// Masīvs, kur glabāt dienasta informācijas ierakstus
// Neefektīvs, jo alocējam tikpat vietas dienasta informācijām, cik alocējam baitus rezervācijām
// Bet nav svarīgi, jo uzdevums ir testēt rezervēšanas efektivitāti nevis ātrdarbību
WorstServiceInfo worstFitServiceInfo[MAX_MEMORY_SIZE];

// Masīvs, kuru izmantot kā rezervējamo atmiņu
unsigned char worstFitBuffer[MAX_MEMORY_SIZE];

// Worst fit algoritma dienasta informācijas sagatavošana
void mallocWorstFitInit(int * chunks) {
    int chunksIterator = 0;
    int bufferIterator = 0;
    while (chunks[chunksIterator] != -1) {
        // Pierakstam pieejamos atmiņas chunk'us
        (worstFitServiceInfo[chunksIterator]).availableMemory = chunks[chunksIterator];
        (worstFitServiceInfo[chunksIterator]).initialAvailableMemory = chunks[chunksIterator];
        (worstFitServiceInfo[chunksIterator]).startOfMemory = &worstFitBuffer[bufferIterator];
        (worstFitServiceInfo[chunksIterator]).initialStartOfMemory = &worstFitBuffer[bufferIterator];
        bufferIterator += chunks[chunksIterator];
        chunksIterator++;
    }
}

// Worst fit algoritma alocēsanas funkcija
void * mallocWorstFit(size_t size) {
    // Meklējam lielāko pieejamo chunk'u
    WorstServiceInfo * largestMemoryInfo = NULL;
    int largestMemoryInfoIterator = 0;
    while (!(
        // Beidzam ciklēt, ja tikām līdz dienasta informācijas masīva galam
        largestMemoryInfoIterator > maxMemorySize ||
        // Beidzam ciklēt, ja izlasījām visas aktuālās dienasta informācijas
        (largestMemoryInfoIterator != 0 && (worstFitServiceInfo[largestMemoryInfoIterator]).initialAvailableMemory == 0)
    )) {
        if (!largestMemoryInfo) {
            // Ja šis ir pirmais chunk's, pieņemam to kā lielāko
            largestMemoryInfo = &worstFitServiceInfo[largestMemoryInfoIterator];
        } else {
            if ((worstFitServiceInfo[largestMemoryInfoIterator]).availableMemory > largestMemoryInfo->availableMemory) {
                // Ja šis ir n-tais chunk's un ir lielāks par iepriekšējo, tad saglabājam to kā lielāko
                largestMemoryInfo = &worstFitServiceInfo[largestMemoryInfoIterator];
            }
        }
        largestMemoryInfoIterator++;
    }

    // Ja neatradām brīvu chunk'u - beidzam
    if (!largestMemoryInfo) {
        return NULL;
    }

    // Ja lielākais chunk's tāpat nav pietiekami daudz - beidzam
    if (largestMemoryInfo->availableMemory < size) {
        return NULL;
    }

    // Saglabājam alocēto adresi, ko atdot lietotājam
    void * allocatedAddress = largestMemoryInfo->startOfMemory;

    // Atjauninam dienasta informāciju
    largestMemoryInfo->availableMemory -= size;
    largestMemoryInfo->startOfMemory += size;

    // Atgriežam lietotājam alocēto adresi 
    return allocatedAddress;
}

// Worst fit algoritma iekšējās atmiņas pilnīga izprintēšana
void mallocWorstFitDump() {
    int memoryInfoIterator = 0;
    int memoryByteIterator = 0;
    int breakLineAfter = 32;
    while (!(
        // Meklējam lielāko pieejamo chunk'u
        memoryInfoIterator > maxMemorySize ||
        // Beidzam ciklēt, ja izlasījām visas aktuālās dienasta informācijas
        (memoryInfoIterator != 0 && (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory == 0)
    )) {
        // Katram chunk'a baitam
        for (int i = 0; i < (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory; i++) {
            // Izprintējam skaitlisku reprezentāciju
            printf("%d", *((unsigned char *)(worstFitServiceInfo[memoryInfoIterator]).initialStartOfMemory + i));
            memoryByteIterator++;
            if (i + 1 == (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory) {
                printf(" # ");
            } else {
                printf("   ");
            }
            if (memoryByteIterator % breakLineAfter == 0) {
                // Ik pa laikam izprintējam jaunu līniju, lai vieglāk lasīt
                printf("\n");
            }
        }
        memoryInfoIterator++;
    }
    printf("\n");
}

// Worst fit algoritma brīvi pieejamo chunk'u izprintēšana atkļūdošanai
void mallocWorstFitFreeDump() {
    // Print all available memory chunks
    int memoryInfoIterator = 0;
    while (!(
        // Beidzam ciklēt, ja tikām līdz dienasta informācijas masīva galam
        memoryInfoIterator > maxMemorySize ||
        // Beidzam ciklēt, ja izlasījām visas aktuālās dienasta informācijas
        (memoryInfoIterator != 0 && (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory == 0)
    )) {
        if ((worstFitServiceInfo[memoryInfoIterator]).availableMemory != 0) {
            printf(
                "Free memory of %ld bytes at %p. Initially allocated as %ld bytes\n",
                (worstFitServiceInfo[memoryInfoIterator]).availableMemory,
                &(worstFitServiceInfo[memoryInfoIterator]),
                (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory
            );
        }
        memoryInfoIterator++;
    }
}

// Worst fit algoritma fragmentācijas aprēķins
double mallocWorstFitFragmentation() {
    int memoryInfoIterator = 0;
    // Atrodam cik kopā brīvu baitu un cik daudz baitu ir lielākajā brīvajā chunk'ā
    double freeBytes = 0;
    double largestChunkFreeBytes = 0;
    while (!(
        // Beidzam ciklēt, ja tikām līdz dienasta informācijas masīva galam
        memoryInfoIterator > maxMemorySize ||
        // Beidzam ciklēt, ja izlasījām visas aktuālās dienasta informācijas
        (memoryInfoIterator != 0 && (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory == 0)
    )) {
        freeBytes += (worstFitServiceInfo[memoryInfoIterator]).availableMemory;
        if ((worstFitServiceInfo[memoryInfoIterator]).availableMemory > largestChunkFreeBytes) {
            largestChunkFreeBytes = (worstFitServiceInfo[memoryInfoIterator]).availableMemory;
        }
        memoryInfoIterator++;
    }

    // Aprēķinam fragmentāciju, kas ir 100%, ja brīvu baitu nav
    double fragmentation = 100;
    if (freeBytes != 0) {
        // Ja ir brīvi baiti, tad aprēķinam attiecīgi brīvajai atmiņai un lielākajam chunk'am 
        fragmentation = ((freeBytes - largestChunkFreeBytes) / freeBytes) * 100.0;
    }

    return fragmentation;
}

// ## First fit (Ģirts)
void * mallocFirstFitInit(int * chunks) {
    // Todo: Replace with actual memory initialisation
}

void * mallocFirstFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ## Next fit (Andris)
void * mallocNextFitInit(int * chunks) {
    // Todo: Replace with actual memory initialisation
}

void * mallocNextFit(size_t size) {
    // Todo: Replace with actual algorithm
    return malloc(size);
}

// ### Galvenā programmas funkcionalitāte
int main(int argc, char *argv[])
{
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
    FILE * chunksFile;
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
    FILE * sizesFile;
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
        if (!fscanf(sizesFile, "%d", &sizes[sizesCreationIterator])) {
            fprintf(stderr, "Sizes file is incorrectly formatted\n");
            fprintf(stderr, "Sizes file should consist of lines of single numbers\n");
            return EXIT_FAILURE;
        }
        sizesCreationIterator++;
    }
    sizes[sizesCreationIterator] = -1;

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
        unsigned char * mem = mallocWorstFit(size);
        if (DEBUG) {
            if (mem) {
                for (int i = 0; i < size; i++) {
                    *(mem + i) = (unsigned char)1;
                }
            }
        }
        sizesTestingIterator++;
    }

    if (DEBUG) {
        mallocWorstFitDump();
        mallocWorstFitFreeDump();
    }

    printf("Worst fit fragmentation: %f\n", mallocWorstFitFragmentation());

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