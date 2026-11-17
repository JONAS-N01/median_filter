#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

// Idea to load image files taken from https://stackoverflow.com/questions/2076475/read-an-image-file-in-c-c-into-an-array
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"

#include "include/utils.h"
#include "include/reference_filter.h"
#include "include/simple_filter.h"
#include "include/avx2_filter.h"
#include "include/multithreaded_filter.h"

int main(int argc, char **argv) {

    if (argc < 5) {
        printf("Usage: %s [reference|simple|avx2] [input_image] [output_file] [# of test iterations]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int32_t width, height, channels;
    
    // load image from file
    uint8_t *img_in = stbi_load(argv[2], &width, &height, &channels, 3); // force 3 channels for RGB image
    if (!img_in) {
        printf("Could not load image at %s.\n", argv[2]);
        stbi_image_free(img_in);
        return EXIT_FAILURE;
    }
    printf("Loaded image %s.\n", argv[2]);

    // pad image with 1 pixel symmetric padding
    int32_t padded_width = width + 2;
    int32_t padded_height = height + 2;
    uint8_t* img_padded = pad_img_1px(img_in, width, height);
    if (!img_padded) {
        printf("Failed to allocate storage for padded input.\n");
        stbi_image_free(img_in);
        free(img_padded);
        return EXIT_FAILURE;
    }

    stbi_image_free(img_in); // img_in is not needed anymore -> free

    // allocate buffer for filtered image
    uint8_t *img_out = malloc(padded_width * padded_height * channels * sizeof(uint8_t));
    if (!img_out) {
        printf("Failed to allocate storage for output.\n");
        free(img_out);
        free(img_padded);
        return EXIT_FAILURE;
    }

    // performance tests for the different median filter versions
    int filter_return = 0;
    struct timespec start, end;
    double total_elapsed_time = 0.0;
    int test_iterations = atoi(argv[4]);
    if (strncmp(argv[1], "avx2", 4) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            filter_return = median_filter_avx2(img_padded, img_out, padded_width, padded_height);
            clock_gettime(CLOCK_MONOTONIC, &end);
            if (i >= 5) {
                total_elapsed_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            }
        }
        
    }
    else if (strncmp(argv[1], "simple", 6) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            filter_return = median_filter_simple(img_padded, img_out, padded_width, padded_height);
            clock_gettime(CLOCK_MONOTONIC, &end);
            if (i >= 5) {
                total_elapsed_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            }
        }
    }
    else if (strncmp(argv[1], "reference", 9) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            filter_return = median_filter_reference(img_padded, img_out, padded_width, padded_height);
            clock_gettime(CLOCK_MONOTONIC, &end);
            if (i >= 5) {
                total_elapsed_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            }
        }
    }
    else if (strncmp(argv[1], "multithreaded", 13) == 0) {
        for (int i = 0; i < test_iterations; ++i) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            filter_return = median_filter_multithreaded(img_padded, img_out, padded_width, padded_height);
            clock_gettime(CLOCK_MONOTONIC, &end);
            if (i >= 5) {
                total_elapsed_time += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            }
        }
    }
    else {
        printf("Please specify the filter implementation from the following options: reference, simple, avx2, multithreaded\n");
        free(img_padded);
        free(img_out);
        return EXIT_FAILURE;
    }

    if (filter_return != 0) {
        printf("Failed to apply median filter.\n");
        free(img_padded);
        free(img_out);
        return EXIT_FAILURE;
    }

    free(img_padded); // padded image not needed anymore -> free

    printf("Test results for %s filter:\n", argv[1]);
    printf("Total runtime: %f s (excluding 5 warmup runs).\n", total_elapsed_time);
    printf("Average runtime: %f ms.\n",  (total_elapsed_time * 1000.0) / (test_iterations - 5) );

    // crop 1 pixel border from output
    uint8_t* img_cropped = crop_img_1px(img_out, padded_width, padded_height);
    if (!img_cropped) {
        printf("Failed to allocate storage for output image.\n");
        free(img_cropped);
        free(img_out);
        return EXIT_FAILURE;
    }
    free(img_out); // output image not needed anymore -> free 

    // write cropped image to specified file
    stbi_write_jpg(argv[3], width, height, channels, img_cropped, 95);
    printf("Stored image at %s.\n", argv[3]);

    free(img_cropped);

    return EXIT_SUCCESS;
}