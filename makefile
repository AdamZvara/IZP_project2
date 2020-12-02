sps: sps.c
	gcc -std=c99 -g -fsanitize=address sps.c -o output

all: sps
