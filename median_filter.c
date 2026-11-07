#include "stdio.h"
#include "stdlib.h"
#include "x86intrin.h"

// Idea to load image files taken from https://stackoverflow.com/questions/2076475/read-an-image-file-in-c-c-into-an-array
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// taken from https://www.geeksforgeeks.org/c/qsort-function-in-c/
int comp_uint8_t(const void *a, const void *b) {
    uint8_t x = *(const int *)a;
    uint8_t y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}


// Reference algorithm adapted from https://en.wikipedia.org/wiki/Median_filter
//  - applies a 3x3 median filter to img_in and writes the result to img_out
//  - assumes the image is RGB and the pixels are accessible via (y * width + x) * 3 + [0/1/2 for R/G/B]
//  - does not apply the filter to the edges of the image
//  - uses qsort from the stdlib
int median_filter_reference(uint8_t *img_in, uint8_t *img_out, int32_t width, int32_t height) {
    printf("width=%d, height=%d\n", width, height);
    int32_t WINDOW_LENGTH = 3; // window size 3x3
    int32_t WINDOW_SIZE = WINDOW_LENGTH * WINDOW_LENGTH;
    int32_t edge_size = WINDOW_LENGTH / 2; // leeway needed on the edge of the image to not apply the buffer out of bounds

    uint8_t *window_red   = malloc(WINDOW_SIZE * sizeof(uint8_t));
    uint8_t *window_green = malloc(WINDOW_SIZE * sizeof(uint8_t));
    uint8_t *window_blue  = malloc(WINDOW_SIZE * sizeof(uint8_t));
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
            qsort(window_red,   WINDOW_SIZE   / sizeof(uint8_t), sizeof(uint8_t), comp_uint8_t);
            qsort(window_green, WINDOW_SIZE / sizeof(uint8_t), sizeof(uint8_t), comp_uint8_t);
            qsort(window_blue,  WINDOW_SIZE  / sizeof(uint8_t), sizeof(uint8_t), comp_uint8_t);

            img_out[(y * width + x) * 3]     = window_red[WINDOW_SIZE / sizeof(uint8_t) / 2];
            img_out[(y * width + x) * 3 + 1] = window_green[WINDOW_SIZE / sizeof(uint8_t) / 2];
            img_out[(y * width + x) * 3 + 2] = window_blue[WINDOW_SIZE / sizeof(uint8_t) / 2];

        }
    }

    free(window_red); free(window_green); free(window_blue); 
    return 0;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        printf("Usage: ./median_filter [input_image] [output_file]\n");
        return 1;
    }

    int32_t width, height, channels;
    
    uint8_t *img_in = stbi_load(argv[1], &width, &height, &channels, 3); // force 3 channels for RGB image
    if (!img_in) {
        printf("Could not load image at %s.\n", argv[1]);
        return 1;
    }
    printf("Loaded image %s.\n", argv[1]);
    uint8_t *img_out = malloc(width * height * channels * sizeof(uint8_t));
    if (!img_out) {
        printf("Failed to allocate storage for output.\n");
        return 1;
    }
    
    if (median_filter_reference(img_in, img_out, width, height) != 0) {
        printf("Failed to apply median filter.\n");
        return -1;
    }
    printf("Applied median filter to image.\n");

    stbi_write_png(argv[2], width, height, channels, img_out, width * channels);
    printf("Stored image at %s.\n", argv[2]);

    stbi_image_free(img_in);
    free(img_out);

    return 0;
}