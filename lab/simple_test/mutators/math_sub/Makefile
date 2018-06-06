include ../../../Makefile.common

BUILD_DIR=./build
FIXTURES_DIR=../../../../unittests/fixtures/simple_test/mutators/math_sub

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
	$(CC) -emit-llvm -c -o $@ $<

$(BUILD_DIR)/%.ll: %.c $(BUILD_DIR)
	$(CC) -S -emit-llvm -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(FIXTURES_DIR):
	mkdir -p $(FIXTURES_DIR)

synchronize_fixtures: llvm_ir bitcode $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.ll $(FIXTURES_DIR)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR)

clean:
	rm -rf build
