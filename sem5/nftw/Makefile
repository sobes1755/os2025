.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu23

.PHONY: all

all: nftw

nftw: nftw.c
	$(CC) $< $(CFLAGS) -o $@	

.PHONY: clean

clean:
	rm -f nftw
