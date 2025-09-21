.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -nostdlib

.PHONY: all

all: syscall_exit readelf.txt objdump.txt strace.txt

syscall_exit: syscall_exit.s
	$(CC) $< $(CFLAGS) -o $@	

readelf.txt: syscall_exit
	readelf -a ./$< > $@

objdump.txt: syscall_exit
	objdump -s -S ./$< > $@

strace.txt: syscall_exit
	strace -o $@ ./$<

.PHONY: clean

clean:
	rm -f strace.txt
	rm -f objdump.txt
	rm -f readelf.txt
	rm -f syscall_exit
