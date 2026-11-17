#ifndef REFERENCE_FILTER_H
#define REFERENCE_FILTER_H

#include "stdint.h"

// taken from https://www.geeksforgeeks.org/c/qsort-function-in-c/
// uint8_t comparator for qsort used in reference function
int comp_uint8_t(const void *a, const void *b);

// Reference algorithm adapted from https://en.wikipedia.org/wiki/Median_filter
//  - applies a 3x3 median filter to img_in and writes the result to img_out
//  - assumes the image is RGB and the pixels are accessible via (y * width + x) * 3 + [0/1/2 for R/G/B]
//  - uses qsort from the stdlib
int median_filter_reference(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height);

#endif