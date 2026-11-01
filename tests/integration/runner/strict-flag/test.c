int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(2, 5) == 7 ? 0 : 1;
}

// clang-format off

/*
mull-warning.yml makes the runner warn because the test directory is not a git repository.

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -grecord-command-line -o %S/test.exe

RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull-warning.yml %mull_runner %S/test.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-WARNING
RUN: (cd %S && unset TERM; env MULL_CONFIG=%S/mull-warning.yml %mull_runner --strict %S/test.exe 2>&1; test $? = 1) | %filecheck %s --dump-input=fail --check-prefix=CHECK-STRICT

CHECK-WARNING:[warning] GitDiffReader: cannot get git diff information.{{.*}}
CHECK-WARNING-NOT:{{.*}}Strict Mode{{.*}}

CHECK-STRICT:[info] Diagnostics: Strict Mode enabled. Warning messages will be treated as fatal errors.
CHECK-STRICT:[warning] GitDiffReader: cannot get git diff information.{{.*}}
CHECK-STRICT:[warning] Strict Mode enabled: warning messages are treated as fatal errors. Exiting now.
*/
