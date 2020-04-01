#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEBUG 0

// Ārēji mainīgie getopt izmantošanai
extern char *optarg;
extern int optind;

// Programmas izmantošanas instrukcija
const char usageFormat[] = "Usage: %s [-c chunks-file] [-s sizes-file]\n";

// Maksimālais alokācijas testējamās atmiņas izmērs
#define MAX_MEMORY_SIZE 1024
const int maxMemorySize = MAX_MEMORY_SIZE;

// ### Alokāciju algoritmi
// ## Worst fit (Ansis)

// Bloku sākumu norādes. beidzas ar norādi, kur sākas neatbrīvota atmiņa
void *chunk_start_ptrs[MAX_MEMORY_SIZE];
// Bloku sākumu norādes pēc sizes ievietošanas
void *chunk_actual_ptrs[MAX_MEMORY_SIZE];
// Neizdalītais baitu daudzums
int failed_bytes = 0;

void *mallocBestFitInit(int *chunks) {
    int i = 0;
    chunk_start_ptrs[0] = malloc(MAX_MEMORY_SIZE);
    chunk_actual_ptrs[0] = chunk_start_ptrs[0];

    while (chunks[i] != 0) {
        chunk_start_ptrs[i + 1] = chunk_start_ptrs[i] + chunks[i];
        chunk_actual_ptrs[i + 1] = chunk_start_ptrs[i + 1];
        i++;
    }
}

void mallocBestFit(size_t size) {
    int i = 1, delta = INT_MAX, best_ptr_index = -1;
    void *tmp_ptr = chunk_start_ptrs[i];
    while (tmp_ptr != 0x0) {
        int space = tmp_ptr - chunk_actual_ptrs[i - 1]; 
        if (size <= space && delta > (space - size)) { 
            delta = space - size; 
            best_ptr_index = i - 1; 
        }
        tmp_ptr = chunk_start_ptrs[++i];
    }

    if (best_ptr_index != -1) {
        chunk_actual_ptrs[best_ptr_index] += size; 
    } else {
        failed_bytes += size; 
    }
}

void bestFitFragmentation() {
    int i = 1, largest_free_space_block = INT_MIN, free_space = 0;
    void *tmp_ptr = chunk_start_ptrs[i];
    while (tmp_ptr != 0x0) {
        int space = tmp_ptr - chunk_actual_ptrs[i - 1];
        free_space += space;
        if (largest_free_space_block < space) {
            largest_free_space_block = space;
        }
        tmp_ptr = chunk_start_ptrs[++i];
    }

    printf("Neizdevās izdalīt - %dB\n", failed_bytes);
    printf("Lielākais brīvais bloks - %dB\n", largest_free_space_block);
    printf("Brīva vieta - %dB\n", free_space);
    printf("Fragmentācija - %.2f\%\n", ((double)(free_space - largest_free_space_block) / (double)free_space) * 100);
}

// ## Worst fit (Krišjānis)

// Dienasta informācija par pieejamo atmiņu
typedef struct WorstServiceInfoStruct {
    size_t availableMemory;
    size_t initialAvailableMemory;
    void *startOfMemory;
    void *initialStartOfMemory;
} WorstServiceInfo;

// Masīvs, kur glabāt dienasta informācijas ierakstus
// Neefektīvs, jo alocējam tikpat vietas dienasta informācijām, cik alocējam
// baitus rezervācijām Bet nav svarīgi, jo uzdevums ir testēt rezervēšanas
// efektivitāti nevis ātrdarbību
WorstServiceInfo worstFitServiceInfo[MAX_MEMORY_SIZE];

// Masīvs, kuru izmantot kā rezervējamo atmiņu
unsigned char worstFitBuffer[MAX_MEMORY_SIZE];

