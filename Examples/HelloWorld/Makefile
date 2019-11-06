MAKEFLAGS=--warn-undefined-variables

BUILD_DIR=build

GTEST_DIR=../../vendor/googletest

GTEST_HEADER_SEARCH_FLAGS=-I$(GTEST_DIR) -I$(GTEST_DIR)/include

# -pthread is needed for GoogleTest.
PTHREADS:=""
SYSROOT:=""
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
    PTHREADS += -pthread
endif
ifeq ($(UNAME_S),Darwin)
    SYSROOT += --sysroot=$(shell xcrun -show-sdk-path)
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: config

run_cxx: prepare_cxx
	mull-cxx -test-framework=GoogleTest $(BUILD_DIR)/HelloWorldTests

prepare_cxx: $(BUILD_DIR)
	$(CXX) $(SYSROOT) \
    -g -O0 -fembed-bitcode $(GTEST_HEADER_SEARCH_FLAGS) $(PTHREADS) \
    HelloWorld.cpp $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/gtest_main.cc \
    -o $(BUILD_DIR)/HelloWorldTests

