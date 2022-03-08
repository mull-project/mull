bool isFalse(bool a) {
  return !a;
}

int main() {
  return (isFalse(true) == false) == false;
}

// clang-format off

/**
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %S && %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTATE

CHECK-MUTATE-NOT:{{^.*[Ee]rror.*$}}

CHECK-MUTATE:[info] Applying filter: junk (threads: 1)

TODO: IDE reporter reports location "!a" but we would rather want to see the location of '!'.
CHECK-MUTATE:[debug] CXXJunkDetector: mutation "Remove Unary Negation": {{.*}}sample.cpp:2:10 (end: 2:12)

RUN: (unset TERM; %mull_runner -debug -reporters=IDE -ide-reporter-show-killed %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:10: warning: Killed: Replaced !a with a [cxx_remove_negation]{{$}}
CHECK:  return !a;
CHECK:         ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
