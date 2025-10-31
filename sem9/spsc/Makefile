.POSIX:

CC      = clang
CFLAGS  = -Wall -Wextra -std=c23 -fsanitize=thread

.PHONY: all

all: spsc

spsc: spsc.c gettime.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: run

run:
	./spsc
	# strace -o spsc.strace -e inject=clone3:error=EPERM ./spsc

.PHONY: clean

clean:
	-rm -f ./spsc
