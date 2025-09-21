.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -nostdlib

.PHONY: all

all: hello

hello: hello.c syscall.s syscall.h
	$(CC) $^ $(CFLAGS) -o $@

.PHONY: run

run:
	./hello.sh

.PHONY: clean

clean:
	rm -f hello
