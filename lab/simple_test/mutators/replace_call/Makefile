include ../../../Makefile.common

LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutators/replace_call/

default: main
	./main

main: replace_call.o
	$(CC) -Wall -o main replace_call.o main.c

%.o: %.c
	$(CC) -S -emit-llvm $*.c
	$(CC) -c -emit-llvm $*.c
	$(CC) -c $*.c

bitcode: replace_call.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

