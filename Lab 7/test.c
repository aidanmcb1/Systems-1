#include <stdio.h>

#include "editDistance.h"

static inline char *randomString(size_t n) {
    char low = 'a';
    char high = 'z';
    char *string = malloc(n + 1);
    for (size_t i = 0; i < n; i++) {
        string[i] = low + (rand() % (high - low + 1));
    }
    string[n] = '\0';
    return string;
}

void tests() {
    char string1[] = "kitten";
    char string2[] = "sitting";
    printf("Basic correctness. Expected: 2. Actual %d\n", editDistance(string1, string2, 6));

    char string3[] = "stujwbfaxm";
    char string4[] = "vuxaqraccz";
    printf("Basic correctness (random). Expected: 9. Actual %d\n", editDistance(string3, string4, 10));

    char string5[] = "sitting";
    char string6[] = "sitting";
    printf("Basic correctness (same). Expected: 0. Actual %d\n", editDistance(string5, string6, 6));

    char *string7 = randomString(50);
    char *string8 = randomString(50);
    printf("Random test, no expected. Result: %d\n", editDistance(string7, string8, 50));

    char *string9 = randomString(1000);
    char *string10 = randomString(1000);
    printf("Random test (larger size), no expected. Result: %d\n", editDistance(string9, string10, 1000));

    char *string11 = randomString(1000000);
    char *string12 = randomString(1000000);
    printf("Random test (largest), no expected. Result: %d\n", editDistance(string11, string12, 1000000));


}
