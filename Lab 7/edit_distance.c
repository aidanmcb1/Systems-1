#include "edit_distance.h"
#include <immintrin.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


//choose minimum of 3 int
int minimum3(const int a, const int b, const int c) {
    int m = a;
    if (b < m) {
        m = b;
    }
    if (c < m) {
        m = c;
    }
    return m;
}

int minimum2(const int a, const int b) {
    if (a < b) {
        return a;
    }
    return b;
}

//choose minimum of 3 m256i
__m256i avxMin(const __m256i a, const __m256i b, const __m256i c) {
    return _mm256_min_epi32(_mm256_min_epi32(a, b), c);
}

//does the big math, run on the threads
void avxMath(const char *string1Point, const char *string2Point, size_t tile_size, const size_t h, const size_t w, const int topLeft, int *rowEdgePoint, int *columnEdgePoint, int *cornerOutPoint) {

    //just a vector of only 1
    const __m256i v_ones = _mm256_set1_epi32(1);

    //avx buffers for diags
    __attribute__((aligned(32))) int secondPreviousBuffer[tile_size + 8];
    __attribute__((aligned(32))) int previousBuffer[tile_size + 8];
    __attribute__((aligned(32))) int currentBuffer[tile_size + 8];

    //buffer pointers
    int *secondPreviousBufferPoint = &secondPreviousBuffer[1];
    int *previousBufferPoint = &previousBuffer[1];
    int *currentBufferPoint = &currentBuffer[1];

    const size_t maxWave = h + w - 2;
    for (size_t wave = 0; wave <= maxWave; wave++) {

        if (wave < w) {
            previousBufferPoint[-1] = rowEdgePoint[wave];

            if (wave == 0) {
                secondPreviousBufferPoint[-1] = topLeft;
            } else {
                secondPreviousBufferPoint[-1] = rowEdgePoint[wave - 1];
            }
        }

        if (wave < h) {
            previousBufferPoint[wave] = columnEdgePoint[wave];
        }

        const size_t iMin = (wave < w) ? 0 : (wave - w + 1);
        const size_t iMax = (wave < h) ? wave : (h - 1);

        for (size_t k = iMin; k + 7 <= iMax; k += 8) {
            const __m256i vectorInsert = _mm256_loadu_si256((__m256i*)&previousBufferPoint[k]);
            const __m256i vectorDelete = _mm256_loadu_si256((__m256i*)&previousBufferPoint[k - 1]);
            const __m256i vectorSub = _mm256_loadu_si256((__m256i*)&secondPreviousBufferPoint[k - 1]);

            //grab the initial string chunks for 1 and 2
            long long string1Chunk;
            memcpy(&string1Chunk, &string1Point[k], sizeof(long long));

            const __m128i String1Vec = _mm_cvtsi64_si128(string1Chunk);
            const __m256i Char1Vec = _mm256_cvtepu8_epi32(String1Vec);

            const __m256i Char2Vec = _mm256_set_epi32(
                (unsigned char)string2Point[wave - (k + 7)],
                (unsigned char)string2Point[wave - (k + 6)],
                (unsigned char)string2Point[wave - (k + 5)],
                (unsigned char)string2Point[wave - (k + 4)],
                (unsigned char)string2Point[wave - (k + 3)],
                (unsigned char)string2Point[wave - (k + 2)],
                (unsigned char)string2Point[wave - (k + 1)],
                (unsigned char)string2Point[wave - (k)]
            );

            //comparison
            const __m256i vectorMatch = _mm256_cmpeq_epi32(Char1Vec, Char2Vec);

            //finding the cost
            __m256i vectorCostSub = _mm256_add_epi32(vectorSub, v_ones);
            vectorCostSub = _mm256_add_epi32(vectorCostSub, vectorMatch);
            const __m256i vectorCostInsert = _mm256_add_epi32(vectorInsert, v_ones);
            const __m256i vectorCostDelete = _mm256_add_epi32(vectorDelete, v_ones);
            const __m256i vectorResult = avxMin(vectorCostDelete, vectorCostInsert, vectorCostSub);

            _mm256_storeu_si256((__m256i*)&currentBufferPoint[k], vectorResult);
        }

        //edges for multithreading purposes
        const size_t start = iMin;
        const size_t end = iMax;

        if (end == h - 1) {
            const size_t columnIndex = wave - (h - 1);
            rowEdgePoint[columnIndex] = currentBufferPoint[h - 1];
        }

        if (start == wave - w  + 1) {
            const size_t rowIndex = start;
            columnEdgePoint[rowIndex] = currentBufferPoint[rowIndex];
        }

        if (wave == maxWave) {
            *cornerOutPoint = currentBufferPoint[h-1];
        }

        //switch the buffer pointers around
        int *temp = secondPreviousBufferPoint;
        secondPreviousBufferPoint = previousBufferPoint;
        previousBufferPoint = currentBufferPoint;
        currentBufferPoint = temp;
    }
}

struct threadArguments {
    pthread_barrier_t *barrier;

