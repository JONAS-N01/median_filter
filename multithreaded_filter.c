#include "include/multithreaded_filter.h"
#include "include/utils.h"

#include "pthread.h"
#include "unistd.h"
#include "x86intrin.h"
#include "stdlib.h"

void* blocked_median_filter_avx2(void* inputs) {
    struct thread_args *in = inputs;
    int32_t width = 3 * in->width;

    int32_t idx = in->start_idx;
    while ((uintptr_t)&in->img_out[idx] % 32 != 0  &&  idx < in->end_idx) {
        uint8_t tl = in->img_in[idx - width - 3];
        uint8_t tc = in->img_in[idx - width];
        uint8_t tr = in->img_in[idx - width + 3];

        uint8_t ml = in->img_in[idx - 3];
        uint8_t mc = in->img_in[idx];
        uint8_t mr = in->img_in[idx + 3];

        uint8_t bl = in->img_in[idx + width - 3];
        uint8_t bc = in->img_in[idx + width];
        uint8_t br = in->img_in[idx + width + 3];

        in->img_out[idx] = get_median_9(tl, tc, tr, ml, mc, mr, bl, bc, br);

        ++idx;
    }

    __m256i t_curr = _mm256_loadu_si256((__m256i*) &in->img_in[idx - width - 3]);
    __m256i m_curr = _mm256_loadu_si256((__m256i*) &in->img_in[idx - 3]);
    __m256i b_curr = _mm256_loadu_si256((__m256i*) &in->img_in[idx + width - 3]);
    for (; idx < in->end_idx - 31 - 3 - 32; idx += 32) {
        __m256i t_next = _mm256_loadu_si256((__m256i*) &in->img_in[idx - width - 3 + 32]);
        __m256i m_next = _mm256_loadu_si256((__m256i*) &in->img_in[idx - 3 + 32]);
        __m256i b_next = _mm256_loadu_si256((__m256i*) &in->img_in[idx + width - 3 + 32]);

        __m256i t_temp = _mm256_permute2x128_si256(t_curr, t_next, 0x21);
        __m256i m_temp = _mm256_permute2x128_si256(m_curr, m_next, 0x21);
        __m256i b_temp = _mm256_permute2x128_si256(b_curr, b_next, 0x21);

        __m256i tl = t_curr;
        __m256i tc = _mm256_alignr_epi8(t_temp, t_curr, 3);
        __m256i tr = _mm256_alignr_epi8(t_temp, t_curr, 6);

        __m256i ml = m_curr;
        __m256i mc = _mm256_alignr_epi8(m_temp, m_curr, 3);
        __m256i mr = _mm256_alignr_epi8(m_temp, m_curr, 6);

        __m256i bl = b_curr;
        __m256i bc = _mm256_alignr_epi8(b_temp, b_curr, 3);
        __m256i br = _mm256_alignr_epi8(b_temp, b_curr, 6);

        __m256i medians = get_median_9_avx2(tl, tc, tr, ml, mc, mr, bl, bc, br);
        _mm256_stream_si256((__m256i*)&in->img_out[idx], medians);

        t_curr = t_next;
        m_curr = m_next;
        b_curr = b_next;
    }

    for (; idx < in->end_idx; ++idx) {
        uint8_t tl = in->img_in[idx - width - 3];
        uint8_t tc = in->img_in[idx - width];
        uint8_t tr = in->img_in[idx - width + 3];

        uint8_t ml = in->img_in[idx - 3];
        uint8_t mc = in->img_in[idx];
        uint8_t mr = in->img_in[idx + 3];

        uint8_t bl = in->img_in[idx + width - 3];
        uint8_t bc = in->img_in[idx + width];
        uint8_t br = in->img_in[idx + width + 3];

        in->img_out[idx] = get_median_9(tl, tc, tr, ml, mc, mr, bl, bc, br);
    }

    return NULL;
}

int median_filter_multithreaded(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height) {
    int32_t num_processors = sysconf(_SC_NPROCESSORS_ONLN); // taken from https://www.cprogramming.com/snippets/source-code/find-the-number-of-cpu-cores-for-windows-mac-or-linux/

    pthread_t threads[num_processors];
    struct thread_args inputs[num_processors];

    // I am used to the threads calculating the start- and end-indices themselves, however the thread ids are not numbered from 0 here
    // Because of that, I used the logic from the following forum post to assing blocks:
    // https://math.stackexchange.com/questions/46014/programming-logic-splitting-up-tasks-between-threads

    // calculate indices starting from the first row for the median filter
    int32_t start_idx = 3 * width + 3;
    int32_t total_indices =  3 * width * (height-1) - (3 * width + 3);
    int32_t default_chunk_size = total_indices / num_processors;
    int32_t remainder = total_indices % num_processors;

    for (int i = 0; i < num_processors; ++i) {
        int32_t chunk_size = default_chunk_size + (i < remainder);
        int32_t end_idx = start_idx + chunk_size;

        struct thread_args thread_input = {
            img_in, img_out, width, height, start_idx, end_idx
        };
        inputs[i] = thread_input;

        if(pthread_create(&(threads[i]), NULL, blocked_median_filter_avx2, &inputs[i])) {
            return -1;
        }

        start_idx = end_idx;
    }

    for (int i = 0; i < num_processors; ++i) { 
        if(pthread_join(threads[i], NULL)) {
            return -1;
        }
    }

    return 0;
}