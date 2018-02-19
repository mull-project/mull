CC=/opt/llvm-3.9/bin/clang
LEVEL=../../../..
BUILD_DIR=./build
FIXTURES_DIR=$(LEVEL)/unittests/fixtures/mutators/cxx/boundary

BITCODE_FILES=main.bc

bitcode: $(BUILD_DIR) $(BITCODE_FILES)

%.bc: %.c
	$(CC) -c -emit-llvm $*.c -o $(BUILD_DIR)/$*.bc

%.bc: %.cpp
	$(CC) -c -emit-llvm $*.cpp -o $(BUILD_DIR)/$*.bc

synchronize_fixtures: bitcode $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
