#ifndef MULTITHREADED_FILTER_H
#define MULTITHREADED_FILTER_H

#include "stdint.h"

// struct containing the needed arguments for a thread to apply the filter to a set of indices
struct thread_args {
    uint8_t *img_in; uint8_t *img_out; int32_t width; int32_t height; int32_t start_idx; int32_t end_idx;
};

// avx2-vectorized median filter function executed by the threads
//  - The same implementation as the avx2_filter, only with specific start- end end-indices
void* blocked_median_filter_avx2(void* inputs);

// This function managed the threads
// calculates the start- and end-indices for the filter and assigns the blocks to the created threads
int median_filter_multithreaded(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height);

#endif