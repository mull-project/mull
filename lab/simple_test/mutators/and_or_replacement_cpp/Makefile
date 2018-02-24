CXX=/opt/llvm-3.9/bin/clang++
LEVEL=../../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/simple_test/mutators/and_or_replacement_cpp/

default: main
	./main

test_and_or_operators.o: test_and_or_operators.cpp
	$(CXX) -g -S -emit-llvm test_and_or_operators.cpp
	$(CXX) -g -c -emit-llvm test_and_or_operators.cpp
	$(CXX) -c test_and_or_operators.cpp

bitcode: test_and_or_operators.o

main: test_and_or_operators.o
	$(CXX) -Wall -o main test_and_or_operators.o main.cpp

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

