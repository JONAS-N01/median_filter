#include "include/utils.h"
#include "stdlib.h"

uint8_t* pad_img_1px(uint8_t *img, int32_t width, int32_t height) {
    int32_t padded_width = (width + 2) * 3;
    int32_t padded_height = height + 2;
    width = width * 3;

    uint8_t *padded_img = malloc(padded_width * padded_height * sizeof(uint8_t));
    if (!padded_img) {
        return NULL;
    }

    // copy input image into the "middle" pixels
    for (int32_t y = 0; y < height; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            padded_img[(y+1) * padded_width + (x+3)] = img[y * width + x];
        }
    }

    // copy first and last row of img into the upper and lower borders
    for (int32_t x = 0; x < width; ++x) {
        padded_img[x+3] = padded_img[padded_width + x + 3];
        padded_img[(padded_height - 1) * padded_width + x + 3] = padded_img[(padded_height - 2) * padded_width + x + 3];
    }

    // copy first and last column of img into the left and right borders
    for (int32_t y = 0; y < height; ++y) {
        for (int32_t i = 0; i < 3; ++i) {
            padded_img[(y+1) * padded_width + i] = padded_img[(y+1) * padded_width + i + 3];
            padded_img[(y+1) * padded_width + padded_width - 3 + i] = padded_img[(y+1) * padded_width + padded_width - 6 + i];
        }
    }

    // copy img corners into the padded corners
    for (int32_t i = 0; i < 3; ++i) {
        padded_img[i] = padded_img[padded_width + 3 + i];
        padded_img[padded_width - 3 + i] = padded_img[2 * padded_width - 6 + i];
        padded_img[(padded_height-1) * padded_width + i] = padded_img[(padded_height-2) * padded_width + 3 + i];
        padded_img[(padded_height-1) * padded_width + padded_width - 3 + i] = padded_img[(padded_height-2) * padded_width + padded_width - 6 + i];
    }

    return padded_img;
}


uint8_t* crop_img_1px(uint8_t *img, int32_t width, int32_t height) {
    int32_t cropped_width = (width - 2) * 3;
    int32_t cropped_height = height - 2;
    width = width * 3;

    uint8_t *cropped_img = malloc(cropped_width * cropped_height * sizeof(uint8_t));
    if (!cropped_img) {
        return NULL;
    }

    // copy the "middle" pixels of img into cropped_img
    for (int32_t y = 0; y < cropped_height; ++y) {
        for (int32_t x = 0; x < cropped_width; ++x) {
            cropped_img[y * cropped_width + x] = img[(y+1) * width + x + 3];
        }
    }

    return cropped_img;
}


/*uint8_t get_median_9 (uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7, uint8_t x8) {
    SWAP(x0, x1); SWAP(x3, x4); SWAP(x6, x7);
    SWAP(x1, x2); SWAP(x4, x5); SWAP(x7, x8);
    SWAP(x0, x1); SWAP(x3, x4); SWAP(x6, x7);

    x3 = MAX(x0, x3); x6 = MAX(x3, x6);
    SWAP(x1, x4); x4 = MIN(x4, x7); x4 = MAX(x1, x4);
    x5 = MIN(x5, x8); x2 = MIN(x2, x5);
    SWAP(x2, x4); x4 = MIN(x4, x6); x4 = MAX(x2, x4);

    return x4;
}


__m256i get_median_9_avx2(__m256i x0, __m256i x1, __m256i x2, __m256i x3, __m256i x4, __m256i x5, __m256i x6, __m256i x7, __m256i x8) {
    AVX2_SWAP(x0, x1); AVX2_SWAP(x3, x4); AVX2_SWAP(x6, x7);
    AVX2_SWAP(x1, x2); AVX2_SWAP(x4, x5); AVX2_SWAP(x7, x8);
    AVX2_SWAP(x0, x1); AVX2_SWAP(x3, x4); AVX2_SWAP(x6, x7);

    x3 = AVX2_MAX(x0, x3); x6 = AVX2_MAX(x3, x6);
    AVX2_SWAP(x1, x4); x4 = AVX2_MIN(x4, x7); x4 = AVX2_MAX(x1, x4);
    x5 = AVX2_MIN(x5, x8); x2 = AVX2_MIN(x2, x5);
    AVX2_SWAP(x2, x4); x4 = AVX2_MIN(x4, x6); x4 = AVX2_MAX(x2, x4);

    return x4;
}*/