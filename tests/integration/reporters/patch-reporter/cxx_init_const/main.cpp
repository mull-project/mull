// clang-format off
int compute() {
  int a = 0;
  int n = -5;
  int braced{3};
  double d = 3.14159265358979;
  return a + n + braced + (int)d;
}

int main() {
  return compute() != 1;
}

/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/main.cpp -o Output/main.cpp-ir.exe

RUN: cd %S/Output; (unset TERM; %mull_runner -debug ./main.cpp-ir.exe --report-name test-ir --reporters Patches --reporters SQLite --reporters IDE; test $? = 0; ls -R %S/Output/test-ir-patches; cd %S/Output/test-ir-patches; cat `ls`) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: cd %S/Output; (unset TERM; %mull_reporter -debug test-ir.sqlite --report-name test-ir2 --reporters Patches; test $? = 0; ls -R %S/Output/test-ir2-patches; cd %S/Output/test-ir2-patches; cat `ls`) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Patchfiles can be found at './test{{.*}}-patches'
CHECK-DAG:+  int a = 1;
CHECK-DAG:+  int n = -6;
CHECK-DAG:+  int braced{2};
CHECK-DAG:+  double d = -3.14159265358979;

*/
