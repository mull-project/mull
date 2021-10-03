int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -reporters=IDE -ide-reporter-show-killed %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-DEBUG
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -reporters=IDE -ide-reporter-show-killed %s.exe -debug | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-DEBUG

WITHOUT-DEBUG-NOT:{{^.*\[debug\].*$}}
WITHOUT-DEBUG:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}

WITH-DEBUG:[debug] Diagnostics: Debug Mode enabled. Debug-level messages will be printed.
WITH-DEBUG:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
**/
