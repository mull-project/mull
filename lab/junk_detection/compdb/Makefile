CC=/opt/llvm-3.9/bin/clang
LEVEL=../../..
BUILD_DIR=./build
FIXTURES_DIR_RELATIVE=$(LEVEL)/unittests/fixtures/junk_detection/compdb_relative_paths
FIXTURES_DIR_ABSOLUTE=$(LEVEL)/unittests/fixtures/junk_detection/compdb_absolute_paths

BITCODE_FILES=main.bc

bitcode: $(BUILD_DIR) $(BITCODE_FILES)

%.bc: %.c
	$(CC) -c -emit-llvm -I include -g $(PWD)/$*.c -o $(BUILD_DIR)/$*.bc

%.bc: %.cpp
	$(CC) -c -emit-llvm -I include -g $(PWD)/$*.cpp -o $(BUILD_DIR)/$*.bc

compdb:
	cat compile_commands.json.in | sed "s|DIR|$$PWD|" | sed "s|INC|.|" > $(BUILD_DIR)/compile_commands.json.relative
	cat compile_commands.json.in | sed "s|DIR|$$PWD|" | sed "s|INC|$$PWD|" > $(BUILD_DIR)/compile_commands.json.absolute

synchronize_fixtures: bitcode compdb $(FIXTURES_DIR_RELATIVE) $(FIXTURES_DIR_ABSOLUTE)
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR_RELATIVE)
	cp $(BUILD_DIR)/compile_commands.json.relative $(FIXTURES_DIR_RELATIVE)/compile_commands.json
	cp $(BUILD_DIR)/*.bc $(FIXTURES_DIR_ABSOLUTE)
	cp $(BUILD_DIR)/compile_commands.json.absolute $(FIXTURES_DIR_ABSOLUTE)/compile_commands.json

$(FIXTURES_DIR_ABSOLUTE):
	mkdir -p $(FIXTURES_DIR_ABSOLUTE)

$(FIXTURES_DIR_RELATIVE):
	mkdir -p $(FIXTURES_DIR_RELATIVE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

