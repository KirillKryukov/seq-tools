.POSIX:
.SUFFIXES:

CC = cc
CFLAGS = -std=c99 \
         -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-prototypes \
         -Wfloat-equal -Wshadow -Wswitch-default -Wswitch-enum \
         -Wconversion -Wsign-compare -Wpointer-arith -Wcast-qual \
         -Wmissing-field-initializers -Wredundant-decls \
         -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-signedness \
         -Wuninitialized -Wundef -Wcast-align \
         -Wc++-compat -Wstrict-aliasing=1 -Wvla -Winit-self -Wwrite-strings \
         -O3 -march=native -ffast-math -s

.PHONY: binaries default all check test clean

tools := seq-t2u seq-u2t seq-merge-lines seq-split-to-lines seq-change-case-to-upper \
         seq-soft-mask-bin-extract

binaries: $(addprefix bin/, $(tools))

default: binaries

bin/%: src/%.c src/common.c | bin
	$(CC) $(CFLAGS) -o $@ $<

bin:
	-mkdir -p bin

all: default

clean:
	-rm -f bin/*
	-rmdir bin
	$(MAKE) -C tests clean

check: test

test: binaries
	$(MAKE) -C tests
