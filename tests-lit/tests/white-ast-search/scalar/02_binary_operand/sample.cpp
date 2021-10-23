/// TODO: IDE reporter reports location of mutation instruction which is '+'
/// but we would rather want to see the location of '5'.
int foo(int a) {
  return a + 5;
}

int main() {
  return foo(-5);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -debug -mutators=scalar_value_mutator -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Scalar Value": {{.*}}sample.cpp:4:12 (end: 4:13)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:4:12: warning: Killed: Replaced scalar with 0 or 42 [scalar_value_mutator]{{$}}
CHECK:  return a + 5;
CHECK:           ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