// Worst fit algoritma dienasta informācijas sagatavošana
void mallocWorstFitInit(int *chunks) {
    int chunksIterator = 0;
    int bufferIterator = 0;
    while (chunks[chunksIterator] != -1) {
        // Pierakstam pieejamos atmiņas chunk'us
        (worstFitServiceInfo[chunksIterator]).availableMemory =
            chunks[chunksIterator];
        (worstFitServiceInfo[chunksIterator]).initialAvailableMemory =
            chunks[chunksIterator];
        (worstFitServiceInfo[chunksIterator]).startOfMemory =
            &worstFitBuffer[bufferIterator];
        (worstFitServiceInfo[chunksIterator]).initialStartOfMemory =
            &worstFitBuffer[bufferIterator];
        bufferIterator += chunks[chunksIterator];
        chunksIterator++;
    }
}

// Worst fit algoritma alocēsanas funkcija
void *mallocWorstFit(size_t size) {
    // Meklējam lielāko pieejamo chunk'u
    WorstServiceInfo *largestMemoryInfo = NULL;
    int largestMemoryInfoIterator = 0;
    while (!(
        // Beidzam ciklēt, ja tikām līdz dienasta informācijas masīva galam
        largestMemoryInfoIterator > maxMemorySize ||
        // Beidzam ciklēt, ja izlasījām visas aktuālās dienasta informācijas
        (largestMemoryInfoIterator != 0 &&
         (worstFitServiceInfo[largestMemoryInfoIterator])
                 .initialAvailableMemory == 0))) {
        if (!largestMemoryInfo) {
            // Ja šis ir pirmais chunk's, pieņemam to kā lielāko
            largestMemoryInfo = &worstFitServiceInfo[largestMemoryInfoIterator];
        } else {
            if ((worstFitServiceInfo[largestMemoryInfoIterator])
                    .availableMemory > largestMemoryInfo->availableMemory) {
                // Ja šis ir n-tais chunk's un ir lielāks par iepriekšējo, tad
                // saglabājam to kā lielāko
                largestMemoryInfo =
                    &worstFitServiceInfo[largestMemoryInfoIterator];
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
    void *allocatedAddress = largestMemoryInfo->startOfMemory;

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
        (memoryInfoIterator != 0 &&
         (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory ==
             0))) {
        // Katram chunk'a baitam
        for (int i = 0;
             i <
             (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory;
             i++) {
            // Izprintējam skaitlisku reprezentāciju
            printf("%d",
                   *((unsigned char *)(worstFitServiceInfo[memoryInfoIterator])
                         .initialStartOfMemory +
                     i));
            memoryByteIterator++;
            if (i + 1 == (worstFitServiceInfo[memoryInfoIterator])
                             .initialAvailableMemory) {
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
        (memoryInfoIterator != 0 &&
         (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory ==
             0))) {
        if ((worstFitServiceInfo[memoryInfoIterator]).availableMemory != 0) {
            printf(
                "Free memory of %ld bytes at %p. Initially allocated as %ld "
                "bytes\n",
                (worstFitServiceInfo[memoryInfoIterator]).availableMemory,
                &(worstFitServiceInfo[memoryInfoIterator]),
                (worstFitServiceInfo[memoryInfoIterator])
                    .initialAvailableMemory);
        }
        memoryInfoIterator++;
    }
}

// Worst fit algoritma fragmentācijas aprēķins
double mallocWorstFitFragmentation() {
    int memoryInfoIterator = 0;
    // Atrodam cik kopā brīvu baitu un cik daudz baitu ir lielākajā brīvajā
    // chunk'ā
    double freeBytes = 0;
    double largestChunkFreeBytes = 0;
    while (!(
        // Beidzam ciklēt, ja tikām līdz dienasta informācijas masīva galam
        memoryInfoIterator > maxMemorySize ||
        // Beidzam ciklēt, ja izlasījām visas aktuālās dienasta informācijas
        (memoryInfoIterator != 0 &&
         (worstFitServiceInfo[memoryInfoIterator]).initialAvailableMemory ==
             0))) {
        freeBytes += (worstFitServiceInfo[memoryInfoIterator]).availableMemory;
        if ((worstFitServiceInfo[memoryInfoIterator]).availableMemory >
            largestChunkFreeBytes) {
            largestChunkFreeBytes =
                (worstFitServiceInfo[memoryInfoIterator]).availableMemory;
        }
        memoryInfoIterator++;
    }

    // Aprēķinam fragmentāciju, kas ir 100%, ja brīvu baitu nav
    double fragmentation = 100;
    if (freeBytes != 0) {
        // Ja ir brīvi baiti, tad aprēķinam attiecīgi brīvajai atmiņai un
        // lielākajam chunk'am
        fragmentation =
            ((freeBytes - largestChunkFreeBytes) / freeBytes) * 100.0;
    }

    return fragmentation;
}

// ## First fit (Ģirts)
void *mallocFirstFitInit(int *chunks) {
       int i = 0;
    chunk_start_ptrs[0] = malloc(MAX_MEMORY_SIZE);
    chunk_actual_ptrs[0] = chunk_start_ptrs[0];

    while (chunks[i] != 0) {
        chunk_start_ptrs[i + 1] = chunk_start_ptrs[i] + chunks[i];
        chunk_actual_ptrs[i + 1] = chunk_start_ptrs[i + 1];
        i++;
    }
    // Todo: Replace with actual memory initialisation
}

void *mallocFirstFit(size_t size) {

    int i = 1, first_ptr_index = -1;
    void *tmp_ptr = chunk_start_ptrs[i];
    while (tmp_ptr != 0x0) {
        int space = tmp_ptr - chunk_actual_ptrs[i - 1]; 
        if (size <= space) {
            first_ptr_index = i - 1;
        }
        tmp_ptr = chunk_start_ptrs[++i]; 
    }

    if (first_ptr_index != -1) { 
        chunk_actual_ptrs[first_ptr_index] += size; 
    } else {
        failed_bytes += size; 
    }

    return first_ptr_index;
}

void FirstFitFragmentation() {
    int i = 1, largest_free_space_block = INT_MIN, free_space = 0;
    void *tmp_ptr = chunk_start_ptrs[i];
    while (tmp_ptr != 0x0) {
        int space = tmp_ptr - chunk_actual_ptrs[i - 1];
        free_space += space;
        if (largest_free_space_block < space) {
            largest_free_space_block = space;
        }
        tmp_ptr = chunk_start_ptrs[++i];
    }
    printf("First fit fragmentation %.2f\%\n", ((double)(free_space - largest_free_space_block) / (double)free_space) * 100);

}
// ## Next fit (Andris)
void *mallocNextFitInit(int *chunks, int *chunks_metadata, int chunks_size) 
{
    // printf("Chunks size: %d\n", chunks_size);
    for (int i = 0; i < chunks_size; i++) {
        // printf("Metadata nr: %d, elem: %d\n", i, chunks[i]);
        chunks_metadata[i] = chunks[i];
    }

    // The last index of array of metadata of chunks stores the "current
    // pointer" of the traversing next fit algorithm.
    // Intilialy starts from 0 (the first element).
    chunks_metadata[chunks_size] = 0;
}

void mallocNextFit(
    int size, int *chunks_metadata, int chunks_size
) {
    // Current index stores the current pointer of next_fit.
    int current_index = chunks_metadata[chunks_size];
    // To detect if we have traversed the whole pool of memory (all 
    // the chunks).
    int loop_nr = 0;

    while (loop_nr < chunks_size) {
        // printf("Index: %d\n", current_index);
        if (chunks_metadata[current_index] > size) {
            // printf("1. Chunk before: %d\n", chunks_metadata[current_index]);
            chunks_metadata[current_index] -= size;
            chunks_metadata[chunks_size] = current_index;
            // printf("1. Chunk after: %d\n", chunks_metadata[current_index]);

            return;
        } else if (chunks_metadata[current_index] == size) {
            // printf("2. Chunk before: %d\n", chunks_metadata[current_index]);
            chunks_metadata[current_index] -= size;
            // Traverse the current pointer one index up and save it already in 
            // the array of metadata.
            chunks_metadata[chunks_size] += 1;
            // printf("2. Chunk after: %d\n", chunks_metadata[current_index]);

            return;
        } else {
            loop_nr++;
            current_index += 1;

            // So that the current index does not get bigger than the size
            // of the chunk array.
            if (current_index == chunks_size) {
                current_index = 0;
            }
        }
    }

    // This line is reached if there is no chunk big enough to fit the given
    // size.
    printf("Could not allocate memory of size %d\n", size);
    // Restore the pointer after unsuccessful loop to the current index.
    chunks_metadata[chunks_size] = current_index;
}

void mallocNextFitDump(int *chunks, int *chunks_metadata, int chunk_size) 
{
    // The difference between the initial chunk block size and chunk block size
    // after allocating memory.
    int diff;
    for (int i = 0; i < chunk_size; i++) {
        diff = chunks[i] - chunks_metadata[i];
        printf(
            "Diff: %d, ch: %d, meta_ch: %d\n", 
            diff, chunks[i], chunks_metadata[i]
        );
    }
}

double mallocNextFitFragmentation(int *chunks_metadata, int chunk_size) 
{
    // Store the sum of free bytes of all the chunks.
    double free_bytes = 0;
    // Store the largest chunk of free bytes.
    double largest_free_chunk = 0;
    // Store the result of calculation of fragmentation.
    // Initially, we suppose that fragmentation is 100 percent.
    double result = 100;

    for (int i = 0; i < chunk_size; i++) {
        free_bytes += chunks_metadata[i];
        if (chunks_metadata[i] > largest_free_chunk) {
            largest_free_chunk = chunks_metadata[i];
        }
    }

    // printf("Free bytes total: %f\n", free_bytes);
    // printf("Largest free chunk: %f\n", largest_free_chunk);

    if (free_bytes != 0) {
        // Return the calculation of fragmentation.
        result = ((free_bytes - largest_free_chunk) / free_bytes) * 100.0;
    }
    
    return result;
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
            fprintf(stderr,
                    "Chunks file should consist of lines of single numbers\n");
            return EXIT_FAILURE;
        } 
        chunkCreationIterator++;
    }
    // chunks[chunkCreationIterator] = -1;

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
        if (!fscanf(sizesFile, "%d", &sizes[sizesCreationIterator])) {
            fprintf(stderr, "Sizes file is incorrectly formatted\n");
            fprintf(stderr,
                    "Sizes file should consist of lines of single numbers\n");
            return EXIT_FAILURE;
        }
        sizesCreationIterator++;
    }
    // sizes[sizesCreationIterator] = -1;

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
    // The array where to store the remainder of free memory and the current
    // pointer of next fit algorithm.
    // Additionally, the purpose of this array is to not corrupt data in the
    // initially made array of chunks.
    // INFO: Because of the chunkCreationIterator value being "the biggest 
    // index of chunks array PLUS TWO gives me one additional space to store
    // the "current pointer" and one addional for some undefined use at
    // the moment.
    int chunks_metadata[chunkCreationIterator];
    mallocNextFitInit(chunks, chunks_metadata, chunkCreationIterator);

    // ### Testējam alokācijas algoritmus
    int sizesTestingIterator;

    printf("Testing best fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizes[sizesTestingIterator] != 0) {
        int size = sizes[sizesTestingIterator];
        mallocBestFit(size);
        sizesTestingIterator++;
    }
    bestFitFragmentation();

    printf("Testing worst fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizes[sizesTestingIterator] != -1) {
        int size = sizes[sizesTestingIterator];
        unsigned char *mem = mallocWorstFit(size);
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
    FirstFitFragmentation();

    printf("Testing next fit\n");
    fflush(stdout);
    sizesTestingIterator = 0;
    while (sizesTestingIterator < sizesCreationIterator) {
        int size = sizes[sizesTestingIterator];
        // printf("Passing argument of size: %d\n", size);
        mallocNextFit(size, chunks_metadata, chunkCreationIterator);
        sizesTestingIterator++;
    }

    if (DEBUG) {
        mallocNextFitDump(chunks, chunks_metadata, chunkCreationIterator);
    }

    // Calculating the fragmentation of memory using Next Fit.
    double result = mallocNextFitFragmentation(
        chunks_metadata, chunkCreationIterator
    );
    printf("Next fit fragmentation: %f\n", result);

    return EXIT_SUCCESS;
}