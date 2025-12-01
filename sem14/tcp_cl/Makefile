.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: tcp_cl

tcp_cl: tcp_cl.c
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: run

run:
	chmod u+x ./cl.sh
	./cl.sh

.PHONY: clean

clean:
	-rm ./tcp_cl
