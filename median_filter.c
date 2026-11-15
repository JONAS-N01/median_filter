#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "x86intrin.h"

// Idea to load image files taken from https://stackoverflow.com/questions/2076475/read-an-image-file-in-c-c-into-an-array
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// macros used for scalar sorting network
#define MIN(a, b)  a < b ? a : b
#define MAX(a, b)  a < b ? b : a
#define SWAP(a, b) do { uint8_t tmp = MIN(a, b); b = MAX(a, b); a = tmp; } while(0)

// macros for AVX2 sorting network
#define AVX2_MIN(a, b) _mm256_min_epu8(a, b)
#define AVX2_MAX(a, b) _mm256_max_epu8(a, b)
#define AVX2_SWAP(a, b) do { __m256i tmp = AVX2_MIN(a, b); b = AVX2_MAX(a, b); a = tmp; } while(0)

// taken from https://www.geeksforgeeks.org/c/qsort-function-in-c/
// uint8_t comparator for qsort used in reference function
int comp_uint8_t(const void *a, const void *b) {
    uint8_t x = *(const int *)a;
    uint8_t y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

// Median sorting network taken from https://stackoverflow.com/questions/45453537/optimal-9-element-sorting-network-that-reduces-to-an-optimal-median-of-9-network
// returns the median of the input values
uint8_t get_median_9 (uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7, uint8_t x8) {
    SWAP(x0, x1); SWAP(x3, x4); SWAP(x6, x7);
    SWAP(x1, x2); SWAP(x4, x5); SWAP(x7, x8);
    SWAP(x0, x1); SWAP(x3, x4); SWAP(x6, x7);

    x3 = MAX(x0, x3); x6 = MAX(x3, x6);
    SWAP(x1, x4); x4 = MIN(x4, x7); x4 = MAX(x1, x4);
    x5 = MIN(x5, x8); x2 = MIN(x2, x5);
    SWAP(x2, x4); x4 = MIN(x4, x6); x4 = MAX(x2, x4);

    return x4;
}

// same median sorting network as above but with AVX2 intrinsics
// returns a vector of medians from the input vectors
__m256i get_median_9_avx2(__m256i x0, __m256i x1, __m256i x2, __m256i x3, __m256i x4, __m256i x5, __m256i x6, __m256i x7, __m256i x8) {
    AVX2_SWAP(x0, x1); AVX2_SWAP(x3, x4); AVX2_SWAP(x6, x7);
    AVX2_SWAP(x1, x2); AVX2_SWAP(x4, x5); AVX2_SWAP(x7, x8);
    AVX2_SWAP(x0, x1); AVX2_SWAP(x3, x4); AVX2_SWAP(x6, x7);

    x3 = AVX2_MAX(x0, x3); x6 = AVX2_MAX(x3, x6);
    AVX2_SWAP(x1, x4); x4 = AVX2_MIN(x4, x7); x4 = AVX2_MAX(x1, x4);
    x5 = AVX2_MIN(x5, x8); x2 = AVX2_MIN(x2, x5);
    AVX2_SWAP(x2, x4); x4 = AVX2_MIN(x4, x6); x4 = AVX2_MAX(x2, x4);

    return x4;
}

// Reference algorithm adapted from https://en.wikipedia.org/wiki/Median_filter
//  - applies a 3x3 median filter to img_in and writes the result to img_out
//  - assumes the image is RGB and the pixels are accessible via (y * width + x) * 3 + [0/1/2 for R/G/B]
//  - does not apply the filter to the edges of the image
//  - uses qsort from the stdlib
int median_filter_reference(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height) {
    int32_t WINDOW_LENGTH = 3; // window size 3x3
    int32_t WINDOW_SIZE = WINDOW_LENGTH * WINDOW_LENGTH;
    int32_t edge_size = WINDOW_LENGTH / 2; // leeway needed on the edge of the image to not apply the buffer out of bounds

    uint8_t *window_red   = malloc(WINDOW_SIZE * sizeof(uint8_t));
    uint8_t *window_green = malloc(WINDOW_SIZE * sizeof(uint8_t));
    uint8_t *window_blue  = malloc(WINDOW_SIZE * sizeof(uint8_t));

    if (!window_red || !window_green || !window_blue) {
        free(window_red); free(window_green); free(window_blue); 
        return -1;
    }

    // iterate over image pixel coordinates
    for (int32_t y = edge_size; y < height - edge_size; ++y) {
        for (int32_t x = edge_size; x < width - edge_size; ++x) {

            // iterate over window
            for (int32_t fy = 0; fy < WINDOW_LENGTH; ++fy) {
                for (int32_t fx = 0; fx < WINDOW_LENGTH; ++fx) {
                    // fill windows
                    window_red[fy * WINDOW_LENGTH + fx]   = img_in[( (y + fy - edge_size) * width + (x + fx - edge_size) ) * 3];
                    window_green[fy * WINDOW_LENGTH + fx] = img_in[( (y + fy - edge_size) * width + (x + fx - edge_size) ) * 3 + 1];
                    window_blue[fy * WINDOW_LENGTH + fx]  = img_in[( (y + fy - edge_size) * width + (x + fx - edge_size) ) * 3 + 2];
                }
            }
            // sort and take the median for the individual color channels
            qsort(window_red,   WINDOW_SIZE / sizeof(uint8_t), sizeof(uint8_t), comp_uint8_t);
            qsort(window_green, WINDOW_SIZE / sizeof(uint8_t), sizeof(uint8_t), comp_uint8_t);
            qsort(window_blue,  WINDOW_SIZE / sizeof(uint8_t), sizeof(uint8_t), comp_uint8_t);

            img_out[(y * width + x) * 3]     = window_red[WINDOW_SIZE / sizeof(uint8_t) / 2];
            img_out[(y * width + x) * 3 + 1] = window_green[WINDOW_SIZE / sizeof(uint8_t) / 2];
            img_out[(y * width + x) * 3 + 2] = window_blue[WINDOW_SIZE / sizeof(uint8_t) / 2];

        }
    }

    free(window_red); free(window_green); free(window_blue); 
    return 0;
}


// Simplified version of the 3x3 median filter that iterates over the image byte values instead of whole pixels
// The edges contain values, however they do not contain proper medians from a 3x3 window!
int median_filter_simple(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height) {
    width = 3 * width;

    // iterate over the bytes of the image instead of pixels
    // start with second row and end at second to last
    for (int32_t idx = width + 3; idx < (height - 1) * width; ++idx) {
        // individual variables for the 3x3 window (e.g. tl meaning top-left; mc meaning middle-center)
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
    }
    return 0;
}

// AVX2-vectorized version of the above simple implementation
// Uses AVX2 sorting networks for median calculation and streams the results into img_out
// Has the exact same edge-related issues as the simple implementation
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

int main(int argc, char **argv) {

    if (argc < 5) {
        printf("Usage: %s [reference|simple|avx2] [input_image] [output_file] [# of test iterations]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int32_t width, height, channels;
    
    uint8_t *img_in = stbi_load(argv[2], &width, &height, &channels, 3); // force 3 channels for RGB image
    if (!img_in) {
        printf("Could not load image at %s.\n", argv[2]);
        stbi_image_free(img_in);
        return EXIT_FAILURE;
    }
    printf("Loaded image %s.\n", argv[2]);
    uint8_t *img_out = calloc(width * height * channels, sizeof(uint8_t));
    if (!img_out) {
        printf("Failed to allocate storage for output.\n");
        stbi_image_free(img_in);
        free(img_out);
        return EXIT_FAILURE;
    }

    int filter_return = 0;
    clock_t time_sum = 0;
    int test_iterations = atoi(argv[4]);
    if (strncmp(argv[1], "avx2", 4) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_t filter_time = clock();
            filter_return = median_filter_avx2(img_in, img_out, width, height);
            filter_time = clock() - filter_time;
            if (i >= 5) {
                time_sum += filter_time;
            }
        }
        
    }
    else if (strncmp(argv[1], "simple", 6) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_t filter_time = clock();
            filter_return = median_filter_simple(img_in, img_out, width, height);
            filter_time = clock() - filter_time;
            if (i >= 5) {
                time_sum += filter_time;
            }
        }
    }
    else if (strncmp(argv[1], "reference", 9) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_t filter_time = clock();
            filter_return = median_filter_reference(img_in, img_out, width, height);
            filter_time = clock() - filter_time;
            if (i >= 5) {
                time_sum += filter_time;
            }
        }
    }
    else {
        printf("Please specify the filter implementation from the following options: reference, simple, avx2\n");
        stbi_image_free(img_in);
        free(img_out);
        return EXIT_FAILURE;
    }

    if (filter_return != 0) {
        printf("Failed to apply median filter.\n");
        stbi_image_free(img_in);
        free(img_out);
        return EXIT_FAILURE;
    }

    printf("Test results for %s filter:\n", argv[1]);
    printf("Total runtime: %f s (excluding 5 warmup runs).\n", ((double) time_sum) / CLOCKS_PER_SEC);
    printf("Average runtime: %f ms.\n", ( ((double) time_sum * 1000.0) / (test_iterations - 5) ) / CLOCKS_PER_SEC);

    stbi_write_png(argv[3], width, height, channels, img_out, width * channels);
    printf("Stored image at %s.\n", argv[3]);

    stbi_image_free(img_in);
    free(img_out);

    return EXIT_SUCCESS;
}