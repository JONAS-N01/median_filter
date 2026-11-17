#include "include/avx2_filter.h"
#include "include/utils.h"

int median_filter_avx2(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height) {
    width = 3 * width;
    int32_t idx = width + 3;

    // Start with scalar loop until img_out address is aligned to 32 Bytes
    while ((uintptr_t)&img_out[idx] % 32 != 0  &&  idx < (height - 1) * width) { 
        uint8_t tl = img_in[idx - width - 3]; // offset 3 because of the 3 channels
        uint8_t tc = img_in[idx - width];
        uint8_t tr = img_in[idx - width + 3];

        uint8_t ml = img_in[idx - 3];
        uint8_t mc = img_in[idx];
        uint8_t mr = img_in[idx + 3];

        uint8_t bl = img_in[idx + width - 3];
        uint8_t bc = img_in[idx + width];
        uint8_t br = img_in[idx + width + 3];

        img_out[idx] = get_median_9(tl, tc, tr, ml, mc, mr, bl, bc, br);

        ++idx;
    }

    // continue with vectorized loop
    // use "sliding" registers to reduce the number of cache/memory accesses with curr and next variables 
    __m256i t_curr = _mm256_loadu_si256((__m256i*) &img_in[idx - width - 3]);
    __m256i m_curr = _mm256_loadu_si256((__m256i*) &img_in[idx - 3]);
    __m256i b_curr = _mm256_loadu_si256((__m256i*) &img_in[idx + width - 3]);
    for (; idx < (height - 1) * width - 31 - 3 - 32; idx += 32) { // -31 to account for 32 byte vectors, -3 to account for right window values and -32 to account for next values
        __m256i t_next = _mm256_loadu_si256((__m256i*) &img_in[idx - width - 3 + 32]);
        __m256i m_next = _mm256_loadu_si256((__m256i*) &img_in[idx - 3 + 32]);
        __m256i b_next = _mm256_loadu_si256((__m256i*) &img_in[idx + width - 3 + 32]);

        // temp vectors contain the last 128 bits of curr and the first 128 bits of next
        __m256i t_temp = _mm256_permute2x128_si256(t_curr, t_next, 0x21);
        __m256i m_temp = _mm256_permute2x128_si256(m_curr, m_next, 0x21);
        __m256i b_temp = _mm256_permute2x128_si256(b_curr, b_next, 0x21);

        // _mm256_alignr_epi8 to construct center and right window values without extra memory accesses:
        //  - lower 128 bits: combine the lower bits of temp (upper bits of curr) and lower bits of curr to get back curr and shift by 3/6 bytes
        //                      -> results in the bytes 3-18 from curr for shift 3
        //  - upper 128 bits: combine the upper bits of temp (lower bits of next) and upper bits of curr and shift by 3/6 bytes to combine curr and next 
        //                    over their boundary -> e.g. for shift 3 we get the upper 13 bytes of curr and the lower 3 bytes of next
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
        _mm256_stream_si256((__m256i*)&img_out[idx], medians);

        t_curr = t_next;
        m_curr = m_next;
        b_curr = b_next;
    }

    for (; idx < (height - 1) * width; ++idx) {
        uint8_t tl = img_in[idx - width - 3];
        uint8_t tc = img_in[idx - width];
        uint8_t tr = img_in[idx - width + 3];

        uint8_t ml = img_in[idx - 3];
        uint8_t mc = img_in[idx];
        uint8_t mr = img_in[idx + 3];

        uint8_t bl = img_in[idx + width - 3];
        uint8_t bc = img_in[idx + width];
        uint8_t br = img_in[idx + width + 3];

        img_out[idx] = get_median_9(tl, tc, tr, ml, mc, mr, bl, bc, br);
    }

    return 0;
}