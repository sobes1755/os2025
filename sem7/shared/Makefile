.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu23

.PHONY: all

all: a b

a: a.c tri207.h libtri207.so 
	$(CC) a.c -o a -L. -ltri207 $(CFLAGS)

b: b.c tri207.h libtri207.so
	$(CC) b.c -o b -L. -ltri207 $(CFLAGS)

libtri207.so: sin207.o cos207.o tri207.h
	$(CC) -shared -o libtri207.so sin207.o cos207.o
	readelf --symbols --use-dynamic libtri207.so

sin207.o : sin207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)
cos207.o : cos207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)

.PHONY: run

run: a b
	ldd ./a
	LD_LIBRARY_PATH=. ./a 1
	ldd ./b
	LD_LIBRARY_PATH=. ./b 1

.PHONY: clean

clean:
	-rm -f ./*.o
	-rm -f ./*.so
	-rm -f ./a
	-rm -f ./b
