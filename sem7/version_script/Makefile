.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

DIR3 = "1.3.0"
DIR4 = "1.4.0"

SUBDIRS = $(DIR3) $(DIR4)

.PHONY: all

all: $(SUBDIRS) vs130 vs140

$(SUBDIRS):
	$(MAKE) -C $@

vs130: vs.c $(DIR3)
	@echo "\e[1;31mLinking against version 1.3.0, soname is libtri207.so.1.\e[0m"
	$(CC) $< -o $@ -L$(DIR3) -ltri207 $(CFLAGS)
	ldd $@

vs140: vs.c $(DIR4)
	@echo "\e[1;31mLinking against version 1.4.0, soname is libtri207.so.1.\e[0m"
	$(CC) $< -o $@ -L$(DIR4) -ltri207 $(CFLAGS)
	ldd $@

.PHONY: run

run: vs130 vs140
	@echo "\e[1;31mRunning vs130...\e[0m"
	./vs130 1
	@echo "\e[1;31mRunning vs140...\e[0m"
	./vs140 1

.PHONY: clean

clean:
	-$(MAKE) -C $(DIR3) clean
	-$(MAKE) -C $(DIR4) clean
	-rm -f ./vs130
	-rm -f ./vs140
