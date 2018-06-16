include ../../Makefile.common

BUILD_DIR=$(PWD)/build
FIXTURES_DIR=$(PWD)/../../../unittests/fixtures/simple_test/count_letters

SOURCES=$(wildcard *.c)
OBJECTS=$(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.o))
BITCODE=$(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.bc))
LLVM_IR=$(addprefix $(BUILD_DIR)/, $(SOURCES:.c=.ll))

binary: $(OBJECTS)
	$(CC) $^ -o $(BUILD_DIR)/$@

bitcode: $(BITCODE)

llvm_ir: $(LLVM_IR)

$(BUILD_DIR)/%.o: %.c $(BUILD_DIR)
	$(CC) -c -o $@ $<

$(BUILD_DIR)/%.bc: %.c $(BUILD_DIR)
	$(CC) -g -emit-llvm -c -o $@ $<

$(BUILD_DIR)/%.ll: %.c $(BUILD_DIR)
	$(CC) -S -emit-llvm -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

synchronize_fixtures: llvm_ir bitcode $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR)

clean:
	rm -rf build
