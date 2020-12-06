sps: sps.c
	gcc -std=c99 -g -fsanitize=address sps.c -o sps

all:
	./spstest.sh

