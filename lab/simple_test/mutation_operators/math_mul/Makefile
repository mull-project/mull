CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutation_operators/math_mul/

default: main
	./main

main: math_mul.o
	$(CC) -Wall -o main math_mul.o main.c

math_mul.o:
	$(CC) -S -emit-llvm math_mul.c
	$(CC) -c -emit-llvm math_mul.c
	$(CC) -c math_mul.c

bitcode: math_mul.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

