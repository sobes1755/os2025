.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu23

.PHONY: all

all: libtri207.so.1 libtri207.so

libtri207.so.1: sin207.o cos207.o tri207.h
	$(CC) -shared -o $@ -Wl,-soname=libtri207.so.1 -Wl,-version-script=vs140.map sin207.o cos207.o
	readelf --symbols --use-dynamic $@

libtri207.so: libtri207.so.1
	ln -f -s $< $@

sin207.o : sin207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)

cos207.o : cos207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)

.PHONY: clean

clean:
	-rm -f ./sin207.o
	-rm -f ./cos207.o
	-rm -f ./libtri207.so
	-rm -f ./libtri207.so.1