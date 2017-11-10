CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutation_operators/and_or_replacement/

default: main
	./main

main: main.c test_and_or_operators.o
	$(CC) -Wall -o main test_and_or_operators.o main.c

test_and_or_operators.o: test_and_or_operators.c
	$(CC) -c -emit-llvm test_and_or_operators.c
	$(CC) -S -emit-llvm test_and_or_operators.c
	$(CC) -c test_and_or_operators.c

bitcode: test_and_or_operators.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

