.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: pingpong

pingpong: pingpong.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	./pingpong -q 2 -d 0.25 -a 0 -z 10

.PHONY: clean

clean:
	-rm -f ./pingpong
