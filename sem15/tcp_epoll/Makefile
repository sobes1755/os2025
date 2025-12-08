.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: tcp_epoll_sv

tcp_epoll_sv: tcp_epoll_sv.c
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: run

run:
	chmod u+x ./tcp_epoll_sv.sh
	./tcp_epoll_sv.sh

.PHONY: clean

clean:
	-rm ./tcp_epoll_sv
