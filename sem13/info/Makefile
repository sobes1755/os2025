.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: getaddrinfo

getaddrinfo: getaddrinfo.c
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: run

run:
	./getaddrinfo -h localhost -s echo

.PHONY: clean

clean:
	-rm ./getaddrinfo
