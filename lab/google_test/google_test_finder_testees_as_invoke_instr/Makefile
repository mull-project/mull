MAKEFLAGS=--warn-undefined-variables

CC=/usr/local/opt/llvm@3.9/bin/clang

# CC=xcrun -sdk macosx clang

BUILD_DIR=build
GTEST_DIR=../../../googletest
DRIVER_DIR=UnitTestMain

HEADER_SEARCH_FLAGS=-I$(GTEST_DIR) -I$(GTEST_DIR)/include
MACRO_DEFINITONS=-DGTEST_NO_LLVM_RAW_OSTREAM=1
STD_VERSION=-std=c++11
# This also depends on min mac version of google test.
MACOSX_VERSION=-mmacosx-version-min=10.12
SYSROOT=-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
#NO_CXA=-fno-use-cxa-atexit
NO_CXA=

CC_FLAGS=-g $(STD_VERSION) $(HEADER_SEARCH_FLAGS) $(MACRO_DEFINITONS) $(MACOSX_VERSION) $(SYSROOT) $(NO_CXA)
# CC_FLAGS=-g -fPIC $(STD_VERSION) $(HEADER_SEARCH_FLAGS) $(MACRO_DEFINITONS) $(MACOSX_VERSION) $(SYSROOT) $(NO_CXA)

OBJECTS= $(BUILD_DIR)/Test.o

LLVM_IR=$(BUILD_DIR)/Test.ll

BITCODE=$(BUILD_DIR)/Test.bc

all: clean llvm_ir bitcode

llvm_ir: $(BUILD_DIR) $(LLVM_IR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

bitcode: $(BITCODE)

driver: $(BUILD_DIR) $(BUILD_DIR)/tests

run: driver
	$(BUILD_DIR)/tests

$(BUILD_DIR):
	mkdir $@

## Bitcode

$(BUILD_DIR)/Test.bc:
	$(CC) -emit-llvm -c $(CC_FLAGS) ./Test.cpp -o $@

## LLVM IR

$(BUILD_DIR)/Test.ll:
	$(CC) -S -emit-llvm -c $(CC_FLAGS) ./Test.cpp -o $@

## Mach-o

$(BUILD_DIR)/Test.o:
	$(CC) -c $(CC_FLAGS) ./Test.cpp -o $@

$(BUILD_DIR)/tests: $(OBJECTS)
	ld -L $(GTEST_DIR) -lgtest -lgtest_main -macosx_version_min 10.12 -lc++ -lc $^ -o $@

clean:
	rm -rf $(BUILD_DIR)


