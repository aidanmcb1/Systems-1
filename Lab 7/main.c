#include <time.h>
#include <stdio.h>

static inline double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

void tests(void);

int main() {
    for (int i = 0; i < 3; i++) {
        double t0 = now_ms();
        tests();
        double t1 = now_ms();
        double elapsed_time = (t1 - t0) / 1000.0;
        printf("Time elapsed: %.4f seconds\n\n", elapsed_time);
    }
}