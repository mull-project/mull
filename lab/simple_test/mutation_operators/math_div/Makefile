CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutation_operators/math_div/

default: main
	./main

main: math_div.o
	$(CC) -Wall -o main math_div.o main.c

math_div.o:
	$(CC) -c -emit-llvm math_div.c
	$(CC) -S -emit-llvm math_div.c
	$(CC) -c math_div.c

bitcode: math_div.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

