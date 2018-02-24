CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutators/scalar_value/

default: main
	./main

main: scalar_value.o
	$(CC) -Wall -o main scalar_value.o main.c

%.o: %.c
	$(CC) -S -emit-llvm $*.c
	$(CC) -c -emit-llvm $*.c
	$(CC) -c $*.c

bitcode: scalar_value.o

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

