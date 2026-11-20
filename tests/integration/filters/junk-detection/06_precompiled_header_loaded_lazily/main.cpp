// clang-format off

int main() {
  return sum(-2, 2);
}

/*
Headers without function bodies stay unloaded in the PCH until a file is translated.
Translating sum.h, the first of them, loads the rest while junk detection iterates the file table.

RUN: rm -rf %t.dir && mkdir -p %t.dir
RUN: echo '#include "%S/sum.h"' > %t.dir/all.h
RUN: for i in $(seq 1000); do echo "int f$i();" > %t.dir/h$i.h; echo "#include \"h$i.h\"" >> %t.dir/all.h; done
RUN: %clang_cxx -x c++-header -c %t.dir/all.h -o %t.pch
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -include-pch %t.pch -g -grecord-command-line %s -o %t.exe
RUN: (unset TERM; %mull_runner --allow-surviving -reporters=IDE -ide-reporter-show-killed %t.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (1/1):
CHECK-NEXT:{{^.*}}/sum.h:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
CHECK-NEXT:  return a + b;
CHECK-NEXT:           ^
CHECK-NEXT:[info] Mutation score: 100%
CHECK-NEXT:[info] All mutations have been killed

*/
