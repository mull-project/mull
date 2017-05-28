
default: main
	./main

test_and_or_operators.o: test_and_or_operators.cpp
	/usr/local/opt/llvm@3.9/bin/clang++ -g -S -emit-llvm test_and_or_operators.cpp
	/usr/local/opt/llvm@3.9/bin/clang++ -c test_and_or_operators.cpp

main: test_and_or_operators.o
	/usr/local/opt/llvm@3.9/bin/clang++ -Wall -o main test_and_or_operators.o main.cpp

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc


