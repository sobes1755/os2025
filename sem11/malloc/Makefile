.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23 -O0 -g

.PHONY: all

all: malloc

pipeline: malloc.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	./malloc.sh


.PHONY: clean

clean:
	-rm -f ./malloc
