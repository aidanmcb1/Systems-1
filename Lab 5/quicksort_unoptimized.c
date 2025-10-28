#include <stdio.h>
#include <stddef.h>

static void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int array[], int low, int high) {
    int pivot = array[high];
    int i = low - 1;

    for (int j = low; j <= high; j++) {
        if (array[j] < pivot) {
            i++;
            swap(&array[i], &array[j]);
        }
    }

    swap(&array[i + 1], &array[high]);  
    return i + 1;
}

void quicksort_unoptimized(int *a, int low, int high) {
    if (low >= high) return;

    int p = partition(a, low, high);

    quicksort_unoptimized(a, low, p - 1);
    quicksort_unoptimized(a, p + 1, high);
}
