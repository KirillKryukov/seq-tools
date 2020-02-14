
.PHONY: default all clean

default: all

all:
	@diff -q Makefile Makefile
	./test-runner.pl 20k ascii errors small
	@echo "Success!"

clean:
	@rm -f */*.out
	@rm -f */*.err
