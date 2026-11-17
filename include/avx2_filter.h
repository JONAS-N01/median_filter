#ifndef AVX2_FILTER_H
#define AVX2_FILTER_H

#include "stdint.h"

// AVX2-vectorized version of the simple filter implementation
//  - applies a 3x3 median filter to img_in and writes the result to img_out
//  - uses AVX2 sorting networks for median calculation and streams the results into img_out
int median_filter_avx2(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height);

#endif