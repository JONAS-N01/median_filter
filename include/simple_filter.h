#ifndef SIMPLE_FILTER_H
#define SIMPLE_FILTER_H

#include "stdint.h"

// Simplified version of the 3x3 median filter that iterates over the image byte values instead of whole pixels
//  - applies a 3x3 median filter to img_in and writes the result to img_out
int median_filter_simple(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height);

#endif