CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutation_operators/math_sub/

default: main
	./main

main: math_sub.o
	$(CC) -Wall -o main math_sub.o main.c

math_sub.o:
	$(CC) -S -emit-llvm math_sub.c
	$(CC) -c -emit-llvm math_sub.c
	$(CC) -c math_sub.c

bitcode: math_sub.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

