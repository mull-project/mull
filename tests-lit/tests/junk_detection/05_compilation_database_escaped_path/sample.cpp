#include <string>
#include <iostream>

#ifndef ESCAPED_DEFINITION_STUB
#error "ESCAPED_DEFINITION_STUB is not defined"
#endif

#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)

int main() {
  bool equal = std::string(STRINGIFY(ESCAPED_DEFINITION_STUB)) == std::string("/src/builds/amd64-mull");
  return ! equal;
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode %TEST_CXX_FLAGS -g -O0 -DESCAPED_DEFINITION_STUB=/src/builds/amd64-mull %s -o %s.exe
RUN: sed -e "s:%PWD:%S:g" -e "s:#TEST_CXX_FLAGS:%TEST_CXX_FLAGS %sysroot:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -mutators=cxx_remove_void_call -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe

**/
