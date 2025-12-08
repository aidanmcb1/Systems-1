#include <immintrin.h>
#include <string.h>

//choose minimum of 3 int
int minimum(int a, int b, int c) {
    int m = a;
    if (b < m) {
        m = b;
    }
    if (c < m) {
        m = c;
    }
    return m;
}

//choose minimum of 3 m256i
__m256i avxMin(const __m256i a, const __m256i b, const __m256i c) {
    return _mm256_min_epi32(_mm256_min_epi32(a, b), c);
}

//does the big math, run on the threads
void compute_tile_avx(const char *string1Point, const char *string2Point, size_t tile_size, const size_t h, const size_t w, const int topLeft, int *rowEdgePoint, int *columnEdgePoint, int *cornerOutPoint) {

    //just a vector of only 1
    const __m256i v_ones = _mm256_set1_epi32(1);

    //avx buffers for diags
    __attribute__((aligned(32))) int secondPreviousBuffer[tile_size + 8];
    __attribute__((aligned(32))) int previousBuffer[tile_size + 8];
    __attribute__((aligned(32))) int currentBuffer[tile_size + 8];

    //buffer pointers
    int *secondPreviousBufferPoint = &secondPreviousBuffer[1];
    int *previousBufferPoint = &previousBuffer[1];
    int *currentBufferPoint = &currentBuffer[1];

    const size_t maxWave = h + w - 2;
    for (size_t wave = 0; wave <= maxWave; wave++) {

        if (wave < w) {
            previousBufferPoint[-1] = rowEdgePoint[wave];

            if (wave == 0) {
                secondPreviousBufferPoint[-1] = topLeft;
            } else {
                secondPreviousBufferPoint[-1] = rowEdgePoint[wave - 1];
            }
        }

        if (wave < h) {
            previousBufferPoint[wave] = columnEdgePoint[wave];
        }

        const size_t iMin = (wave < w) ? 0 : (wave - w + 1);
        const size_t iMax = (wave < h) ? wave : (h - 1);

        size_t k = iMin;
        for (; k + 7 <= iMax; k += 8) {
            const __m256i vectorInsert = _mm256_loadu_si256((__m256i*)&previousBufferPoint[k]);
            const __m256i vectorDelete = _mm256_loadu_si256((__m256i*)&previousBufferPoint[k - 1]);
            const __m256i vectorSub = _mm256_loadu_si256((__m256i*)&secondPreviousBufferPoint[k - 1]);

            //grab the initial string chunks for 1 and 2
            long long string1Chunk;
            memcpy(&string1Chunk, &string1Point[k], sizeof(long long));

            const __m128i String1Vec = _mm_cvtsi64_si128(string1Chunk);
            const __m256i Char1Vec = _mm256_cvtepu8_epi32(String1Vec);

            const __m256i Char2Vec = _mm256_set_epi32(
                (unsigned char)string2Point[wave - (k + 7)],
                (unsigned char)string2Point[wave - (k + 6)],
                (unsigned char)string2Point[wave - (k + 5)],
                (unsigned char)string2Point[wave - (k + 4)],
                (unsigned char)string2Point[wave - (k + 3)],
                (unsigned char)string2Point[wave - (k + 2)],
                (unsigned char)string2Point[wave - (k + 1)],
                (unsigned char)string2Point[wave - (k)]
            );

            //comparison
            const __m256i vectorMatch = _mm256_cmpeq_epi32(Char1Vec, Char2Vec);

            //finding the cost
            __m256i vectorCostSub = _mm256_add_epi32(vectorSub, v_ones);
            vectorCostSub = _mm256_add_epi32(vectorCostSub, vectorMatch);
            const __m256i vectorCostInsert = _mm256_add_epi32(vectorInsert, v_ones);
            const __m256i vectorCostDelete = _mm256_add_epi32(vectorDelete, v_ones);
            const __m256i vectorResult = avxMin(vectorCostDelete, vectorCostInsert, vectorCostSub);

            _mm256_storeu_si256((__m256i*)&currentBufferPoint[k], vectorResult);
        }

        //edges for multithreading purposes
        const size_t start = iMin;
        const size_t end = iMax;

        if (end == h - 1) {
            const size_t columnIndex = wave - (h - 1);
            rowEdgePoint[columnIndex] = currentBufferPoint[h - 1];
        }

        if (start == wave - w  + 1) {
            const size_t rowIndex = start;
            columnEdgePoint[rowIndex] = currentBufferPoint[rowIndex];
        }

        if (wave == maxWave) {
            *cornerOutPoint = currentBufferPoint[h-1];
        }

        //switch the buffer pointers around
        int *temp = secondPreviousBufferPoint;
        secondPreviousBufferPoint = previousBufferPoint;
        previousBufferPoint = currentBufferPoint;
        currentBufferPoint = temp;
    }
}

int edit_distance(const char *str1, const char *str2, size_t len) {
    return 1;
}
