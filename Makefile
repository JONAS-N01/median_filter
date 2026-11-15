all:
	gcc -O1 -Wall median_filter.c -o median_filter -lm -mavx2

debug:
	gcc -g -O1 -Wall median_filter.c -o median_filter -lm -mavx2

clean:
	rm median_filter
