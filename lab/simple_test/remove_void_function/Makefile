
default: main
	./main

%.o: %.c
	clang -S -emit-llvm $*.c
	clang -c $*.c

main: testee.o tester.o
	clang -Wall -o main testee.o tester.o main.c

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc


