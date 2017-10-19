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

HEADER_SEARCH_FLAGS=-I$(GTEST_DIR) -I$(GTEST_DIR)/include
MACRO_DEFINITONS=-DGTEST_NO_LLVM_RAW_OSTREAM=1
STD_VERSION=-std=c++11

# This also depends on min mac version of google test.
MACOSX_VERSION=-mmacosx-version-min=10.12
SYSROOT=-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk

NO_CXA=
#NO_CXA=-fno-use-cxa-atexit

CXXFLAGS:=$(shell /opt/llvm-3.9/bin/llvm-config --cxxflags) -g
CXXFLAGS:=$(subst -O3,-O0,$(CXXFLAGS))

CC_FLAGS=$(CXXFLAGS) $(STD_VERSION) $(HEADER_SEARCH_FLAGS) $(MACRO_DEFINITONS) $(MACOSX_VERSION) $(SYSROOT) $(NO_CXA)

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

mull: config
	$(MULL) $(BUILD_DIR)/config.yml

driver: $(BUILD_DIR) $(BUILD_DIR)/tests

run: driver
	$(BUILD_DIR)/tests

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

## Bitcode

$(BUILD_DIR)/%.bc:
	$(MULL_CC) -emit-llvm -c $(CC_FLAGS) $(PWD)/$*.cpp -o $@

## LLVM IR

$(BUILD_DIR)/%.ll:
	$(MULL_CC) -S -emit-llvm -c $(CC_FLAGS) $(PWD)/$*.cpp -o $@

## Mach-o

$(BUILD_DIR)/%.o:
	$(MULL_CC) -c $(CC_FLAGS) ./$*.cpp -o $@

# -stdlib=libc++ comes from https://stackoverflow.com/a/16353112/598057
$(BUILD_DIR)/tests: $(OBJECTS) $(BUILD_DIR)/libgtest.a
	$(MULL_CC)++ -L$(BUILD_DIR) -lgtest $(OBJECTS) -o $@

clean:
	rm -rf $(BUILD_DIR)

# Google Test

$(BUILD_DIR)/libgtest.a: $(BUILD_DIR)/gtest.o $(BUILD_DIR)/gtest_main.o
	ar -rv $(BUILD_DIR)/libgtest.a $(BUILD_DIR)/gtest.o $(BUILD_DIR)/gtest_main.o

$(BUILD_DIR)/gtest.o:
	$(MULL_CC) -c $(CC_FLAGS) $(GTEST_DIR)/src/gtest-all.cc -o $@

$(BUILD_DIR)/gtest.bc:
	$(MULL_CC) -c -emit-llvm $(CC_FLAGS) $(GTEST_DIR)/src/gtest-all.cc -o $@

$(BUILD_DIR)/gtest_main.o:
	$(MULL_CC) -c $(CC_FLAGS) $(GTEST_DIR)/src/gtest_main.cc -o $@
