#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "quicksort_unoptimized.h"
#include "quicksort_optimized_CMOV.h"
#include "quicksort_optimized_XOR.h"

static void fill_random(int *a, size_t n) {
    srand((unsigned)time(NULL)); // vary data each run
    for (size_t i = 0; i < n; ++i) {
        a[i] = rand(); // [0, RAND_MAX]
    }
}

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

int comp(const void *a, const void *b) {
    int intA = *(const int *)a;
    int intB = *(const int *)b;
    return (intA > intB) - (intA < intB);
}

int correctCheck(int *a, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (a[i] > a[i+1]) {
            return 0;
        }
    }
    return 1;
}

static inline void noBranchSelect(int *a, int low, int high) {
    #ifdef __clang__
        return quicksort_optimized_CMOV(a, low, high);
    #else
        return quicksort_optimized_XOR(a, low, high);
    #endif
}

int main() {
    int arraySize = 10000000;
    int *A = malloc(arraySize * sizeof(int));
    int *B = malloc(arraySize * sizeof(int));
    int *C = malloc(arraySize * sizeof(int));

    fill_random(A, arraySize);
    memcpy(B, A, arraySize * sizeof(int));
    memcpy(C, A, arraySize * sizeof(int));

    printf("Before sort:\n");
    printf("Arrays are sorted %s\n", correctCheck(A, arraySize) ? "correctly" : "incorrectly");
    
    printf("\nResults\n");

    double startA = now_ms();
    quicksort_unoptimized(A, 0, arraySize);
    double endA = now_ms();
    printf("Unoptimized quicksort: %.3f ms\n", endA - startA);

    double startB = now_ms();
    noBranchSelect(B, 0, arraySize);
    double endB = now_ms();
    printf("Optimized quicksort: %.3f ms\n", endB - startB);

    double startC = now_ms();
    qsort(C, arraySize, sizeof(int), comp);
    double endC = now_ms();
    printf("Qsort: %.3f ms\n\n", endB - startB);

    printf("After sort:\n");
    printf("Unoptimized quicksort is sorted %s\n", correctCheck(A, arraySize) ? "correctly" : "incorrectly");
    printf("Optimized quicksort is sorted %s\n", correctCheck(B, arraySize) ? "correctly" : "incorrectly");
    printf("qsort's array is sorted %s\n", correctCheck(C, arraySize) ? "correctly" : "incorrectly");

    free(A);
    free(B);
    free(C);
    return 0;
}