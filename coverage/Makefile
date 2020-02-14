.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -std=c99 -O0 -coverage

.PHONY: default all coverage clean

default: coverage

all: default

coverage: seq-tools.c.gcov

seq-tools.c.gcov: seq-tools.gcda seq-tools.gcno
	@gcov seq-tools.c

seq-tools%gcda seq-tools%gcno: seq-tools
	@rm -f ../tests/*/*.out
	@rm -f ../tests/*/*.err
	../tests/test-runner.pl --bindir . --testdir ../tests 20k ascii errors small

seq-tools: ../src/seq-tools.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	@rm -f seq-tools*
