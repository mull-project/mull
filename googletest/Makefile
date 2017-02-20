
# This Makefile is based on:
# "How to install GTest on Mac OS X with homebrew?"
# http://stackoverflow.com/a/15872801/598057

GT_TMP_DIR=/tmp/mull_googletest_src
GT_SOURCE_DIR=$(GT_TMP_DIR)/googletest
GT_BUILD_DIR=$(PWD)/build

install: download compile link copy_headers

compile: $(GT_BUILD_DIR)/gtest-all.o $(GT_BUILD_DIR)/gtest_main.o
$(GT_BUILD_DIR)/gtest-all.o:
	mkdir -p $(GT_BUILD_DIR)
	g++ -I$(GT_SOURCE_DIR)/include -I$(GT_SOURCE_DIR) -c $(GT_SOURCE_DIR)/src/gtest-all.cc -o $(GT_BUILD_DIR)/gtest-all.o
$(GT_BUILD_DIR)/gtest_main.o:
	mkdir -p $(GT_BUILD_DIR)
	g++ -I$(GT_SOURCE_DIR)/include -I$(GT_SOURCE_DIR) -c $(GT_SOURCE_DIR)/src/gtest_main.cc -o $(GT_BUILD_DIR)/gtest_main.o

link: libgtest.a libgtest_main.a
libgtest.a: $(GT_BUILD_DIR)/gtest-all.o
	ar -rv libgtest.a $(GT_BUILD_DIR)/gtest-all.o

libgtest_main.a: $(GT_BUILD_DIR)/gtest_main.o
	ar -rv libgtest_main.a $(GT_BUILD_DIR)/gtest_main.o

# headers must be in gtest folder for imports #include "gtest/gtest.h" to work
copy_headers: gtest
gtest:
	cp -Rv $(GT_SOURCE_DIR)/include/gtest .

download: $(GT_TMP_DIR)
$(GT_TMP_DIR):
	git clone https://github.com/google/googletest.git $(GT_TMP_DIR)

clean:
	rm -rfv libgtest*
	rm -rfv build/
	rm -rfv gtest

