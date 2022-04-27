CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
	./test.sh

build_asm:
	cc -o tmp tmp.s

clean: # Delete temporary files
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean