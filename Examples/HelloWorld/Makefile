define HELP
Please provide two parameters:
MULL    - full path to mull-driver binary.
MULL_CC - path to Clang.
Note: Use Clang from http://releases.llvm.org/download.html. Mull is known to
      not work with default Mac OS Clang from Apple.
Example: make example MULL=/usr/local/bin/mull MULL_CC=/opt/llvm-3.9/bin/clang
endef
export HELP

help:
	@echo "$$HELP"

MAKEFLAGS=--warn-undefined-variables

BUILD_DIR=$(PWD)/build

GTEST_DIR=$(PWD)/../../googletest

GTEST_HEADER_SEARCH_FLAGS=-I$(GTEST_DIR) -I$(GTEST_DIR)/include

# -pthread is needed for GoogleTest.
PTHREADS:=""
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PTHREADS += -pthread
endif

CXXFLAGS=-g -O0

OBJECTS=$(BUILD_DIR)/HelloWorld.o
LLVM_IR=$(BUILD_DIR)/HelloWorld.ll

BITCODE=$(BUILD_DIR)/HelloWorld.bc \
        $(BUILD_DIR)/gtest.bc

example: clean llvm_ir bitcode config mull

llvm_ir: $(BUILD_DIR) $(LLVM_IR)

bitcode: $(BITCODE)

config:
	echo $(BUILD_DIR)/HelloWorld.bc > $(BUILD_DIR)/bitcodelist
	echo $(BUILD_DIR)/gtest.bc >> $(BUILD_DIR)/bitcodelist
	echo "bitcode_file_list: $(BUILD_DIR)/bitcodelist" > $(BUILD_DIR)/config.yml
	# echo "fork: false" >> $(BUILD_DIR)/config.yml

mull: config
	$(MULL) $(BUILD_DIR)/config.yml

driver: $(BUILD_DIR) $(BUILD_DIR)/tests

run: driver
	$(BUILD_DIR)/tests

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

## Bitcode

$(BUILD_DIR)/%.bc:
	$(MULL_CC) -emit-llvm -c $(CXXFLAGS) $(GTEST_HEADER_SEARCH_FLAGS) $(PWD)/$*.cpp -o $@

## LLVM IR

$(BUILD_DIR)/%.ll:
	$(MULL_CC) -S -emit-llvm -c $(GTEST_HEADER_SEARCH_FLAGS) $(PWD)/$*.cpp -o $@

## Mach-o

$(BUILD_DIR)/%.o:
	$(MULL_CC) -c $(GTEST_HEADER_SEARCH_FLAGS) ./$*.cpp -o $@

# -stdlib=libc++ comes from https://stackoverflow.com/a/16353112/598057
$(BUILD_DIR)/tests: $(OBJECTS) $(BUILD_DIR)/libgtest.a
	$(MULL_CC)++ -L$(BUILD_DIR) -lgtest $(OBJECTS) $(PTHREADS) -o $@

clean:
	rm -rfv $(BUILD_DIR)
	rm -rfv *.sqlite

# Google Test

$(BUILD_DIR)/libgtest.a: $(BUILD_DIR)/gtest.o $(BUILD_DIR)/gtest_main.o
	ar -rv $(BUILD_DIR)/libgtest.a $(BUILD_DIR)/gtest.o $(BUILD_DIR)/gtest_main.o

$(BUILD_DIR)/gtest.o:
	$(MULL_CC) -c $(GTEST_HEADER_SEARCH_FLAGS) $(GTEST_DIR)/src/gtest-all.cc -o $@

$(BUILD_DIR)/gtest.bc:
	$(MULL_CC) -c -emit-llvm $(CXXFLAGS) $(GTEST_HEADER_SEARCH_FLAGS) $(GTEST_DIR)/src/gtest-all.cc -o $@

$(BUILD_DIR)/gtest_main.o:
	$(MULL_CC) -c $(GTEST_HEADER_SEARCH_FLAGS) $(GTEST_DIR)/src/gtest_main.cc -o $@
