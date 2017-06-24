
default: main
	./main

main: main.c test_and_or_operators.o
	clang -Wall -o main test_and_or_operators.o main.c

test_and_or_operators.o: test_and_or_operators.c
	clang -S -emit-llvm test_and_or_operators.c
	clang -c test_and_or_operators.c

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc


