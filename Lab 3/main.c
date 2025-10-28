#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "quicksort.h"

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
    if (!sizeof(a)/sizeof a[0] == size) {
        return 0;
    }
    for (int i = 0; i < size - 1; i++) {
        if (a[i] > a[i+1]) {
            return 0;
        }
    }
    return 1;
}

    int main() {
    int arraySize = 10000000;
    int *A = malloc(arraySize * sizeof(int));
    int *B = malloc(arraySize * sizeof(int));

    fill_random(A, arraySize);
    memcpy(B, A, arraySize * sizeof(int));

    printf("Before sort:\n");
    printf("My array is sorted %s\n", correctCheck(A, arraySize) ? "correctly" : "incorrectly");
    printf("qsort's array is sorted %s\n", correctCheck(B, arraySize) ? "correctly" : "incorrectly");

    double startA = now_ms();
    quickSort(A, arraySize);
    double endA = now_ms();
    printf("My quicksort: %.3f ms\n", endA - startA);

    double startB = now_ms();
    qsort(B, arraySize, sizeof(int), comp);
    double endB = now_ms();
    printf("qsort: %.3f ms\n", endB - startB);

    printf("%s wins\n", (endA - startA) < (endB - startB) ? "My quicksort" : "qsort");

    printf("After sort:\n");
    printf("My array is sorted %s\n", correctCheck(A, arraySize) ? "correctly" : "incorrectly");
    printf("qsort's array is sorted %s\n", correctCheck(B, arraySize) ? "correctly" : "incorrectly");

    free(A);
    free(B);
    return 0;
}