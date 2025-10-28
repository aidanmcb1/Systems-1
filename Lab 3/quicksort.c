#include <stdio.h>

int partition(int array[], int low, int high) {
    int pivot = array[low];
    int i = low - 1;
    int j = high + 1;

    while (1) {
        do {
            i++;
        } while (array[i] < pivot);

        do {
            j--;
        } while (array[j] > pivot);

        if (i >= j)
            return j;

        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void quickSort(int *a, size_t n) {
    size_t low = 0;
    size_t high = n - 1;

    while (low < high) {
        size_t p = partition(a, low, high);

        if (p - low < high - p) {
            quickSort(a + low, p - low + 1);
            low = p + 1;
        } else {
            quickSort(a + p + 1, high - p);
            high = p;
        }
    }

}
