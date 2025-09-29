.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: mktime

mktime: mktime.c
	$(CC) $< $(CFLAGS) -o $@	

.PHONY: clean

clean:
	rm -f mktime
