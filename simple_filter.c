#include "include/simple_filter.h"
#include "include/utils.h"

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