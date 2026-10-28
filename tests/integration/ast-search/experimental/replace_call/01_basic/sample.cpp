int callee() {
  return 1;
}

void caller(int *result) {
  *result = callee();
}

int main() {
  int result;
  caller(&result);
  return result != 1;
}

// clang-format off

/**
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %S && %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTATE

CHECK-MUTATE-NOT:{{^.*[Ee]rror.*$}}

CHECK-MUTATE:[info] Applying filter: junk (threads: 1)
CHECK-MUTATE-DAG:{{.*}}[debug] CXXJunkDetector: mutation "Replace Call with Zero": {{.*}}sample.cpp:6:13 (end: 6:21)
CHECK-MUTATE-DAG:{{.*}}[debug] CXXJunkDetector: mutation "Replace Call with Non-Zero": {{.*}}sample.cpp:6:13 (end: 6:21)

RUN: (unset TERM; %mull_runner -debug -reporters=IDE -ide-reporter-show-killed %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (2/2):
CHECK:{{^.*}}sample.cpp:6:13: warning: Killed: Replaced callee() with 42 [cxx_replace_scalar_call_nz]{{$}}
CHECK:  *result = callee();
CHECK:            ^
CHECK:{{^.*}}sample.cpp:6:13: warning: Killed: Replaced callee() with 0 [cxx_replace_scalar_call_zr]{{$}}
CHECK:  *result = callee();
CHECK:            ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
