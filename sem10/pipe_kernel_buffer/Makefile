.POSIX:

CC      = gcc
CFLAGS  = -g -O0 -Wall -Wextra -std=c23

.PHONY: all

all: pipe_kb_size

pipe_kb_size: pipe_kb_size.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	./pipe_kb_size.sh

.PHONY: clean

clean:
	-rm -f ./pipe_kb_size
