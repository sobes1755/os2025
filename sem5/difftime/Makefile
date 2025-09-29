.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: difftime

difftime: difftime.c
	$(CC) $< $(CFLAGS) -o $@	

.PHONY: clean

clean:
	rm -f difftime
