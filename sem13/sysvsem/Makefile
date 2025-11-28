.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23
LDLIBS  = -lm

.PHONY: all

all: dphsysvsem

dphsysvsem: dphsysvsem.c gettime.c gettime.h
	$(CC) dphsysvsem.c gettime.c $(CFLAGS) -o $@ $(LDLIBS)

.PHONY: run

run:
	./dphsysvsem -q 13 -i 0.5 -e 0.25 -t 10

.PHONY: clean

clean:
	-rm ./dphsysvsem
