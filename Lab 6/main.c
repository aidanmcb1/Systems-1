#include <stdlib.h>
#include <time.h>
#include <openblas/cblas.h>

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
    const size_t block_size = 1;

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
    const size_t block_size = 1;

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

int main () {

}