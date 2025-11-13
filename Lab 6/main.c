#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <openblas/cblas.h>
#include <openblas/openblas_config.h>

void naive(const double* A, const double* B, double* C, const size_t m, const size_t n, const size_t p) {
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < p; j++) {
            double sum = 0.0;
            for (size_t k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * p + j];
            }
            C[i * p + j] = sum;
        }
    }
}

void optimized_reorder(const double* A, const double* B, double* C, const size_t m, const size_t n, const size_t p) {
    for (size_t i = 0; i < m; i++) {
        for (size_t k = 0; k < n; k++) {
            const double a_ik = A[i * n + k];
            for (size_t j = 0; j < p; j++) {
                C[i * p + j] += a_ik * B[k * p + j];
            }
        }
    }
}

void optimized_blocking(const double* A, const double* B, double* C, const size_t m, const size_t n, const size_t p) {
    const size_t block_size = 128;

    for (size_t i = 0; i < m; i += block_size) {
        for (size_t j = 0; j < p; j += block_size) {
            for (size_t k = 0; k < n; k += block_size) {

                for (size_t block_i = i; block_i < i + block_size && block_i < m; block_i++) {
                    for (size_t block_j = j; block_j < j + block_size && block_j < p; block_j++) {
                        double sum = C[block_i * p + block_j];
                         for (size_t block_k = k; block_k < k + block_size && block_k < n; block_k++) {
                            sum += A[block_i * n + block_k] * B[block_k * p + block_j];
                        }
                        C[block_i * p + block_j] = sum;
                    }
                }

            }
        }
    }
}

void optimized_combined(const double* A, const double* B, double* C, const size_t m, const size_t n, const size_t p) {
    const size_t block_size = 128;

    for (size_t i = 0; i < m; i += block_size) {
        for (size_t j = 0; j < p; j += block_size) {
            for (size_t k = 0; k < n; k += block_size) {

                for (size_t block_i = i; block_i < i + block_size && block_i < m; block_i++) {
                    for (size_t block_k = k; block_k < k + block_size && block_k < n; block_k++) {
                        const double a_ik = A[block_i * n + block_k];
                        for (size_t block_j = j; block_j < j + block_size && block_j < p; block_j++) {
                            C[block_i * p + block_j] += a_ik * B[block_k * p + block_j];
                        }
                    }
                }

            }
        }
    }
}

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static void fill_random(double *a, const size_t n) {
    srand(time(NULL));
    for (size_t i = 0; i < n; ++i) {
        a[i] = (double)rand()/RAND_MAX*2.0-1.0 + rand();
    }
}

//type 1 for test - ref norms, 0 for single matrix norms
double frobeniusNorm(const double *a, const double *b, const size_t n, const int type) {
    double total = 0.0;
    for (int i = 0; i < n; ++i) {
        const double difference = a[i] - (b[i] * type);
        total += (difference * difference);
    }
    return sqrt(total);
}

double gflops(const double n, const double elapsedTime) {
    return ((n * n * n) * 2) / ((elapsedTime / 1000) * 1000000000);
}

int main () {
    openblas_set_num_threads(1);

    const int n = 4000, m = 4000, p = 4000;
    double *a = malloc(n * m * sizeof(double));
    double *b = malloc(n * p * sizeof(double));
    double *cNaive = malloc(m * p * sizeof(double));
    double *cReorder = malloc(m * p * sizeof(double));
    double *cBlocking = malloc(m * p * sizeof(double));
    double *cCombined = malloc(m * p * sizeof(double));
    double *cOpenBlas = malloc(m * p * sizeof(double));

    if (a == NULL || b == NULL || cNaive == NULL || cReorder == NULL || cBlocking == NULL || cCombined == NULL || cOpenBlas == NULL)
    {
        printf("Malloc failed\n");
        free(a);
        free(b);
        free(cNaive);
        free(cReorder);
        free(cBlocking);
        free(cCombined);
        free(cOpenBlas);
        return 1;
    }

    fill_random(a, n * m);
    fill_random(b, n * p);

    printf("At n = %d\n", n);

    double start = now_ms();
    naive(a, b, cNaive, m, n, p);
    double end = now_ms();
    double time = end - start;
    printf("Naive: %.4f ms: %.4f GFLOPS\n", time, gflops(n, (int)time));

    start = now_ms();
    optimized_reorder(a, b, cReorder, m, n, p);
    end = now_ms();
    time = end - start;
    printf("Reordered loops: %.4f ms: %.4f GFLOPS\n", time, gflops(n, (int)time));

    start = now_ms();
    optimized_blocking(a, b, cBlocking, m, n, p);
    end = now_ms();
    time = end - start;
    printf("Blocking: %.4f ms: %.4f GFLOPS\n", time, gflops(n, (int)time));

    start = now_ms();
    optimized_combined(a, b, cCombined, m, n, p);
    end = now_ms();
    time = end - start;
    printf("Combined optimizations: %.4f ms: %.4f GFLOPS\n", time, gflops(n, (int)time));

    start = now_ms();
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, p, n, 1.0, a, n, b, p, 0.0, cOpenBlas, p);
    end = now_ms();
    time = end - start;
    printf("OpenBlas: %.4f ms: %.4f GFLOPS\n", time, gflops(n, (int)time));

    printf("Relative error:\n");
    printf("Naive: %.12f%%\n", (frobeniusNorm(cNaive, cOpenBlas, m * p, 1) / frobeniusNorm(cNaive, cOpenBlas, m * p, 0)) * 100);
    printf("Reordered loops: %.12f%%\n", (frobeniusNorm(cReorder, cOpenBlas, m * p, 1) / frobeniusNorm(cReorder, cOpenBlas, m * p, 0)) * 100);
    printf("Blocking: %.12f%%\n", (frobeniusNorm(cBlocking, cOpenBlas, m * p, 1) / frobeniusNorm(cBlocking, cOpenBlas, m * p, 0)) * 100);
    printf("Combined: %.12f%%\n", (frobeniusNorm(cCombined, cOpenBlas, m * p, 1) / frobeniusNorm(cCombined, cOpenBlas, m * p, 0)) * 100);

    free(a);
    free(b);
    free(cNaive);
    free(cReorder);
    free(cBlocking);
    free(cCombined);
    free(cOpenBlas);
}