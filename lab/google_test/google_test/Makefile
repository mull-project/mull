MAKEFLAGS=--warn-undefined-variables

include ../../Makefile.common

LEVEL=../../..
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/google_test/google_test/

BUILD_DIR=build
GTEST_DIR=../../../googletest
DRIVER_DIR=UnitTestMain

HEADER_SEARCH_FLAGS=-I$(GTEST_DIR) -I$(GTEST_DIR)/include
MACRO_DEFINITONS=-DGTEST_NO_LLVM_RAW_OSTREAM=1
STD_VERSION=-std=c++11

CC_FLAGS=-g -fno-exceptions -fno-rtti -fPIC $(STD_VERSION) $(HEADER_SEARCH_FLAGS) $(MACRO_DEFINITONS) $(MACOSX_VERSION) $(SYSROOT)

OBJECTS= $(BUILD_DIR)/Test.o \
        $(BUILD_DIR)/Testee.o

LLVM_IR=$(BUILD_DIR)/Test.ll \
        $(BUILD_DIR)/Testee.ll

BITCODE=$(BUILD_DIR)/Test.bc \
        $(BUILD_DIR)/Testee.bc

all: clean llvm_ir bitcode

llvm_ir: $(BUILD_DIR) $(LLVM_IR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

bitcode: $(BITCODE)

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR)

driver: $(BUILD_DIR) $(BUILD_DIR)/tests

run: driver
	$(BUILD_DIR)/tests

## Bitcode

$(BUILD_DIR)/Test.bc: $(BUILD_DIR)
	$(CC) -emit-llvm -c $(CC_FLAGS) ./Test.cpp -o $@

$(BUILD_DIR)/Testee.bc: $(BUILD_DIR)
	$(CC) -emit-llvm -c $(CC_FLAGS) ./Testee.cpp -o $@

## LLVM IR

$(BUILD_DIR)/Test.ll: $(BUILD_DIR)
	$(CC) -S -emit-llvm -c $(CC_FLAGS) ./Test.cpp -o $@

$(BUILD_DIR)/Testee.ll: $(BUILD_DIR)
	$(CC) -S -emit-llvm -c $(CC_FLAGS) ./Testee.cpp -o $@

## Mach-o

$(BUILD_DIR)/Test.o: $(BUILD_DIR)
	$(CC) -c $(CC_FLAGS) ./Test.cpp -o $@

$(BUILD_DIR)/Testee.o: $(BUILD_DIR)
	$(CC) -c $(CC_FLAGS) ./Testee.cpp -o $@

$(BUILD_DIR)/tests: $(OBJECTS)
	ld -L $(GTEST_DIR) -lgtest -lgtest_main -macosx_version_min 10.12 -lc++ -lc $^ -o $@

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

clean:
	rm -rf $(BUILD_DIR)

