.POSIX:

CC      = clang
CFLAGS  = -Wall -Wextra -std=gnu23 -fsanitize=thread

.PHONY: all

all: rdwr rdwr_without_lock

rdwr: rdwr.c
	$(CC) $< -o $@ $(CFLAGS)
rdwr_without_lock: rdwr.c
	$(CC) $< -o $@ $(CFLAGS) -DWITHOUT_RW_LOCK

.PHONY: run

run:
	./rdwr
	./rdwr_without_lock

.PHONY: clean

clean:
	-rm -f ./rdwr
	-rm -f ./rdwr_without_lock
