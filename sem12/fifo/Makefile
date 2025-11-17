.POSIX:

CC      = gcc
CFLAGS  = -g -Wall -Wextra -std=c23

.PHONY: all

all: server client

server: server.c readwrite.c readwrite.h
	$(CC) $(CFLAGS) server.c readwrite.c -o $@

client: client.c readwrite.c readwrite.h
	$(CC) $(CFLAGS) client.c readwrite.c -o $@

.PHONY: run

run:
	./server &
	./client

.PHONY: clean

clean:
	-rm ./server
	-rm ./client
	-rm ./server.fifo
	-rm ./client.fifo.*
