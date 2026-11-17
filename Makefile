all:
	gcc -O1 -Wall median_filter.c utils.c reference_filter.c simple_filter.c avx2_filter.c multithreaded_filter.c -o median_filter -lm -mavx2

debug:
	gcc -g -O1 -Wall median_filter.c utils.c reference_filter.c simple_filter.c avx2_filter.c multithreaded_filter.c -o median_filter -lm -mavx2

clean:
	rm median_filter
