CLANG=/usr/local/opt/llvm/bin/clang

default: main
	./main

%.o: %.c %.h
	$(CLANG) -g -S -emit-llvm $*.c
	$(CLANG) -g -c $*.c

main: testee.o tester.o
	$(CLANG) -Wall -o main testee.o tester.o main.c

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc


