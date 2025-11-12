#include <stdlib.h>
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
    const size_t block_size = 32;

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
    const size_t block_size = 32;

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

int main () {
    openblas_set_num_threads(1);

    const int n = 2000, m = 2000, p = 2000;
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

    const double startNaive = now_ms();
    naive(a, b, cNaive, m, n, p);
    const double endNaive = now_ms();
    printf("Naive: %f ms\n", endNaive - startNaive);

    const double startReorder = now_ms();
    optimized_reorder(a, b, cReorder, m, n, p);
    const double endReorder = now_ms();
    printf("Reordered loops: %f ms\n", endReorder - startReorder);

    const double startBlocking = now_ms();
    optimized_blocking(a, b, cBlocking, m, n, p);
    const double endBlocking = now_ms();
    printf("Blocking: %f ms\n", endBlocking - startBlocking);

    const double startCombined = now_ms();
    optimized_combined(a, b, cCombined, m, n, p);
    const double endCombined = now_ms();
    printf("Combined optimizations: %f ms\n", endCombined - startCombined);

    const double startOpenBlas = now_ms();
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, p, n, 1.0, a, n, b, p, 0.0, cOpenBlas, p);
    const double endOpenBlas = now_ms();
    printf("OpenBlas: %f ms\n", endOpenBlas - startOpenBlas);



    free(a);
    free(b);
    free(cNaive);
    free(cReorder);
    free(cBlocking);
    free(cCombined);
    free(cOpenBlas);
}