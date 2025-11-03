.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: pipeline

pipeline: pipeline.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run

run:
	ls -h
	ls -h | wc -l
	./pipeline ls -h wc -l

.PHONY: clean

clean:
	-rm -f ./pipeline
