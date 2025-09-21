.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra

.PHONY: all

all: hello.s hello readelf.txt objdump.txt strace.txt

hello.s: hello.c
	$(CC) $< $(CFLAGS) -S -o $@

hello: hello.c
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
	rm -f hello.s
