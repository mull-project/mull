// clang-format off

int main(void) {
  return sum(-2, 2);
}

/*
RUN: %clang_cc -x c-header -c %S/sum.h -o %t.pch
RUN: %clang_cc %sysroot %pass_mull_ir_frontend -include-pch %t.pch -g -grecord-command-line %s -o %t.exe
RUN: (unset TERM; %mull_runner --allow-surviving -reporters=IDE -ide-reporter-show-killed %t.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (1/1):
CHECK-NEXT:{{^.*}}/sum.h:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return a + b;
CHECK-NEXT:           ^
CHECK-NEXT:[info] Mutation score: 100%
CHECK-NEXT:[info] All mutations have been killed
*/
