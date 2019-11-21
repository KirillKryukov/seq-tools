.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -std=gnu99 -Wall -Wextra -O3 -march=native -ffast-math -s -Wno-unused-variable

.PHONY: default all clean

tools := seq-t2u seq-u2t seq-merge-lines

default: $(addprefix bin/, $(tools))

bin/%: src/%.c | bin
	$(CC) $(CFLAGS) -o $@ $^

bin:
	-mkdir bin

all: default

clean:
	-rm -f bin/*
	-rmdir bin
