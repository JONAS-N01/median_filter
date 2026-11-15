# 3x3 Median Filter
This repository provides three C implementation of a 3x3 median filter on an image and a simple performance test. 
The first implementation is a naive reference algorithm taken from Wikipedia, the second is a simple scalar algorithm and the third is an optimized version with AVX2 intrinsics.
## Usage
``` ./median_filter [implementation] [input image file] [output file.png] [test iterations] ```

```[implementation]``` is one of the following:  ```reference | simple | avx2```

```[input image file] [output file.png]``` are the input image file to apply the filter to and which output file to write the output png image to

```[test iterations]``` describes the number of iterations to run the test for. The output measurement removes the first 5 "warm-up" iterations. 

## Example output
``` ./median_filter avx2 images/image_large.jpg output_images/avx2_out_large.png 1000```
```Test results for avx2 filter: 1000 iterations
Loaded image pictures/img_large.jpg.
Total runtime: 5.810320 s (excluding 5 warmup runs).
Average runtime: 5.839518 ms.
Stored image at output_images/avx2_out_large.png.
```

## Performance measurements
The following measurements were conducted on a computer with an Intel Core i7-13700K CPU and 32 GB of DDR5 RAM running Ubuntu 26.04 LTS.

The following tables show the average runtime of the filter functions for the small and large images seen in the ```images``` folder:

The first table shows measurements for the program compiled with optimization ```-O1```.
|               | reference | simple  | avx2  |
|---------------|-----------|-------- |------ |
| img_small.jpg |12474.00 μs|361.43 μs|7.69 μs|    
| img_large.jpg |8689.65 ms |270.35 ms|5.83 ms|

The second table shows measurements for the program compiled with optimization ```-O3```.
|               | reference | simple  | avx2  |
|---------------|-----------|-------- |------ |
| img_small.jpg |12305.00 μs|7.89 μs  |7.33 μs|    
| img_large.jpg |8647.13 ms |6.90 ms  |5.75 ms|

The following commands were used to take these measurements:
```
with compiler flag -O1:
./median_filter reference pictures/img_small.jpg output_images/reference_out_small.png 1000
./median_filter simple    pictures/img_small.jpg output_images/simple_out_small.png    100000
./median_filter avx2      pictures/img_small.jpg output_images/avx2_out_small.png      1000000

./median_filter reference pictures/img_large.jpg output_images/reference_out_large.png 50
./median_filter simple    pictures/img_large.jpg output_images/simple_out_large.png    100
./median_filter avx2      pictures/img_large.jpg output_images/avx2_out_large.png      1000

with compiler flag -O3:
./median_filter reference pictures/img_small.jpg output_images/reference_out_small.png 1000
./median_filter simple    pictures/img_small.jpg output_images/simple_out_small.png    1000000
./median_filter avx2      pictures/img_small.jpg output_images/avx2_out_small.png      1000000

./median_filter reference pictures/img_large.jpg output_images/reference_out_large.png 50
./median_filter simple    pictures/img_large.jpg output_images/simple_out_large.png    1000
./median_filter avx2      pictures/img_large.jpg output_images/avx2_out_large.png      1000
```

## Sources
I used several external sources as help for coding, non-standard C libraries and sample images for testing.
### Code sources
- Reference algorithm and additional information on the median filter: https://en.wikipedia.org/wiki/Median_filter
- Use of the qsort function: https://www.geeksforgeeks.org/c/qsort-function-in-c/
- Optimal 9 element median sorting network: https://stackoverflow.com/questions/45453537/optimal-9-element-sorting-network-that-reduces-to-an-optimal-median-of-9-network
- AVX2 intrinsics: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
- The code does not contain any noteworthy content of the following source, however I consulted it before arriving at my current implementation: https://xhad1234.github.io/Parallel-Sort-Merge-Join-in-Peloton/
- Additionally, I used the ```perf``` utility to analyze the performance of my implementations during coding.
### C libraries
For reading and writing image files, I used the ```stb_image.h``` and ```stb_image_write.h``` libraries from the https://github.com/nothings/stb/tree/master repository.
### Images
- ```img_small.jpg``` was taken from the following paper: Al Azzeh, Jamil & Zahran, Bilal & Alqadi, Ziad. (2018). Salt and pepper noise: Effects and removal.
- ```img_large.jpg``` was taken by the photographer **Willian Justen de Vasconcellos** and is available at https://www.pexels.com/de-de/foto/beruhmte-michael-jordan-statue-im-united-center-29944200/
