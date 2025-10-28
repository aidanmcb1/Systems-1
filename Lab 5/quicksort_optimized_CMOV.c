#include <stdio.h>
#include <stddef.h>

const int cutoff_cmov = 16;

static inline void cmovSwap(int *a, int *b, int cond) {
    int ai = *a, bi = *b;
    int na = cond ? bi : ai;
    int nb = cond ? ai : bi;
    *a = na;
    *b = nb;
}    

static void insertionSort(int *a, int n) {
    for (int i = 1; i < n; i++) {
        int x = a[i];
        if (x >= a[i - 1]) {
            continue;
        }
        int j = i - 1;
        while (j >= 0 && a[j] > x) {
            a[j + 1] = a[j];
            j--;
        }
    }
}

static int median3(int *a, int low, int high) {
    int mid = low + ((high - low) >> 1);
    int x = a[low], y = a[mid], z = a[high];
    if (x < y) {
        if (y < z) {
            return mid;
        } else if (x < z) {
            return high;
        } else {
            return low;
        }
    } else {
        if (x < z) {
            return low;
        } else if (y < z) {
            return high;
        } else {
            return mid;
        }
    }
}

static int partition_cmov(int array[], int low, int high) {
    int pivot_idx = median3(array, low, high);
    int pivot = array[pivot_idx];
    int i = low;

    for (int j = low; j < high; j++) {
        cmovSwap(&array[i], &array[j], array[j] < pivot);
        if (array[i] < pivot) ++i; /* maintain i as last < pivot index */
    }

    int temp = array[i + 1];
    array[i + 1] = array[high];
    array[high] = temp;
    return i + 1;
}

void quicksort_optimized_CMOV(int *a, int low, int high) {
    if (high - low + 1 <= cutoff_cmov) {
        insertionSort(a + low, high - low + 1);
        return;
    }

    int p = partition_cmov(a, low, high);

    quicksort_optimized_CMOV(a, low, p - 1);
    quicksort_optimized_CMOV(a, p + 1, high);
}
