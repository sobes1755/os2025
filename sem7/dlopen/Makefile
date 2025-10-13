.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu23

.PHONY: all

all: d

d: d.c libtri207.so
	$(CC) $< -o $@ $(CFLAGS)

libtri207.so: sin207.o cos207.o tri207.h
	$(CC) -shared -o libtri207.so sin207.o cos207.o
	readelf --symbols --use-dynamic libtri207.so

sin207.o : sin207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)
cos207.o : cos207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)

.PHONY: run

run: d
	ldd ./d
	./d 1

.PHONY: clean

clean:
	-rm -f ./*.o
	-rm -f ./*.so
	-rm -f ./d
