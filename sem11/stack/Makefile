.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23 -g

.PHONY: all

all: overflow handler1 handler2 unwinding

overflow: overflow.c
	$(CC) $(CFLAGS) $< -o $@

handler1: handler1.c
	$(CC) $(CFLAGS) $< -o $@

handler2: handler2.c
	$(CC) $(CFLAGS) $< -o $@

unwinding: unwinding.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	./unwinding


.PHONY: clean

clean:
	-rm -f ./overflow
	-rm -f ./handler1
	-rm -f ./handler2
	-rm -f ./unwinding
