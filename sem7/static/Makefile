.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu23

.PHONY: all

all: a b

a: a.c tri207.h libtri207.a 
	$(CC) a.c -o a -L. -ltri207 $(CFLAGS)

b: b.c tri207.h libtri207.a 
	$(CC) b.c -o b -L. -ltri207 $(CFLAGS)

libtri207.a: sin207.o cos207.o tri207.h
	ar r libtri207.a sin207.o cos207.o
	ar tv libtri207.a

sin207.o : sin207.c tri207.h
	$(CC) -c $< $(CFLAGS)
cos207.o : cos207.c tri207.h
	$(CC) -c $< $(CFLAGS)

.PHONY: run

run: a b
	echo "s(1)" | bc -l
	echo "c(1)" | bc -l
	./a 1
	./b 1

.PHONY: clean

clean:
	-rm -f ./*.o
	-rm -f ./*.a
	-rm -f ./a
	-rm -f ./b
