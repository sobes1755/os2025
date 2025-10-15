.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

DIR3 = "1.3.0"
DIR4 = "1.4.0"

SUBDIRS = $(DIR3) $(DIR4)

.PHONY: all

all: $(SUBDIRS) a130 a140

$(SUBDIRS):
	$(MAKE) -C $@

a130: a.c $(DIR3)
	@echo "\e[1;31mLinking against libtri207.so.1.3.0, soname is libtri207.so.1.\e[0m"
	$(CC) a.c -o a130 -L$(DIR3) -ltri207 $(CFLAGS)
	ldd ./a130

a140: a.c $(DIR4)
	@echo "\e[1;31mLinking against libtri207.so.1.4.0, soname is libtri207.so.1.\e[0m"
	$(CC) a.c -o a140 -L$(DIR4) -ltri207 $(CFLAGS)
	ldd ./a140

.PHONY: run

run: a130 a140
	@echo "\e[1;31mInstalling libtri207.so.1.3.0...\e[0m"
	$(MAKE) -C $(DIR3) install
	@echo "\e[1;31mRunning a130...\e[0m"
	./a130 1
	@echo "\e[1;31mRunning a140...\e[0m"
	./a140 1
	@echo "\e[1;31mInstalling libtri207.so.1.4.0...\e[0m"
	$(MAKE) -C $(DIR4) install
	@echo "\e[1;31mRunning a130...\e[0m"
	./a130 1
	@echo "\e[1;31mRunning a140...\e[0m"
	./a140 1

.PHONY: clean

clean:
	-$(MAKE) -C $(DIR3) clean
	-$(MAKE) -C $(DIR4) clean
	-rm -f ./a130
	-rm -f ./a140
