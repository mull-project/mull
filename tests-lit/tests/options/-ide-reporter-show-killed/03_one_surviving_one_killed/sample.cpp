// clang-format off

/**
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s.exe

/// IDEReporter is enabled by default. So we test both invocations of mull-cxx:
/// 1) when -reporters=IDE is not provided
/// 2) when -reporters=IDE is provided

RUN: (unset TERM; %mull_runner %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
RUN: (unset TERM; %mull_runner -reporters=IDE %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
WITHOUT-OPTION:[info] Running mutants (threads: 2)
WITHOUT-OPTION:{{^       \[################################\] 2/2\. Finished .*}}
WITHOUT-OPTION:[info] Survived mutants (1/2):
WITHOUT-OPTION-NEXT:{{^.*}}sample.cpp:37:19: warning: Survived: Replaced + with - [cxx_add_to_sub]
WITHOUT-OPTION-NEXT:  result = result + 0;
WITHOUT-OPTION-NEXT:                  ^
WITHOUT-OPTION-NEXT:[info] Mutation score: 50%

RUN: (unset TERM; %mull_runner -ide-reporter-show-killed %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
RUN: (unset TERM; %mull_runner -reporters=IDE -ide-reporter-show-killed %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
WITH-OPTION:[info] Running mutants (threads: 2)
WITH-OPTION:{{^       \[################################\] 2/2\. Finished .*}}
WITH-OPTION:[info] Killed mutants (1/2):
WITH-OPTION-NEXT:{{^.*}}sample.cpp:36:18: warning: Killed: Replaced + with - [cxx_add_to_sub]
WITH-OPTION-NEXT:  int result = a + b;
WITH-OPTION-NEXT:                 ^
WITH-OPTION-NEXT:[info] Survived mutants (1/2):
WITH-OPTION-NEXT:{{^.*}}sample.cpp:37:19: warning: Survived: Replaced + with - [cxx_add_to_sub]
WITH-OPTION-NEXT:  result = result + 0;
WITH-OPTION-NEXT:                  ^
WITH-OPTION-NEXT:[info] Mutation score: 50%
*/
extern "C" int printf(const char *, ...);
int sum(int a, int b) {
  int result = a + b;
  result = result + 0;
  printf("%d + %d = %d\n", a, b, result);
  return result;
}

int main() {
  return sum(-2, 2);
}
