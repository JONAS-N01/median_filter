# 3x3 Median Filter
This repository provides four C implementations of a 3x3 median filter on an image and a simple performance test. They can be found in their respective ```*_filter.c``` files.
The first implementation is a naive reference algorithm taken from Wikipedia, the second is a simple scalar algorithm, the third is an optimized version with AVX2 intrinsics and the fourth is a multithreaded AVX2 implementation.
## Usage
``` ./median_filter [implementation] [input image file] [output file.png] [test iterations] ```

- ```[implementation]``` is one of the following:  ```reference | simple | avx2 | multithreaded```

- ```[input image file] [output file.jpg]``` are the input image file to apply the filter to and which output file to write the output jpg image to

- ```[test iterations]``` describes the number of iterations to run the test for. The output measurement removes the first 5 "warm-up" iterations. 

## Example output
``` ./median_filter simple images/img_small.jpg output_images/simple_out_small.jpg 100000```
```
Loaded image images/img_small.jpg.
Test results for simple filter:
Total runtime: 0.897175 s (excluding 5 warmup runs).
Average runtime: 0.008972 ms.
Stored image at output_images/simple_out_small.jpg.
```

## Performance measurements
The measurements were made on a computer with an Intel Core i7-13700K CPU and 32 GB of DDR5 RAM running Ubuntu 26.04 LTS.
The following tables show the average runtime of the filter functions for the small and large images seen in the ```images``` folder:

The first table shows measurements for the program compiled with optimization ```-O1```.
|               | reference | simple  | avx2  | multithreaded |
|---------------|-----------|-------- |------ |---------------|
| img_small.jpg |12817.75 μs|366.09 μs|7.61 μs|242.09 μs      |
| img_large.jpg |8653.27 ms |257.32 ms|5.79 ms|1.63 ms        |

The second table shows measurements for the program compiled with optimization ```-O3```.
|               | reference | simple  | avx2  | multithreaded |
|---------------|-----------|-------- |------ |---------------|
| img_small.jpg |12734.09 μs|8.80 μs  |7.23 μs|240.72 μs      | 
| img_large.jpg |8705.67 ms |6.81 ms  |5.74 ms|1.65 ms        |

The following commands were used to take the measurements:
```
with compiler flag -O1:
./median_filter reference     images/img_small.jpg output_images/reference_out_small.jpg     1000
./median_filter simple        images/img_small.jpg output_images/simple_out_small.jpg        100000
./median_filter avx2          images/img_small.jpg output_images/avx2_out_small.jpg          1000000
./median_filter multithreaded images/img_small.jpg output_images/multithreaded_out_small.jpg 100000

./median_filter reference     images/img_large.jpg output_images/reference_out_large.jpg     50
./median_filter simple        images/img_large.jpg output_images/simple_out_large.jpg        100
./median_filter avx2          images/img_large.jpg output_images/avx2_out_large.jpg          1000
./median_filter multithreaded images/img_large.jpg output_images/multithreaded_out_large.jpg 10000

with compiler flag -O3:
./median_filter reference     images/img_small.jpg output_images/reference_out_small.jpg     1000
./median_filter simple        images/img_small.jpg output_images/simple_out_small.jpg        1000000
./median_filter avx2          images/img_small.jpg output_images/avx2_out_small.jpg          1000000
./median_filter multithreaded images/img_small.jpg output_images/multithreaded_out_small.jpg 100000

./median_filter reference     images/img_large.jpg output_images/reference_out_large.jpg     50
./median_filter simple        images/img_large.jpg output_images/simple_out_large.jpg        1000
./median_filter avx2          images/img_large.jpg output_images/avx2_out_large.jpg          1000
./median_filter multithreaded images/img_large.jpg output_images/multithreaded_out_large.jpg 10000
```
  
## Sources
I used several external sources as help for coding, non-standard C libraries and sample images for testing.
### Code sources
- Reference algorithm and additional information on the median filter: https://en.wikipedia.org/wiki/Median_filter
- Use of the qsort function: https://www.geeksforgeeks.org/c/qsort-function-in-c/
- Optimal 9 element median sorting network: https://stackoverflow.com/questions/45453537/optimal-9-element-sorting-network-that-reduces-to-an-optimal-median-of-9-network
- AVX2 intrinsics: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
- Task splitting: https://math.stackexchange.com/questions/46014/programming-logic-splitting-up-tasks-between-threads
- CPU info: https://www.cprogramming.com/snippets/source-code/find-the-number-of-cpu-cores-for-windows-mac-or-linux/
- Multithreading: https://www.geeksforgeeks.org/c/multithreading-in-c/
- The code does not contain any noteworthy content of the following source, however I consulted it before arriving at my current implementation: https://xhad1234.github.io/Parallel-Sort-Merge-Join-in-Peloton/
- Additionally, I used the ```perf``` utility to analyze the performance of my implementations during coding.
### C libraries
For reading and writing image files, I used the ```stb_image.h``` and ```stb_image_write.h``` libraries from the https://github.com/nothings/stb/tree/master repository.
### Images
- ```img_small.jpg``` was taken from the following paper: Al Azzeh, Jamil & Zahran, Bilal & Alqadi, Ziad. (2018). Salt and pepper noise: Effects and removal.
- ```img_large.jpg``` was taken by the photographer **Willian Justen de Vasconcellos** and is available at https://www.pexels.com/de-de/foto/beruhmte-michael-jordan-statue-im-united-center-29944200/
