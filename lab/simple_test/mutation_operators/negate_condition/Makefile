
default: main
	./main

testee.o:
	clang -S -emit-llvm testee.c
	clang -c testee.c

tester.o:
	clang -S -emit-llvm tester.c
	clang -c tester.c

main: testee.o tester.o
	clang -Wall -o main testee.o tester.o main.c

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc


