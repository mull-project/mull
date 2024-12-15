// clang-format off

/**
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s.exe

/// IDEReporter is enabled by default. So we test both invocations of mull-cxx:
/// 1) when -reporters=IDE is not provided
/// 2) when -reporters=IDE is provided

RUN: (unset TERM; %mull_runner %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
RUN: (unset TERM; %mull_runner -reporters=IDE %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
WITHOUT-OPTION:[info] Running mutants (threads: 1)
WITHOUT-OPTION:{{^       \[################################\] 1/1\. Finished .*}}
WITHOUT-OPTION-NEXT:[info] All mutations have been killed
WITHOUT-OPTION-NEXT:[info] Mutation score: 100%

RUN: (unset TERM; %mull_runner -ide-reporter-show-killed %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
RUN: (unset TERM; %mull_runner -reporters=IDE -ide-reporter-show-killed %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
WITH-OPTION:[info] Running mutants (threads: 1)
WITH-OPTION:{{^       \[################################\] 1/1\. Finished .*}}
WITH-OPTION:[info] Killed mutants (1/1):
WITH-OPTION-NEXT:{{^.*}}sample.cpp:30:18: warning: Killed: Replaced + with - [cxx_add_to_sub]
WITH-OPTION-NEXT:  int result = a + b;
WITH-OPTION-NEXT:                 ^
WITH-OPTION-NEXT:[info] All mutations have been killed
WITH-OPTION-NEXT:[info] Mutation score: 100%
*/

int sum(int a, int b) {
  int result = a + b;
  return result;
}

int main() {
  return sum(-2, 2);
}
