.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23 -g

.PHONY: all

all: signalfd killqueue

signalfd: signalfd.c
	$(CC) $(CFLAGS) $< -o $@

killqueue: killqueue.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	./sigrt.sh

.PHONY: clean

clean:
	-rm -f ./signalfd
	-rm -f ./killqueue
