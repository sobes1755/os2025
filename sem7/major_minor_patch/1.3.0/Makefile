.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu23

.PHONY: all

all: libtri207.so.1.3.0 libtri207.so

libtri207.so.1.3.0: sin207.o cos207.o tri207.h
	$(CC) -shared -Wl,-soname=libtri207.so.1 -o $@ sin207.o cos207.o
	readelf --symbols --use-dynamic $@

libtri207.so: libtri207.so.1.3.0
	ln -f -s $< $@

sin207.o : sin207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)

cos207.o : cos207.c tri207.h
	$(CC) -c -fPIC $< $(CFLAGS)

.PHONY: install

install: libtri207.so.1.3.0
	sudo cp $< /usr/local/lib
	sudo cp tri207.h /usr/local/include
	sudo ldconfig -v 2>/dev/null | grep libtri207
	sudo ln -f -s /usr/local/lib/libtri207.so.1 /usr/local/lib/libtri207.so
	ls -al /usr/local/lib | grep libtri207
	sudo ldconfig -p | grep libtri207

.PHONY: clean

clean:
	-rm -f ./sin207.o
	-rm -f ./cos207.o
	-rm -f ./libtri207.so.1.3.0
	-rm -f ./libtri207.so
	-sudo rm -f /usr/local/include/tri207.h
	-sudo rm -f /usr/local/lib/libtri207.so
	-sudo rm -f /usr/local/lib/libtri207.so.1
	-sudo rm -f /usr/local/lib/libtri207.so.1.3.0
