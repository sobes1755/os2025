.POSIX:

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c23

.PHONY: all

all: fork fork_pfbf fork_cewu

fork: fork.c
	$(CC) $(CFLAGS) $< -o $@

fork_pfbf: fork.c
	$(CC) $(CFLAGS) $< -o $@ -DPARENT_FLUSHES_BEFORE_FORK

fork_cewu: fork.c
	$(CC) $(CFLAGS) $< -o $@ -DCHILD_EXITS_WITH_UNDERSCORE

.PHONY: run

run:
	./fork
	./fork& sleep 1; pgrep -n "fork" | xargs kill
	./fork > fork.txt; cat fork.txt
	./fork_pfbf > fork_pfbf.txt; cat fork_pfbf.txt
	./fork_cewu > fork_cewu.txt; cat fork_cewu.txt

.PHONY: clean

clean:
	-rm -f ./fork
	-rm -f ./fork.txt
	-rm -f ./fork_pfbf
	-rm -f ./fork_pfbf.txt
	-rm -f ./fork_cewu
	-rm -f ./fork_cewu.txt
