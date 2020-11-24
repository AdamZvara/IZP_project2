sps: sps.c
	gcc -std=c99 -Wall -g -Wextra -Werror -fsanitize=address sps.c -o output

all: sps
