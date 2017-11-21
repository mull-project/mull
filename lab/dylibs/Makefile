CC=/opt/llvm-3.9/bin/clang
BUILD_DIR=./build
LEVEL=../..
FIXTURES_DIR=$(PWD)/$(LEVEL)/unittests/fixtures/custom_test/dylibs

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

build_all: $(BUILD_DIR)
	$(CC) -c -emit-llvm test.c -o $(BUILD_DIR)/test.bc
	$(CC) -c -emit-llvm main.c -o $(BUILD_DIR)/main.bc
	$(CC) -shared distance.c -o $(BUILD_DIR)/libdistance.dylib
	echo $(FIXTURES_DIR)/libdistance.dylib > $(BUILD_DIR)/dynamic_libraries.list

synchronize_fixtures: build_all $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.dylib $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.list $(FIXTURES_DIR)

clean:
	rm -rf build

