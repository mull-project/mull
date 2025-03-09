// clang-format off

int main() {
  return sum(-2, 2);
}

/*

RUN: %clang_cxx -c %S/main.hh -o %T/main.hh.pch
RUN: cd / && env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot %pass_mull_ir_frontend -include-pch %T/main.hh.pch -g -grecord-command-line %s -o %s.exe
RUN: (unset TERM; %mull_runner --allow-surviving -reporters=IDE -ide-reporter-show-killed %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (1/1):
CHECK-NEXT:{{^.*}}/main.hh:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return a + b;
CHECK-NEXT:           ^
CHECK-NEXT:[info] All mutations have been killed
CHECK-NEXT:[info] Mutation score: 100%

*/
