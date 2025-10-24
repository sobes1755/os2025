.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23
LDLIBS  = -lm

.PHONY: all

all: dph

dph: dph.c gettime.c gettime.h swap.h
	$(CC) dph.c gettime.c $(CFLAGS) -o $@ $(LDLIBS)

.PHONY: run

run:
	./dph -q 2 -i 0.5 -e 0.25 -t 5 -v yes

.PHONY: clean

clean:
	-rm -f ./dph
