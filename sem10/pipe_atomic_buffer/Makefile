.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23 -fsanitize=leak

.PHONY: all

all: pipe_ab_size

pipe_ab_size: pipe_ab_size.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	./pipe_ab_size.sh

.PHONY: clean

clean:
	-find . -type f -name "pipe_ab_size" -delete
	-find . -type f -regex "\./[0-9]+-[0-9]+\.txt" -delete
