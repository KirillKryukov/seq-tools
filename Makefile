.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic \
         -O3 -march=native -ffast-math -s

.PHONY: default all clean

tools := seq-t2u seq-u2t seq-merge-lines seq-split-to-lines seq-change-case-to-upper

default: $(addprefix bin/, $(tools))

bin/%: src/%.c src/common.c | bin
	$(CC) $(CFLAGS) -o $@ $<

bin:
	-mkdir bin

all: default

clean:
	-rm -f bin/*
	-rmdir bin
