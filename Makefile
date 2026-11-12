all:
	gcc -Wall median_filter.c -o median_filter -lm -mavx2

debug:
	gcc -g -Wall median_filter.c -o median_filter -lm -mavx2

clean:
	rm median_filter
