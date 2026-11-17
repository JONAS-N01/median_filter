#ifndef UTILS_H
#define UTILS_H

#include "stdint.h"
#include "x86intrin.h"

// Adds symmetric one pixel padding around the input image
// Assumes a three channel image
uint8_t* pad_img_1px(uint8_t *img, int32_t width, int32_t height);

// Removes a one pixel border from the image
// Assumes a three channel image
uint8_t* crop_img_1px(uint8_t *img, int32_t width, int32_t height);

// macros used for scalar sorting network
// used for simple and avx2 versions of the filter
#define MIN(a, b)  a < b ? a : b
#define MAX(a, b)  a < b ? b : a
#define SWAP(a, b) do { uint8_t tmp = MIN(a, b); b = MAX(a, b); a = tmp; } while(0)

// Median sorting network taken from https://stackoverflow.com/questions/45453537/optimal-9-element-sorting-network-that-reduces-to-an-optimal-median-of-9-network
// returns the median of the input values
static inline uint8_t get_median_9 (uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7, uint8_t x8) {
    SWAP(x0, x1); SWAP(x3, x4); SWAP(x6, x7);
    SWAP(x1, x2); SWAP(x4, x5); SWAP(x7, x8);
    SWAP(x0, x1); SWAP(x3, x4); SWAP(x6, x7);

    x3 = MAX(x0, x3); x6 = MAX(x3, x6);
    SWAP(x1, x4); x4 = MIN(x4, x7); x4 = MAX(x1, x4);
    x5 = MIN(x5, x8); x2 = MIN(x2, x5);
    SWAP(x2, x4); x4 = MIN(x4, x6); x4 = MAX(x2, x4);

    return x4;
}

// macros for AVX2 sorting network
#define AVX2_MIN(a, b) _mm256_min_epu8(a, b)
#define AVX2_MAX(a, b) _mm256_max_epu8(a, b)
#define AVX2_SWAP(a, b) do { __m256i tmp = AVX2_MIN(a, b); b = AVX2_MAX(a, b); a = tmp; } while(0)

// Same sorting networks as get_median_9, but vectorized with AVX2 intrinsics
// returns a vector of medians from the input vectors
static inline __m256i get_median_9_avx2(__m256i x0, __m256i x1, __m256i x2, __m256i x3, __m256i x4, __m256i x5, __m256i x6, __m256i x7, __m256i x8) {
    AVX2_SWAP(x0, x1); AVX2_SWAP(x3, x4); AVX2_SWAP(x6, x7);
    AVX2_SWAP(x1, x2); AVX2_SWAP(x4, x5); AVX2_SWAP(x7, x8);
    AVX2_SWAP(x0, x1); AVX2_SWAP(x3, x4); AVX2_SWAP(x6, x7);

    x3 = AVX2_MAX(x0, x3); x6 = AVX2_MAX(x3, x6);
    AVX2_SWAP(x1, x4); x4 = AVX2_MIN(x4, x7); x4 = AVX2_MAX(x1, x4);
    x5 = AVX2_MIN(x5, x8); x2 = AVX2_MIN(x2, x5);
    AVX2_SWAP(x2, x4); x4 = AVX2_MIN(x4, x6); x4 = AVX2_MAX(x2, x4);

    return x4;
}

#endif
