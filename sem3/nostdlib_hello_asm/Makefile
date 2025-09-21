.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -nostdlib

.PHONY: all

all: hello readelf.txt objdump.txt strace.txt

hello: hello.s
	$(CC) $< $(CFLAGS) -o $@

readelf.txt: hello
	readelf -a ./$< > $@

objdump.txt: hello
	objdump -s -S ./$< > $@

strace.txt: hello
	strace -o $@ ./$<

.PHONY: clean

clean:
	rm -f strace.txt
	rm -f objdump.txt
	rm -f readelf.txt
	rm -f hello
