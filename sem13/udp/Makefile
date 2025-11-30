.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: udpecho_sv udpecho_cl

udpecho_sv: udpecho_sv.c udpecho.h
	$(CC) $< -o $@ $(CFLAGS)

udpecho_cl: udpecho_cl.c udpecho.h
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: run

run:
	chmod u+x ./echo.sh
	./echo.sh

.PHONY: clean

clean:
	-rm ./udpecho_sv
	-rm ./udpecho_cl
