.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: munge

munge: munge.c
	$(CC) $< $(CFLAGS) -o $@	

.PHONY: run

run: munge
	./$<

.PHONY: clean

clean:
	rm -f munge
