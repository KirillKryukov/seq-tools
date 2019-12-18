.POSIX:
.SUFFIXES:

export prefix = /usr/local

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

.PHONY: default all mockobjs check test coverage clean install uninstall

MOCKS := fclose malloc

BINARY := seq-tools

default: $(BINARY)

all: default

$(BINARY): src/seq-tools.c
	$(CC) $(CFLAGS) -o $@ $<

mockobjs: $(addsuffix .so, $(addprefix so/, $(MOCKS)))

so/%.so: src/mocks/%.c | so
	$(CC) $(CFLAGS) -shared -o $@ $<

so:
	@mkdir -p so

check: test

test: $(BINARY) mockobjs
	@$(MAKE) -C tests

# Coverage does not actually depend on normally built binary,
# but still it should be tested only after normal built succeeds.
coverage: $(BINARY) mockobjs
	@$(MAKE) -C coverage

clean:
	@rm -f seq-tools
	@rm -f so/*
	@rm -df so
	@$(MAKE) -C tests clean
	@$(MAKE) -C coverage clean
	@echo "All clean!"

install: $(BINARY)
	@mkdir -p $(DESTDIR)$(prefix)/bin
	cp -f $(BINARY) $(DESTDIR)$(prefix)/bin

uninstall:
	rm -f $(DESTDIR)$(prefix)/bin/$(BINARY)
