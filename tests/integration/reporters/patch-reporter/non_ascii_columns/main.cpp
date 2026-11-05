// clang-format off
int sum(int a, int b) {
  const char *s = "üü"; return a + b;
}

int main() {
  return sum(1, 0) == 1 ? 0 : 1;
}

/**

Columns are bytes, the multi-byte characters before the mutant must not shift it

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/main.cpp -o Output/main.cpp-ir.exe

RUN: cd %S/Output; (unset TERM; %mull_runner ./main.cpp-ir.exe --allow-surviving --report-name test-ir --reporters Patches --reporters IDE --reporters SQLite; cat %S/Output/test-ir.txt %S/Output/test-ir-patches/survived-*) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: cd %S/Output; (unset TERM; %mull_reporter test-ir.sqlite --allow-surviving --report-name test-ir2 --reporters Patches; cat %S/Output/test-ir2-patches/survived-*) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=PATCH

CHECK:{{.*}}/Output/sandbox/main.cpp:3:36: warning: Survived: Replaced + with - [cxx_add_to_sub]
CHECK-NEXT:  const char *s = "üü"; return a + b;
CHECK-NEXT:                                 ^
CHECK:+  const char *s = "üü"; return a - b;

PATCH:+  const char *s = "üü"; return a - b;

*/
