sps: sps.c
	gcc -std=c99 -Wall -g -Wextra -Werror sps.c -o output
	./output
all: sps
