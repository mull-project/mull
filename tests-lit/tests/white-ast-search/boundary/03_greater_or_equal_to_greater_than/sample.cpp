int greater_than_or_equal(int a, int b) {
  return a >= b;
}

int main() {
  return ! (greater_than_or_equal(3, 3) == 1);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -debug -mutators=cxx_ge_to_gt -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Greater Or Equal to Greater Than": {{.*}}sample.cpp:2:12 (end: 2:14)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced >= with > [cxx_ge_to_gt]{{$}}
CHECK:  return a >= b;
CHECK:           ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
