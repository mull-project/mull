int foo() {
  return 0;
}

int main() {
  return foo();
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -debug -mutators=scalar_value_mutator -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Scalar Value": {{.*}}sample.cpp:2:3 (end: 2:9)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:3: warning: Killed: Replaced scalar with 0 or 42 [scalar_value_mutator]{{$}}
CHECK:  return 0;
CHECK:  ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
