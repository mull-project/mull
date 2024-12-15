int bitwise_and(int a, int b) {
  return a & b;
}

int main() {
  return ! (bitwise_and(1, 2) == 0);
}

// clang-format off

/**
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %S && %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTATE

CHECK-MUTATE-NOT:{{^.*[Ee]rror.*$}}

CHECK-MUTATE:[info] Applying filter: junk (threads: 1)
CHECK-MUTATE:[debug] CXXJunkDetector: mutation "Bitwise And to Or": {{.*}}sample.cpp:2:12 (end: 2:13)

RUN: (unset TERM; %mull_runner -debug -reporters=IDE -ide-reporter-show-killed %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced & with | [cxx_and_to_or]{{$}}
CHECK:  return a & b;
CHECK:           ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/