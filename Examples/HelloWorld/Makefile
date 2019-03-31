MAKEFLAGS=--warn-undefined-variables

BUILD_DIR=build

GTEST_DIR=../../vendor/googletest

GTEST_HEADER_SEARCH_FLAGS=-I$(GTEST_DIR) -I$(GTEST_DIR)/include

# -pthread is needed for GoogleTest.
PTHREADS:=""
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PTHREADS += -pthread
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: config prepare_driver

run_driver: prepare_driver
	mull-driver $(BUILD_DIR)/config.yml

run_cxx: prepare_cxx
	mull-cxx -test-framework=GoogleTest $(BUILD_DIR)/HelloWorldTests

prepare_driver: $(BUILD_DIR) config
	$(CXX) -emit-llvm -c \
    -g -O0 $(GTEST_HEADER_SEARCH_FLAGS) \
    HelloWorld.cpp \
    -o $(BUILD_DIR)/HelloWorld.bc
	$(CXX) -emit-llvm -c \
    -g -O0 $(GTEST_HEADER_SEARCH_FLAGS) \
    $(GTEST_DIR)/src/gtest-all.cc \
    -o $(BUILD_DIR)/gtest-all.bc
	$(CXX) -emit-llvm -c \
    -g -O0 $(GTEST_HEADER_SEARCH_FLAGS) \
    $(GTEST_DIR)/src/gtest_main.cc \
    -o $(BUILD_DIR)/gtest-main.bc

config:
	echo $(BUILD_DIR)/HelloWorld.bc > $(BUILD_DIR)/bitcodelist
	echo $(BUILD_DIR)/gtest-all.bc >> $(BUILD_DIR)/bitcodelist
	echo $(BUILD_DIR)/gtest-main.bc >> $(BUILD_DIR)/bitcodelist
	echo "bitcode_file_list: $(BUILD_DIR)/bitcodelist" > $(BUILD_DIR)/config.yml

prepare_cxx: $(BUILD_DIR)
	$(CXX) \
    -g -O0 -fembed-bitcode $(GTEST_HEADER_SEARCH_FLAGS) $(PTHREADS) \
    HelloWorld.cpp $(GTEST_DIR)/src/gtest-all.cc $(GTEST_DIR)/src/gtest_main.cc \
    -o $(BUILD_DIR)/HelloWorldTests

