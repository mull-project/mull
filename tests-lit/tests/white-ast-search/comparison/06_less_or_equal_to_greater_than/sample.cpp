int less_than_or_equal(int a, int b) {
  return a <= b;
}

int main() {
  return less_than_or_equal(2, 3) != 1;
}

// clang-format off

/**
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g %s -o %s.exe
RUN: cd %S && %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTATE

RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -debug -mutators=cxx_le_to_gt -mutate-only -output=%s-mutated.exe -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTATE
CHECK-MUTATE-NOT:{{^.*[Ee]rror.*$}}

CHECK-MUTATE:[info] Applying filter: junk (threads: 1)
CHECK-MUTATE:[debug] CXXJunkDetector: mutation "Less Or Equal To Greater Than": {{.*}}sample.cpp:2:12 (end: 2:14)

RUN: (unset TERM; %mull_runner -debug -reporters=IDE -ide-reporter-show-killed %s-mutated.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: (unset TERM; %mull_runner -debug -reporters=IDE -ide-reporter-show-killed %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced <= with > [cxx_le_to_gt]{{$}}
CHECK:  return a <= b;
CHECK:           ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
