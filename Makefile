all:
	gcc -Wall median_filter.c -o median_filter -lm

debug:
	gcc -g -Wall median_filter.c -o median_filter -lm

clean:
	rm median_filter
