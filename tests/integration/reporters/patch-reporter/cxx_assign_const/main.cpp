// clang-format off
void assign(int *x, int y) {
  x[0] = y;
}

int main() {
  int x = 15;
  assign(&x, 10);

  return 0;
}

/**

RUN: cd %S
RUN: mkdir -p %S/Output/sandbox
RUN: cp %S/main.cpp %S/Output/sandbox/main.cpp
RUN: cd %S/Output/sandbox

/// We cd to the the test directory and compile using relative paths.
RUN: cd %S; %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g Output/sandbox/main.cpp -o Output/main.cpp-ir.exe

RUN: cd %S/Output; (unset TERM; %mull_runner -debug ./main.cpp-ir.exe --report-name test-ir --reporters Patches --reporters IDE; test $? = 0; ls -R %S/Output/test-ir-patches; cd %S/Output/test-ir-patches; cat `ls`) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[debug] Writing Patchfile: {{.*}}
CHECK:[info] Patchfiles can be found at './test{{.*}}-patches'
CHECK:survived-{{.*}}
CHECK:--- a/{{.*}}/Output/sandbox/main.cpp 0
CHECK:+++ b/{{.*}}/Output/sandbox/main.cpp 0
CHECK:+{{\s+}}x[0] = 42; y;

*/
