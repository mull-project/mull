int callee() {
  return 0;
}

void caller(int *result) {
  *result = callee();
}

int main() {
  int result;
  caller(&result);
  return result != 0;
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -debug -mutators=cxx_replace_scalar_call -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Replace Call": {{.*}}sample.cpp:6:13 (end: 6:21)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:6:13: warning: Killed: Replaced call to a function with 42 [cxx_replace_scalar_call]{{$}}
CHECK:  *result = callee();
CHECK:            ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
