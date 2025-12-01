.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: tcp_sv

tcp_sv: tcp_sv.c
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: run

run:
	chmod u+x ./sv.sh
	./sv.sh

.PHONY: clean

clean:
	-rm ./tcp_sv