    const char *string1;
    const char *string2;
    int *rowEdge;
    int *columnEdge;
    int *corners;

    size_t length;
    size_t tileSize;
    size_t tileCountI;
    size_t tileCountJ;

    size_t threadID;
    size_t threadCount;
};

void *computeTile(void *arg) {
    const struct threadArguments *args = (struct threadArguments *)arg;

    //grab values from struct
    const char *string1 = args->string1;
    const char *string2 = args->string2;
    int *rowEdge = args->rowEdge;
    int *columnEdge = args->columnEdge;
    int *corners = args->corners;
    const size_t length = args->length;
    const size_t tileSize = args->tileSize;
    const size_t tileCountI = args->tileCountI;
    const size_t tileCountJ = args->tileCountJ;
    const size_t threadCount = args->threadCount;
    const size_t threadID = args->threadID;

    const size_t totalWaves = tileCountI + tileCountJ - 1;

    //wavefront loop
    for (size_t wave = 0; wave < totalWaves; wave++) {

        const size_t tileMin = (wave < tileCountI) ? 0 : (wave - tileCountJ + 1);
        const size_t tileMax = (wave < tileCountI) ? wave : (tileCountI - 1);

        for (size_t ti = tileMin + threadID; ti <= tileMax; ti += threadCount) {

            const size_t tj = wave - ti;

            //finding exact tile size based on predetermined size from user
            const size_t iStart = ti * tileSize + 1;
            const size_t jStart = tj * tileSize + 1;

            const size_t iEnd = minimum2(iStart + tileSize - 1, length);
            const size_t jEnd = minimum2(jStart + tileSize - 1, length);

            const size_t height = iEnd - iStart + 1;
            const size_t width = jEnd - jStart + 1;

            int currentCorner;
            if (ti == 0 && tj == 0) {
                currentCorner = 0;
            } else if (ti == 0) {
                currentCorner = rowEdge[jStart - 1];
            } else if (tj == 0) {
                currentCorner = columnEdge[iStart - 1];
            } else {
                currentCorner = corners[tj - 1];
            }

            const int bottomRightCorner = rowEdge[jStart + width - 1];

            // Intra-tile computation
            avxMath(
                &string1[iStart - 1],
                &string2[jStart - 1],
                tileSize,
                height,
                width,
                currentCorner,
                &rowEdge[jStart],
                &columnEdge[iStart],
                &corners[tj]
            );

            corners[tj] = bottomRightCorner;
        }

        // Wait until other threads finish intra-tile operations
        pthread_barrier_wait(args->barrier);
    }

    return NULL;
}

int editDistance(const char *string1, const char *string2, const size_t length) {

    if (length == 0) {
        return 0;
    }
    if (length == 1) {
        return (string1[0] == string2[0]) ? 0 : 1;
    }


    //automatically grabs all available threads from the computer or 1 if it messes up
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    if (nproc < 1) {
        nproc = 1;
    }

    const size_t tileSize = 250;

    const size_t tileCountI = (length + tileSize - 1) / tileSize;
    const size_t tileCountJ = (length + tileSize - 1) / tileSize;

    //edges and corners allocation
    int *rowEdge = (int *)malloc((length + 1) * sizeof(int));
    int *columnEdge = (int *)malloc((length + 1) * sizeof(int));
    int *corners = (int *)malloc(tileCountJ * sizeof(int));

    if (!rowEdge || !columnEdge || !corners) {
        free(rowEdge);
        free(columnEdge);
        free(corners);
        return -1;
    }

    //fill the edges
    for (size_t k = 0; k <= length; k++) {
        rowEdge[k] = k;
    }
    for (size_t k = 0; k <= length; k++) {
        columnEdge[k] = k;
    }

    const size_t minimumDimension = minimum2(tileCountI, tileCountJ);

    size_t threadCount = (size_t)nproc;

    if (threadCount > minimumDimension) {
        threadCount = minimumDimension;
    }
    if (length <= tileSize) {
        threadCount = 1;
    }

    pthread_t threads[threadCount];
    struct threadArguments threadArguments[threadCount];

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threadCount);

    // Spawn worker threads
    for (size_t i = 0; i < threadCount; i++) {
        threadArguments[i].barrier = &barrier;
        threadArguments[i].string1 = string1;
        threadArguments[i].string2 = string2;
        threadArguments[i].rowEdge = rowEdge;
        threadArguments[i].columnEdge = columnEdge;
        threadArguments[i].corners = corners;

        threadArguments[i].length = length;
        threadArguments[i].tileSize = tileSize;
        threadArguments[i].tileCountI = tileCountI;
        threadArguments[i].tileCountJ = tileCountJ;

        threadArguments[i].threadID = i;
        threadArguments[i].threadCount = threadCount;

        pthread_create(&threads[i], NULL, computeTile, &threadArguments[i]);
    }

    for (size_t i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    const int result = rowEdge[length];

    pthread_barrier_destroy(&barrier);

    free(rowEdge);
    free(columnEdge);
    free(corners);

    return result;
}