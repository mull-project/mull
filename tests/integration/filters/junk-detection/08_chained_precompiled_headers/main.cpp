// clang-format off

int main() {
  return triple(1) - 3;
}

/*
RUN: %clang_cxx -x c++-header -c %S/base.hh -o %t.base.pch
RUN: %clang_cxx -x c++-header -include-pch %t.base.pch -c %S/derived.hh -o %t.derived.pch
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -include-pch %t.derived.pch -g -grecord-command-line %s -o %t.exe
RUN: (unset TERM; %mull_runner --allow-surviving -reporters=IDE -ide-reporter-show-killed %t.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (2/2):
CHECK-NEXT:{{^.*}}/base.hh:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return a + b;
CHECK-NEXT:           ^
CHECK-NEXT:{{^.*}}/derived.hh:2:20: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return sum(a, a) + a;
CHECK-NEXT:                   ^
CHECK-NEXT:[info] Mutation score: 100%
CHECK-NEXT:[info] All mutations have been killed
*/
