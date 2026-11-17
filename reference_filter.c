#include "include/reference_filter.h"
#include "stdlib.h"

int comp_uint8_t(const void *a, const void *b) {
    uint8_t x = *(const int *)a;
    uint8_t y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

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