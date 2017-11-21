CC=/opt/llvm-3.9/bin/clang
BUILD_DIR=./build
LEVEL=../..
FIXTURES_DIR=$(PWD)/$(LEVEL)/unittests/fixtures/custom_test/dylibs_and_objects

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

build_all: $(BUILD_DIR)
	$(CC) -c -emit-llvm test.c -o $(BUILD_DIR)/test.bc
	$(CC) -c -emit-llvm main.c -o $(BUILD_DIR)/main.bc
	$(CC) -c -emit-llvm distance.c -o $(BUILD_DIR)/distance.bc
	$(CC) -fPIC -c main.c -o $(BUILD_DIR)/main.o
	$(CC) -shared distance.c -o $(BUILD_DIR)/libdistance.dylib
	echo $(FIXTURES_DIR)/libdistance.dylib > $(BUILD_DIR)/dynamic_libraries.list
	echo $(FIXTURES_DIR)/main.o > $(BUILD_DIR)/object_files.list

synchronize_fixtures: build_all $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.o $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.dylib $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.list $(FIXTURES_DIR)

clean:
	rm -rf build

