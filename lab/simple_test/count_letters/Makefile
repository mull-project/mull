include ../../Makefile.common

LEVEL=../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/count_letters/

llvm_ir:
	$(CC) -g -S -emit-llvm test_count_letters.c
	$(CC) -g -S -emit-llvm count_letters.c

bitcode:
	$(CC) -g -c -emit-llvm test_count_letters.c
	$(CC) -g -c -emit-llvm count_letters.c

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp ./*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf main
	rm -rf *.o
	rm -rf *.bc
	rm -rf *.ll

