// clang-format off

int main() {
  int values[2] = {};
  return sum(-2, 2) + twice(1) + (int)distance(values, values + 2) - 4;
}

/*
CMake's target_precompile_headers passes both the PCH and its header through -Xclang

RUN: printf '#pragma once\n#include "%S/pch.hh"\n' > %t.hxx
RUN: %clang_cxx -x c++-header -c %t.hxx -o %t.hxx.pch
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -Xclang -include-pch -Xclang %t.hxx.pch -Xclang -include -Xclang %t.hxx -g -grecord-command-line %s -o %t.exe
RUN: (unset TERM; %mull_runner --allow-surviving -reporters=IDE -ide-reporter-show-killed %t.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

The pointer difference in the header is junk

CHECK:[info] Killed mutants (5/5):
CHECK-NEXT:{{^.*}}/main.cpp:5:21: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return sum(-2, 2) + twice(1) + (int)distance(values, values + 2) - 4;
CHECK-NEXT:                    ^
CHECK-NEXT:{{^.*}}/main.cpp:5:32: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return sum(-2, 2) + twice(1) + (int)distance(values, values + 2) - 4;
CHECK-NEXT:                               ^
CHECK-NEXT:{{^.*}}/main.cpp:5:68: warning: Killed: Replaced - with + [cxx_sub_to_add]{{$}}
CHECK-NEXT:  return sum(-2, 2) + twice(1) + (int)distance(values, values + 2) - 4;
CHECK-NEXT:                                                                   ^
CHECK-NEXT:{{^.*}}/pch.hh:7:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return a + b;
CHECK-NEXT:           ^
CHECK-NEXT:{{^.*}}/pch.hh:11:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return a + a;
CHECK-NEXT:           ^
CHECK-NEXT:[info] Mutation score: 100%
CHECK-NEXT:[info] All mutations have been killed
*/
