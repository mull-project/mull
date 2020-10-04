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

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode %TEST_CXX_FLAGS -g -O0 -DESCAPED_DEFINITION_STUB=/src/builds/amd64-mull %s -o %s.exe
RUN: sed -e "s:%PWD:%S:g" -e "s:#TEST_CXX_FLAGS:%TEST_CXX_FLAGS:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR
RUN: %MULL_EXEC -test-framework CustomTest -mutators=all -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1 | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines

CHECK-NOT:{{^.*[Ww]arning.*$}}
**/
