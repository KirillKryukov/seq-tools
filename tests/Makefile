
.PHONY: default all clean

default: all

all:
	@diff -q Makefile Makefile
	./test-runner.pl ascii errors
	@echo "Success!"

clean:
	@rm -f */*.out
	@rm -f */*.err
