int bitwise_left_shift_assign(int a, int b) {
  int c = a;
  c <<= b;
  return c;
}

int main() {
  return ! (bitwise_left_shift_assign(2, 1) == 4);
}

// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -debug -mutators=cxx_lshift_assign_to_rshift_assign -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Left Shift-Assign to Right Shift-Assign": {{.*}}sample.cpp:3:5 (end: 3:8)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:3:5: warning: Killed: Replaced <<= with >>= [cxx_lshift_assign_to_rshift_assign]{{$}}
CHECK:  c <<= b;
CHECK:    ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
