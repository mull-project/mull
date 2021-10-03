static int globalVar = -1;
void voidFunction() {
  globalVar = 0;
}

int foo() {
  voidFunction();
  return 0;
};

int main() {
  foo();
  return globalVar;
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -debug -mutators=cxx_remove_void_call -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Remove Void": {{.*}}sample.cpp:7:3 (end: 7:17)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:7:3: warning: Killed: Removed the call to the function [cxx_remove_void_call]{{$}}
CHECK:  voidFunction();
CHECK:  ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
