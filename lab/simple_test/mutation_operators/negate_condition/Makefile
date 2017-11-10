CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutation_operators/negate_condition/

default: main
	./main

testee.o:
	$(CC) -S -emit-llvm testee.c
	$(CC) -c -emit-llvm testee.c
	$(CC) -c testee.c

tester.o:
	$(CC) -S -emit-llvm tester.c
	$(CC) -c -emit-llvm tester.c
	$(CC) -c tester.c

main: testee.o tester.o
	$(CC) -Wall -o main testee.o tester.o main.c

bitcode: testee.o tester.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

