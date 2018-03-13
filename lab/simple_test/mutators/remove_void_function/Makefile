include ../../../Makefile.common

LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutators/remove_void_function/

default: main
	./main

%.o: %.c
	$(CC) -S -emit-llvm $*.c
	$(CC) -c -emit-llvm $*.c
	$(CC) -c $*.c

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

